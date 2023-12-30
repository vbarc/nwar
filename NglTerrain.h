#pragma once

class NglTerrain {
public:
    NglTerrain();
    ~NglTerrain();

private:
    int mWidth;
    int mHeight;
    unsigned char *mData;
};
