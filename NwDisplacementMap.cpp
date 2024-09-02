#include "NwDisplacementMap.h"

#include <stb_image.h>

#include "nglassert.h"
#include "ngllog.h"

NwDisplacementMap::NwDisplacementMap(const char* path) {
    mData = stbi_load(path, &mWidth, &mDepth, nullptr, STBI_grey);
    NGL_ASSERT(mData);
    NGL_ASSERT(mWidth > 0);
    NGL_ASSERT(mDepth > 0);
    NGL_LOGI("%s loaded, width: %d, depth: %d", path, mWidth, mDepth);
    for (int r = 0; r < mDepth; r++) {
        for (int c = 0; c < mWidth; c++) {
            NGL_LOGI("(%02d, %02d): %d", r, c, mData[r * mWidth + c]);
        }
    }
}

NwDisplacementMap::~NwDisplacementMap() {
    stbi_image_free(mData);
}

int NwDisplacementMap::width() const {
    return mWidth;
}

int NwDisplacementMap::depth() const {
    return mDepth;
}

float NwDisplacementMap::lookup(int x, int z) const {
    NGL_ASSERT(x >= 0);
    NGL_ASSERT(x < mWidth);
    NGL_ASSERT(z >= 0);
    NGL_ASSERT(z < mDepth);
    return mData[z * mWidth + x] / 255.0f;
}
