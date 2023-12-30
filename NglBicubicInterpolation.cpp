#include "NglBicubicInterpolation.h"

#include <Eigen/Dense>

using EVector = Eigen::Vector<float, 16>;
using EMatrix = Eigen::Matrix<float, 16, 16>;

const auto kSolver =
        EMatrix{
                {1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1},  //
                {1, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, -1, 0, 0, 0},        //
                {1, 1, 1, 1, -1, -1, -1, -1, 1, 1, 1, 1, -1, -1, -1, -1},  //
                {1, 2, 4, 8, -1, -2, -4, -8, 1, 2, 4, 8, -1, -2, -4, -8},  //
                //
                {1, -1, 1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  //
                {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},    //
                {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},    //
                {1, 2, 4, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},    //
                //
                {1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1},  //
                {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},          //
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},          //
                {1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8},          //
                //
                {1, -1, 1, -1, 2, -2, 2, -2, 4, -4, 4, -4, 8, -8, 8, -8},  //
                {1, 0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 8, 0, 0, 0},          //
                {1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 8, 8, 8, 8},          //
                {1, 2, 4, 8, 2, 4, 8, 16, 4, 8, 16, 32, 8, 16, 32, 64},    //
        }
                .colPivHouseholderQr();

NglBicubicInterpolation::NglBicubicInterpolation() : mA{0} {}

NglBicubicInterpolation::NglBicubicInterpolation(float samples[16]) {
    EVector f{samples};
    EVector a = kSolver.solve(f);
    for (int i = 0; i < 16; i++) {
        mA[i] = a(i);
    }
}

float NglBicubicInterpolation::interpolate(float x, float y) {
    float x2 = x * x;
    float x3 = x2 * x;
    float y2 = y * y;
    float y3 = y2 * y;
    return mA[0] + mA[1] * x + mA[2] * x2 + mA[3] * x3                          //
           + mA[4] * y + mA[5] * y * x + mA[6] * y * x2 + mA[7] * y * x3        //
           + mA[8] * y2 + mA[9] * y2 * x + mA[10] * y2 * x2 + mA[11] * y2 * x3  //
           + mA[12] * y3 + mA[13] * y3 * x + mA[14] * y3 * x2 + mA[15] * y3 * x3;
}
