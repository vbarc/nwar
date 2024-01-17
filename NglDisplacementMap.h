#pragma once

class NglDisplacementMap {
public:
    NglDisplacementMap(const char* path);
    NglDisplacementMap(const NglDisplacementMap&) = delete;
    NglDisplacementMap& operator=(const NglDisplacementMap&) = delete;
    NglDisplacementMap(NglDisplacementMap&&) = delete;
    NglDisplacementMap& operator=(NglDisplacementMap&&) = delete;
    ~NglDisplacementMap();

    int width() const;
    int depth() const;

    float lookup(int x, int z) const;

private:
    int mWidth;
    int mDepth;
    unsigned char* mData;
};
