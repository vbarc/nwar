#include "NglBicubicInterpolation.h"

// The math is from https://en.wikipedia.org/wiki/Bicubic_interpolation.

const auto kL = glm::mat4(  //
        1, 0, -3, 2,        //
        0, 0, 3, -2,        //
        0, 1, -2, 1,        //
        0, 0, -1, 1         //
);

const auto kR = glm::transpose(kL);

class F {
public:
    F(const float* samples) : mSamples(samples) {}

    const float operator()(int x, int y) {
        return mSamples[(y + 1) * 4 + (x + 1)];
    }

private:
    const float* mSamples;
};

NglBicubicInterpolation::NglBicubicInterpolation() {}

NglBicubicInterpolation::NglBicubicInterpolation(int column, int row, float samples[16]) : mColumn(column), mRow(row) {
    F f(samples);
    glm::mat4 m = glm::mat4(                                                                                          //
            f(0, 0), f(1, 0), (f(1, 0) - f(-1, 0)) / 2, (f(2, 0) - f(0, 0)) / 2,                                      //
            f(0, 1), f(1, 1), (f(1, 1) - f(-1, 1)) / 2, (f(2, 1) - f(0, 1)) / 2,                                      //
            (f(0, 1) - f(0, -1)) / 2, (f(1, 1) - f(1, -1)) / 2, (f(1, 1) - f(-1, -1)) / 4, (f(2, 1) - f(0, -1)) / 4,  //
            (f(0, 2) - f(0, 0)) / 2, (f(1, 2) - f(1, 0)) / 2, (f(1, 2) - f(-1, 0)) / 4, (f(2, 2) - f(0, 0)) / 4       //
    );
    mA = kL * m * kR;
}

float NglBicubicInterpolation::interpolate(float x, float y) {
    float x2 = x * x;
    float x3 = x2 * x;
    float y2 = y * y;
    float y3 = y2 * y;
    return glm::dot(glm::vec4(1, x, x2, x3) * mA, glm::vec4(1, y, y2, y3));
}
