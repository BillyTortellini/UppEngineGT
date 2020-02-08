#ifndef __WIN32_GL_FUNCTIONS__
#define __WIN32_GL_FUNCTIONS__

// FUNCTION POINTERS
// Debug functions
PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;

// Buffer functions
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLUSEPROGRAMPROC glUseProgram;
// Drawing 
//PFNGLDRAWARRAYSPROC glDrawArrays;
//PFNGLDRAWELEMENTSPROC glDrawElements;
// Shader creation
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
//PFNGLVIEWPORTPROC glViewport;
// Get infos
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
// Uniform stuff
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM2IPROC glUniform2i;
PFNGLUNIFORM3IPROC glUniform3i;
PFNGLUNIFORM4IPROC glUniform4i;
PFNGLUNIFORM1UIPROC glUniform1ui;
PFNGLUNIFORM2UIPROC glUniform2ui;
PFNGLUNIFORM3UIPROC glUniform3ui;
PFNGLUNIFORM4UIPROC glUniform4ui;
PFNGLUNIFORM1FVPROC glUniform1fv;
PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORM1IVPROC glUniform1iv;
PFNGLUNIFORM2IVPROC glUniform2iv;
PFNGLUNIFORM3IVPROC glUniform3iv;
PFNGLUNIFORM4IVPROC glUniform4iv;
PFNGLUNIFORM1UIVPROC glUniform1uiv;
PFNGLUNIFORM2UIVPROC glUniform2uiv;
PFNGLUNIFORM3UIVPROC glUniform3uiv;
PFNGLUNIFORM4UIVPROC glUniform4uiv;
PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv;
PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv;
PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv;
PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv;
PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv;
PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv;
// String stuff
PFNGLGETSTRINGIPROC glGetStringi;

// EXTENSIONS
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalExt;
PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = 0;

HMODULE openGlModule = NULL;
void* getAnyGLFuncAddress(const char* name) 
{
    void* p = (void*)wglGetProcAddress(name);
    if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1) )
    {
        if (openGlModule == NULL) 
        {
            openGlModule = LoadLibraryA("opengl32.dll");
            if (openGlModule == NULL) {
                return 0;
            }
        }
        p = (void *)GetProcAddress(openGlModule, name);
    }

    return p;
}

void printAllExtensions()
{ 
    GLint extCount; 
    glGetIntegerv(GL_NUM_EXTENSIONS, &extCount);
    debugPrintf("Extensions:\n---------\n");
    for (int i = 0; i < extCount; i++)
    {
        const char* extension = (const char*) glGetStringi(GL_EXTENSIONS, (GLuint)i);
        debugPrintf("\t#%d: %s\n", i, extension);

    }
    debugPrintf("\n");
    //const char* wglExtensions = (const char*) wglGetExtensionsStringARB();
    //debugPrintf("WGL Extensions:\n------------------%s\n----------------------\n", wglExtensions);
}

extern HDC deviceContext;
bool loadExtensions()
{
    wglSwapIntervalExt = (PFNWGLSWAPINTERVALEXTPROC) getAnyGLFuncAddress("wglSwapIntervalEXT");
    bool success = true;
    success = success && 
        (wglSwapIntervalExt != NULL);

    if (wglGetExtensionsStringARB == NULL)
    {
        wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC) wglGetProcAddress("wglGetExtensionsStringARB");
        if (wglGetExtensionsStringARB == NULL) {
            log("wglGetExtensionsStringARB not supported!\n");
        }
        else 
        {
            char* string = (char*) wglGetExtensionsStringARB(deviceContext);
            log("WGL extensions: \n%s\n", string);
        }
    }

    return success;
}

bool loadAllFunctions()
{
    glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) getAnyGLFuncAddress("glDebugMessageCallback");
    glGenBuffers = (PFNGLGENBUFFERSPROC) getAnyGLFuncAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC) getAnyGLFuncAddress("glBindBuffer");
    glGenBuffers = (PFNGLGENBUFFERSPROC) getAnyGLFuncAddress("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC) getAnyGLFuncAddress("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC) getAnyGLFuncAddress("glBufferData");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) getAnyGLFuncAddress("glVertexAttribPointer");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) getAnyGLFuncAddress("glEnableVertexAttribArray");
    glCreateShader = (PFNGLCREATESHADERPROC) getAnyGLFuncAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC) getAnyGLFuncAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC) getAnyGLFuncAddress("glCompileShader");
    glDeleteShader = (PFNGLDELETESHADERPROC) getAnyGLFuncAddress("glDeleteShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC) getAnyGLFuncAddress("glCreateProgram");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC) getAnyGLFuncAddress("glDeleteProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC) getAnyGLFuncAddress("glAttachShader");
    glDetachShader = (PFNGLDETACHSHADERPROC) getAnyGLFuncAddress("glDetachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC) getAnyGLFuncAddress("glLinkProgram");
    glGetShaderiv = (PFNGLGETSHADERIVPROC) getAnyGLFuncAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) getAnyGLFuncAddress("glGetShaderInfoLog");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC) getAnyGLFuncAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) getAnyGLFuncAddress("glGetProgramInfoLog");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) getAnyGLFuncAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) getAnyGLFuncAddress("glBindVertexArray");
    glUseProgram = (PFNGLUSEPROGRAMPROC) getAnyGLFuncAddress("glUseProgram");
    //glViewport = (PFNGLVIEWPORTPROC) getAnyGLFuncAddress("glViewport");
    glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) getAnyGLFuncAddress("glGetActiveUniform");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) getAnyGLFuncAddress("glGetUniformLocation");
    glUniform1f = (PFNGLUNIFORM1FPROC) getAnyGLFuncAddress("glUniform1f");
    glUniform2f = (PFNGLUNIFORM2FPROC) getAnyGLFuncAddress("glUniform2f");
    glUniform3f = (PFNGLUNIFORM3FPROC) getAnyGLFuncAddress("glUniform3f");
    glUniform4f = (PFNGLUNIFORM4FPROC) getAnyGLFuncAddress("glUniform4f");
    glUniform1i = (PFNGLUNIFORM1IPROC) getAnyGLFuncAddress("glUniform1i");
    glUniform2i = (PFNGLUNIFORM2IPROC) getAnyGLFuncAddress("glUniform2i");
    glUniform3i = (PFNGLUNIFORM3IPROC) getAnyGLFuncAddress("glUniform3i");
    glUniform4i = (PFNGLUNIFORM4IPROC) getAnyGLFuncAddress("glUniform4i");
    glUniform1ui = (PFNGLUNIFORM1UIPROC) getAnyGLFuncAddress("glUniform1ui");
    glUniform2ui = (PFNGLUNIFORM2UIPROC) getAnyGLFuncAddress("glUniform2ui");
    glUniform3ui = (PFNGLUNIFORM3UIPROC) getAnyGLFuncAddress("glUniform3ui");
    glUniform4ui = (PFNGLUNIFORM4UIPROC) getAnyGLFuncAddress("glUniform4ui");
    glUniform1fv = (PFNGLUNIFORM1FVPROC) getAnyGLFuncAddress("glUniform1fv");
    glUniform2fv = (PFNGLUNIFORM2FVPROC) getAnyGLFuncAddress("glUniform2fv");
    glUniform3fv = (PFNGLUNIFORM3FVPROC) getAnyGLFuncAddress("glUniform3fv");
    glUniform4fv = (PFNGLUNIFORM4FVPROC) getAnyGLFuncAddress("glUniform4fv");
    glUniform1iv = (PFNGLUNIFORM1IVPROC) getAnyGLFuncAddress("glUniform1iv");
    glUniform2iv = (PFNGLUNIFORM2IVPROC) getAnyGLFuncAddress("glUniform2iv");
    glUniform3iv = (PFNGLUNIFORM3IVPROC) getAnyGLFuncAddress("glUniform3iv");
    glUniform4iv = (PFNGLUNIFORM4IVPROC) getAnyGLFuncAddress("glUniform4iv");
    glUniform1uiv = (PFNGLUNIFORM1UIVPROC) getAnyGLFuncAddress("glUniform1uiv");
    glUniform2uiv = (PFNGLUNIFORM2UIVPROC) getAnyGLFuncAddress("glUniform2uiv");
    glUniform3uiv = (PFNGLUNIFORM3UIVPROC) getAnyGLFuncAddress("glUniform3uiv");
    glUniform4uiv = (PFNGLUNIFORM4UIVPROC) getAnyGLFuncAddress("glUniform4uiv");
    glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC) getAnyGLFuncAddress("glUniformMatrix2fv");
    glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC) getAnyGLFuncAddress("glUniformMatrix3fv");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) getAnyGLFuncAddress("glUniformMatrix4fv");
    glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) getAnyGLFuncAddress("glUniformMatrix2x3fv");
    glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC) getAnyGLFuncAddress("glUniformMatrix3x2fv");
    glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC) getAnyGLFuncAddress("glUniformMatrix2x4fv");
    glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC) getAnyGLFuncAddress("glUniformMatrix4x2fv");
    glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC) getAnyGLFuncAddress("glUniformMatrix3x4fv");
    glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC) getAnyGLFuncAddress("glUniformMatrix4x3fv");
    glGetStringi = (PFNGLGETSTRINGIPROC) getAnyGLFuncAddress("glGetStringi");

    bool success = true;
    success = success && 
        (glDebugMessageCallback != NULL) &&
        (glGenBuffers != NULL) &&
        (glBindBuffer != NULL) &&
        (glBufferData != NULL) &&
        (glVertexAttribPointer != NULL) &&
        (glEnableVertexAttribArray != NULL) &&
        (glCreateShader != NULL) &&
        (glShaderSource != NULL) &&
        (glCompileShader != NULL) &&
        (glDeleteShader != NULL) &&
        (glCreateProgram != NULL) &&
        (glDeleteProgram != NULL) &&
        (glAttachShader != NULL) &&
        (glDetachShader != NULL) &&
        (glLinkProgram != NULL) &&
        (glGetShaderiv != NULL) &&
        (glGetShaderInfoLog != NULL) &&
        (glGetProgramiv != NULL) &&
        (glGenVertexArrays != NULL) &&
        (glBindVertexArray != NULL) &&
        (glUseProgram != NULL) &&
        (glGetActiveUniform != NULL) &&
        (glGetUniformLocation != NULL) &&
        (glUniform1f != NULL) &&
        (glUniform2f != NULL) &&
        (glUniform3f != NULL) &&
        (glUniform4f != NULL) &&
        (glUniform1i != NULL) &&
        (glUniform2i != NULL) &&
        (glUniform3i != NULL) &&
        (glUniform4i != NULL) &&
        (glUniform1ui != NULL) &&
        (glUniform2ui != NULL) &&
        (glUniform3ui != NULL) &&
        (glUniform4ui != NULL) &&
        (glUniform1fv != NULL) &&
        (glUniform2fv != NULL) &&
        (glUniform3fv != NULL) &&
        (glUniform4fv != NULL) &&
        (glUniform1iv != NULL) &&
        (glUniform2iv != NULL) &&
        (glUniform3iv != NULL) &&
        (glUniform4iv != NULL) &&
        (glUniform1uiv != NULL) &&
        (glUniform2uiv != NULL) &&
        (glUniform3uiv != NULL) &&
        (glUniform4uiv != NULL) &&
        (glUniformMatrix2fv != NULL) &&
        (glUniformMatrix3fv != NULL) &&
        (glUniformMatrix4fv != NULL) &&
        (glUniformMatrix2x3fv != NULL) &&
        (glUniformMatrix3x2fv != NULL) &&
        (glUniformMatrix2x4fv != NULL) &&
        (glUniformMatrix4x2fv != NULL) &&
        (glUniformMatrix3x4fv != NULL) &&
        (glUniformMatrix4x3fv != NULL) &&
        (glGetStringi != NULL) &&
        //        (glViewport != NULL) &&
        (glGetProgramInfoLog != NULL);
    
    printAllExtensions();
    success = success && loadExtensions();

    return success;
}


#endif
