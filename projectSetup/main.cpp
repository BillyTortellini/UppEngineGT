#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
using std::string;

void createDir(const string& name)
{
    char cmd[1024];
    sprintf(cmd, "mkdir %s", name.c_str());
    system(cmd);
}

void createFile(const string& filename, const string& content)
{
    FILE* file = fopen(filename.c_str(), "w");
    if (file == nullptr) {
        printf("could not create %s\n", filename.c_str());
        return;
    }

    size_t len = content.length();
    fwrite(content.c_str(), 1, len, file);

    fclose(file);
}

int main(int argc, char** argv) 
{
    printf("Enter project name: ");
    char buffer[512];
    scanf("%s", buffer);
    string name = buffer;

    // Create project directory
    createDir(name);
    createDir(name + "\\code");
    createDir(name + "\\build");

    // Switch to new directory

    //char* buildContent = "@echo off\npushd build\nstart \"\" cmd /c \"cl /Zi /FC /EHsc ..\\main.cpp & pause\"\npopd";
    // Create main
    string mainContent = "#include <cstdio>\n\nint main(int argc, char** argv){\n\tprintf(\"Hello there\\n\");\n\treturn 0;\n}\n";
    createFile(name+"/code/main.cpp", mainContent);

    string buildContent = "@echo off\nmkdir ..\\build\npushd ..\\build\nstart \"\" cmd /c \"cl /Zi /EHsc ..\\code\\main.cpp & pause\"\npopd";
    createFile(name + "/code/build.bat", buildContent);

    string runContent = "@echo off\npushd ..\\\nstart \"\" cmd /c \"build\\main.exe & pause\"\npopd";
    createFile(name + "/code/run.bat", runContent);

    string buildAndRunContent = "@echo off\npushd ..\\build\ncl /EHsc /Zi /FC ..\\code\\main.cpp && (start \"\" cmd /c \"cd .. & cls & build\\main.exe &pause\") || pause\npopd";
    createFile(name + "/code/buildAndRun.bat", buildAndRunContent);

    system("pause");

    return 0;
}
