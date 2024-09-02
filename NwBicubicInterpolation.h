#pragma once

#include "nwglm.h"

class NwBicubicInterpolation {
public:
    // samples are row-major, samples[y * 4 + x] = f(x, y)
    class NwBicubicInterpolation();
    class NwBicubicInterpolation(int column, int row, float samples[16]);
    ~NwBicubicInterpolation() = default;

    float interpolate(float normalizedX, float normalizedY);

private:
    int mColumn;
    int mRow;
    glm::mat4 mA;
};
