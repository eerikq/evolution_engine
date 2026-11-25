#include <evo/assets/image.h>
#include <evo/core/print.h>
#include <evo/core/types.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool assets::LoadImage(const char* path, Image* image) {
    if (path == nullptr || image == nullptr) {
        print::Error("Invalid LoadImage() input");
        return false;
    }

    int x = 0, y = 0, comp = 0;
    u8* data = stbi_load(path, &x, &y, &comp, 4); // forces 4 channels atm. TODO: add support for 1 & 2 channels (probably via "const u8 desired_channels = 4" parameter)

    if (data == nullptr) {
        print::Error("Failed to load image at /{}", path);
        return false;
    }

    image->data = data;
    image->size = x * y * comp;
    image->width = x;
    image->height = y;
    image->channels = comp;

    print::Debug("Loaded image at /{} ({}x{}px with {} channels)", path, image->width, image->height, image->channels);
    return true;
}

void assets::FreeImage(Image* image) {
    if (image == nullptr) {
        print::Error("Invalid FreeImage() input");
        return;
    }

    stbi_image_free(image->data);
    image->data = nullptr;

    image->size = 0;
    image->width = 0;
    image->height = 0;
    image->channels = 0;
}
