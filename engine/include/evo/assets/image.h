#pragma once
#include <evo/core/types.h>

struct Image {
    u8* data = nullptr;
    usize size = 0;
    u32 width = 0;
    u32 height = 0;
    u32 channels = 0;
    // 4 bytes padding
};

namespace assets {
    bool LoadImage(const char* path, Image* image);
    void FreeImage(Image* image);
} // namespace assets
