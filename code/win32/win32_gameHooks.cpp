#ifndef __WIN32_GAME_HOOKS__
#define __WIN32_GAME_HOOKS__

// GameAllocator
struct GameDataAndAlloc
{
    // Allocator
    StackAllocator _gameStack;
    Blk _tmpAllocBlk;
    BlockAllocator _b32;
    BlockAllocator _b64;
    BlockAllocator _b256;
    BlockAllocator _b1024;
    BlockAllocator _b4096;
    BlockAllocator _b8192;
    ListAllocator _listAlloc;
    SegregateAllocator<BlockAllocator, ListAllocator> _seg8192;
    SegregateAllocator<BlockAllocator, decltype(_seg8192)> _seg4096;
    SegregateAllocator<BlockAllocator, decltype(_seg4096)> _seg1024;
    SegregateAllocator<BlockAllocator, decltype(_seg1024)> _seg256;
    SegregateAllocator<BlockAllocator, decltype(_seg256)> _seg64;
    SegregateAllocator<BlockAllocator, decltype(_seg64)> gameAlloc;
    // Testing
    u64 oldGameDataSize;
    // Game Data
    GameData gameData;
};
GameDataAndAlloc* gameDataAndAlloc;

void initGlobals(GameState* state) 
{
    gameState = state;
    gameDataAndAlloc = (GameDataAndAlloc*) gameState->memory.data;
    gameData = (GameData*) &gameDataAndAlloc->gameData;
    gameAlloc = &gameDataAndAlloc->gameAlloc;
}

void createGameAlloc()
{
    GameDataAndAlloc* d = gameDataAndAlloc;
    // Check if gameMemory is big enough
    assert(gameState->memory.size > 1024*1024*256 + 1024*1024*8, "Not enough memory in memory\n");

    // Create game stack allocator
    Blk stackData = gameState->memory;
    // Skip some bytes so that we can add stuff to gamedata later
    u64 offset = sizeof(GameData) * 16;
    stackData.data = roundToAligned((void*)((u64)stackData.data + offset));
    stackData.size -= offset * 2; // This could be done lot better because rounding
    d->_gameStack.init(stackData);

    // Create temp alloc bloc
    d->_tmpAllocBlk = d->_gameStack.alloc(1024 * 1024 * 256); // 256 MB

    // Block allocators for small allocations
    d->_b32.init(&d->_gameStack, 32, 8192*4*4); 
    d->_b64.init(&d->_gameStack, 64, 8192*2*4); 
    d->_b256.init(&d->_gameStack, 256, 8192*2); 
    d->_b1024.init(&d->_gameStack, 1024, 8192); 
    d->_b4096.init(&d->_gameStack, 4096, 2048);
    d->_b8192.init(&d->_gameStack, 8192, 1024);
    // For big allocations use the list allocator
    d->_listAlloc.init(d->_gameStack.allocAll());

    d->_seg8192.init(8192, &d->_b8192, &d->_listAlloc); 
    d->_seg4096.init(4096, &d->_b4096, &d->_seg8192);
    d->_seg1024.init(1024, &d->_b1024, &d->_seg4096);
    d->_seg256.init(256, &d->_b256, &d->_seg1024);
    d->_seg64.init(64, &d->_b64, &d->_seg256);
    d->gameAlloc.init(32, &d->_b32, &d->_seg64);
}

// PLATFORM CALLBACKS
typedef ListenerToken (*createFileListenerFunc)(const char* path, listenerCallbackFunc callback, void* userData);
typedef void (*deleteFileListenerFunc)(ListenerToken token);
createFileListenerFunc _createFileListener;
deleteFileListenerFunc _deleteFileListener;

ListenerToken createFileListener(const char* path, listenerCallbackFunc callback, void* userData) {
    return _createFileListener(path, callback, userData);
}

void deleteFileListener(ListenerToken token) {
    _deleteFileListener(token);
}

// BINDINGS FOR DLL LOADING
extern "C"
{
    __declspec(dllexport) void gameInit(GameState* state) 
    {
        // Set important globals
        initGlobals(state);
        createGameAlloc();
        initTmpAlloc(gameDataAndAlloc->_tmpAllocBlk);
        // Init game
        gameInit();
        gameAfterReload();
    }

    __declspec(dllexport) void gameTick(GameState* state) {
        //initGlobals(state);
        gameTick();
    }

    __declspec(dllexport) void gameShutdown(GameState* state) {
        initGlobals(state);
        gameBeforeReload();
        gameShutdown();
        shutdownTmpAlloc();
    }

    __declspec(dllexport) void gameBeforeReset(GameState* state) {
        initGlobals(state);
        gameBeforeReload();
        gameDataAndAlloc->oldGameDataSize = sizeof(GameData);
        shutdownTmpAlloc();
    }

    __declspec(dllexport) void gameAfterReset(GameState* state) 
    {
        initGlobals(state);

        // Check if gameData size has changed
        if (gameDataAndAlloc->oldGameDataSize != sizeof(GameData)) 
        {
            loggf("RESET GAME\n");
            // Finish shutdown after gameBeforeReset
            gameShutdown();
            shutdownTmpAlloc();

            // Initialize everything again
            createGameAlloc();
            initTmpAlloc(gameDataAndAlloc->_tmpAllocBlk);
            gameInit();
            gameAfterReload();
            return;
        }

        initTmpAlloc(gameDataAndAlloc->_tmpAllocBlk);
        gameAfterReload();
    }

    __declspec(dllexport) void gameAudio(GameState* state, int length, byte* data) {
        initGlobals(state);
        gameAudioTick(length, data);
    }

    __declspec(dllexport) void gameLoadFunctionPtrs(void** functions) 
    {
        int i = 0;
        loggFunc loggFuncPtr = (loggFunc)functions[i++];
        invalid_pathFunc invalid_pathPtr = (invalid_pathFunc) functions[i++];
        setDebugFunctions(loggFuncPtr, invalid_pathPtr);
        //loggf("GameLoadFunctionPtrs\n");
        i++; // Init file listener
        i++; // check file changed
        _createFileListener = (createFileListenerFunc) functions[i++];
        _deleteFileListener = (deleteFileListenerFunc) functions[i++];
        glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) functions[i++];
        glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) functions[i++];
        glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) functions[i++];
        glGenBuffers = (PFNGLGENBUFFERSPROC) functions[i++];
        glBindBuffer = (PFNGLBINDBUFFERPROC) functions[i++];
        glBufferData = (PFNGLBUFFERDATAPROC) functions[i++];
        glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) functions[i++];
        glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) functions[i++];
        glUseProgram = (PFNGLUSEPROGRAMPROC) functions[i++];
        glCreateShader = (PFNGLCREATESHADERPROC) functions[i++];
        glShaderSource = (PFNGLSHADERSOURCEPROC) functions[i++];
        glCompileShader = (PFNGLCOMPILESHADERPROC) functions[i++];
        glDeleteShader = (PFNGLDELETESHADERPROC) functions[i++];
        glCreateProgram = (PFNGLCREATEPROGRAMPROC) functions[i++];
        glDeleteProgram = (PFNGLDELETEPROGRAMPROC) functions[i++];
        glAttachShader = (PFNGLATTACHSHADERPROC) functions[i++];
        glDetachShader = (PFNGLDETACHSHADERPROC) functions[i++];
        glLinkProgram = (PFNGLLINKPROGRAMPROC) functions[i++];
        glGetShaderiv = (PFNGLGETSHADERIVPROC) functions[i++];
        glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) functions[i++];
        glGetProgramiv = (PFNGLGETPROGRAMIVPROC) functions[i++];
        glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) functions[i++];
        glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) functions[i++];
        glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) functions[i++];
        glUniform1f = (PFNGLUNIFORM1FPROC) functions[i++];
        glUniform2f = (PFNGLUNIFORM2FPROC) functions[i++];
        glUniform3f = (PFNGLUNIFORM3FPROC) functions[i++];
        glUniform4f = (PFNGLUNIFORM4FPROC) functions[i++];
        glUniform1i = (PFNGLUNIFORM1IPROC) functions[i++];
        glUniform2i = (PFNGLUNIFORM2IPROC) functions[i++];
        glUniform3i = (PFNGLUNIFORM3IPROC) functions[i++];
        glUniform4i = (PFNGLUNIFORM4IPROC) functions[i++];
        glUniform1ui = (PFNGLUNIFORM1UIPROC) functions[i++];
        glUniform2ui = (PFNGLUNIFORM2UIPROC) functions[i++];
        glUniform3ui = (PFNGLUNIFORM3UIPROC) functions[i++];
        glUniform4ui = (PFNGLUNIFORM4UIPROC) functions[i++];
        glUniform1fv = (PFNGLUNIFORM1FVPROC) functions[i++];
        glUniform2fv = (PFNGLUNIFORM2FVPROC) functions[i++];
        glUniform3fv = (PFNGLUNIFORM3FVPROC) functions[i++];
        glUniform4fv = (PFNGLUNIFORM4FVPROC) functions[i++];
        glUniform1iv = (PFNGLUNIFORM1IVPROC) functions[i++];
        glUniform2iv = (PFNGLUNIFORM2IVPROC) functions[i++];
        glUniform3iv = (PFNGLUNIFORM3IVPROC) functions[i++];
        glUniform4iv = (PFNGLUNIFORM4IVPROC) functions[i++];
        glUniform1uiv = (PFNGLUNIFORM1UIVPROC) functions[i++];
        glUniform2uiv = (PFNGLUNIFORM2UIVPROC) functions[i++];
        glUniform3uiv = (PFNGLUNIFORM3UIVPROC) functions[i++];
        glUniform4uiv = (PFNGLUNIFORM4UIVPROC) functions[i++];
        glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC) functions[i++];
        glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC) functions[i++];
        glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) functions[i++];
        glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) functions[i++];
        glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC) functions[i++];
        glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC) functions[i++];
        glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC) functions[i++];
        glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC) functions[i++];
        glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC) functions[i++];
        glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC) functions[i++];
        glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) functions[i++];
        glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) functions[i++];
        glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) functions[i++];
        glGetStringi = (PFNGLGETSTRINGIPROC) functions[i++];
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) functions[i++];
        wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC) functions[i++];
        glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) functions[i++];
        glActiveTexture = (PFNGLACTIVETEXTUREPROC) functions[i++];
        glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) functions[i++];
        glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) functions[i++];
        glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) functions[i++];
        glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) functions[i++];
        glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) functions[i++];
        glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) functions[i++];
        glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) functions[i++];
        glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) functions[i++];
        glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) functions[i++];
        glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) functions[i++];
    }
}




// Implmenentat of STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"






#endif 
