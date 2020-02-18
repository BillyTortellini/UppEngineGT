#ifndef __FILE_IO_HPP__ 
#define __FILE_IO_HPP__ 

#include <cstdio>
#include "uppLib.hpp"
#include "utils/tmpAlloc.hpp"

u64 get_file_size(const char* filepath) 
{
    FILE* file = fopen(filepath, "rb");
    assert(file != nullptr, "File &s could not be openend!\n", filepath);
    SCOPE_EXIT(fclose(file));

    // Get File size
    fseek(file, 0, SEEK_END); 
    u64 fileSize =  ftell(file);
    return fileSize;
}

char* load_text_file_tmp(const char* filepath) 
{
    FILE* file = fopen(filepath, "rb");
    assert(file != NULL, "File %s could not be opened!\n", filepath);
    SCOPE_EXIT(fclose(file));

    // Get File size
    fseek(file, 0, SEEK_END); 
    u64 fileSize =  ftell(file);
    fseek(file, 0, SEEK_SET); // Put cursor back to start of file

    // Alloc memory
    char* text = (char*) tmpAlloc.alloc(fileSize+1);
    
    // Read
    u64 readSize = (u64) fread(text, 1, fileSize, file); 
    assert(readSize == fileSize, "fread failed, it returned %d size instead of %d fileSize\n", readSize, fileSize);

    // Add null terminator
    text[fileSize] = '\0';

    return text;
}

// Allocates memory for the whole file, then reads the file into the specified memory area
void load_file(const char* filepath, const Blk& memory) 
{
    FILE* file = fopen(filepath, "rb");
    assert(file != NULL, "File %s could not be opnened!\n", filepath);
    SCOPE_EXIT(fclose(file));

    // Get File size
    fseek(file, 0, SEEK_END); 
    u64 fileSize =  ftell(file);
    fseek(file, 0, SEEK_SET); // Put cursor back to start of file

    assert(memory.size >= fileSize, "Given memory block too small!\n");
    
    // Read 
    u64 readSize = (u64) fread(memory.data, 1, fileSize, file); 
    assert(readSize == fileSize, "fread failed, it returned %d size instead of %d fileSize\n", readSize, fileSize);

    return;
}

#endif
