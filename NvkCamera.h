#pragma once

#include "nvkvk.h"

struct GLFWwindow;

class NvkCamera {
public:
    NvkCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

    bool onKeyEvent(int key, int scancode, int action, int mods);
    bool onMouseButtonEvent(GLFWwindow* window, int button, int action, int mods);
    bool onMouseMotionEvent(GLFWwindow* window, double x, double y);
    void onNextFrame(double time);

    glm::mat4 getModelViewMatrix() const;

private:
    void reset();
    void resetUp();

    const glm::vec3 mOriginalPosition;
    const glm::vec3 mOriginalTarget;
    const glm::vec3 mOriginalUp;

    glm::vec3 mPosition;
    glm::quat mLookAtOrientation;
    glm::vec3 mVelocity;

    struct {
        bool forward = false;
        bool backward = false;
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        bool faster = false;
    } mMoveOrder;

    bool mRotationActive = false;
    glm::vec2 mMousePositionAtRotationStart;
    glm::quat mLookAtOrientationAtRotationStart;

    double mPreviousFrameTime = 0;
};
