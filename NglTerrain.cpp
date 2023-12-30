#include "NglTerrain.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "nglassert.h"
#include "ngllog.h"

NglTerrain::NglTerrain() {
    int n;
    mData = stbi_load("terrain.png", &mWidth, &mHeight, &n, 1);
    NGL_LOGI("Terrain loaded, data: %p, width: %d, height: %d, n: %d", mData, mWidth, mHeight, n);
    NGL_ASSERT(mData);
    for (int y = 0; y < mHeight; y++) {
        for (int x = 0; x < mWidth; x++) {
            NGL_LOGI("(%02d, %02d): %d", y, x, mData[y * mWidth + x]);
        }
    }
}

NglTerrain::~NglTerrain() {
    stbi_image_free(mData);
}
