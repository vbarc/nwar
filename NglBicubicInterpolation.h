#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>

class NglBicubicInterpolation {
public:
    // samples are row-major, samples[y * 4 + x] = f(x, y)
    class NglBicubicInterpolation();
    class NglBicubicInterpolation(int column, int row, float samples[16]);
    ~NglBicubicInterpolation() = default;

    float interpolate(float normalizedX, float normalizedY);

private:
    int mColumn;
    int mRow;
    glm::mat4 mA;
};
