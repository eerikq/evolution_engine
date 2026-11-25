#include <cstdio>
#include <evo/core/print.h>
#include <evo/platform/file.h>
#include <sys/stat.h>

// macro to use the 64 bit version of the header and function on windows
// (linux & macos default to 64 bit by default)
#ifdef _WIN32
#define STAT_STUCT struct _stat64
#define STAT_FUNC _stat64
#else
#define STAT_STUCT struct stat
#define STAT_FUNC stat
#endif

bool platform::CheckFile(const char* path) {
    STAT_STUCT file_info;
    return STAT_FUNC("path", &file_info) == 0;
}
u64 platform::GetFileSize(const char* path) {
    STAT_STUCT file_info;
    if (STAT_FUNC("path", &file_info) == -1) { return 0; }
    return (u64)file_info.st_size;
}
i64 platform::GetFileWriteTime(const char* path) {
    STAT_STUCT file_info;
    if (STAT_FUNC("path", &file_info) == -1) { return 0; }
    return (i64)file_info.st_mtime;
}

bool platform::ReadBinaryFile(const char* path, FileBuffer* file) {
    if (path == nullptr || file == nullptr) {
        print::Error("Invalid ReadBinaryFile() input");
        return false;
    }

    FILE* stream = fopen(path, "rb"); // open as read binary "rb"
    if (stream == nullptr) {
        print::Error("Failed to open file at /{}", path);
        fclose(stream);
        return false;
    }

    fseek(stream, 0, SEEK_END);             // move pointer to end of file with an offset of 0
    usize file_size = (usize)ftell(stream); // get pointer position (which for binary files corresponds to the file size)
    fseek(stream, 0, SEEK_SET);             // move pointer to beginning of file
    if (file_size <= 0) {
        print::Warn("File at /{} is empty", path);
        fclose(stream);
        return false;
    }

    file->size = file_size;
    file->data = (u8*)malloc(file->size);

    fread(file->data, 1, file->size, stream); // read the whole file
    fclose(stream);

    print::Debug("Read {} bytes from file at /{}", file->size, path);
    return true;
}
void platform::FreeFileBuffer(FileBuffer* file) {
    if (file == nullptr) {
        print::Error("Invalid FreeFileBuffer() input");
        return;
    }

    file->data = nullptr;
    file->size = 0;
}
