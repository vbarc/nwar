#pragma once

class NglBicubicInterpolation {
public:
    // samples are row-major, samples[y * 4 + x] = f(x, y)
    class NglBicubicInterpolation();
    class NglBicubicInterpolation(float samples[16]);
    ~NglBicubicInterpolation() = default;

    float interpolate(float normalizedX, float normalizedY);

private:
    float mA[16];
};
