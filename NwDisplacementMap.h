#pragma once

class NwDisplacementMap {
public:
    NwDisplacementMap(const char* path);
    NwDisplacementMap(const NwDisplacementMap&) = delete;
    NwDisplacementMap& operator=(const NwDisplacementMap&) = delete;
    ~NwDisplacementMap();

    int width() const;
    int depth() const;

    float lookup(int x, int z) const;

private:
    int mWidth;
    int mDepth;
    unsigned char* mData;
};
