#pragma once
#include <evo/core/types.h>

namespace platform {
    struct FileBuffer {
        u8* data = nullptr;
        usize size = 0;
    };

    bool CheckFile(const char* path);       // check whether a file exists
    u64 GetFileSize(const char* path);      // check the size of a file
    i64 GetFileWriteTime(const char* path); // check the last time a file was written to

    bool ReadBinaryFile(const char* path, FileBuffer* file);
    void FreeFileBuffer(FileBuffer* file);
} // namespace platform
