#include "NvkCamera.h"

#include <algorithm>

#include "nvkvk.h"

constexpr float kAcceleration = 8.0f;
constexpr float kDeceleration = 6.0f;
constexpr float kFasterFactor = 8.0f;
constexpr float kMaxSpeed = 1.0f;
constexpr float kRotationFactor = 2.0f;

NvkCamera::NvkCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
    : mOriginalPosition(position), mOriginalTarget(target), mOriginalUp(up) {
    reset();
}

bool NvkCamera::onKeyEvent(int key, int /*scancode*/, int action, int mods) {
    bool handled = false;
    bool pressed = action != GLFW_RELEASE;
    if (key == GLFW_KEY_W) {
        mMoveOrder.forward = pressed;
        handled = true;
    }
    if (key == GLFW_KEY_S) {
        mMoveOrder.backward = pressed;
        handled = true;
    }
    if (key == GLFW_KEY_A) {
        mMoveOrder.left = pressed;
        handled = true;
    }
    if (key == GLFW_KEY_D) {
        mMoveOrder.right = pressed;
        handled = true;
    }
    if (key == GLFW_KEY_1) {
        mMoveOrder.up = pressed;
        handled = true;
    }
    if (key == GLFW_KEY_2) {
        mMoveOrder.down = pressed;
        handled = true;
    }
    if (key == GLFW_KEY_0) {
        reset();
        handled = true;
    }
    mMoveOrder.faster = (mods & GLFW_MOD_SHIFT) != 0;
    return handled;
}

bool NvkCamera::onMouseButtonEvent(GLFWwindow* window, int button, int action, int /*mods*/) {
    bool handled = false;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mRotationActive = action == GLFW_PRESS;
        handled = true;
    }
    if (mRotationActive) {
        double x;
        double y;
        glfwGetCursorPos(window, &x, &y);
        mMousePositionAtRotationStart = glm::vec2(x, y);
        mLookAtOrientationAtRotationStart = mLookAtOrientation;
    }
    return handled;
}

bool NvkCamera::onMouseMotionEvent(GLFWwindow* window, double x, double y) {
    bool handled = false;
    if (mRotationActive) {
        glm::vec2 mousePosition = glm::vec2(x, y);
        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);
        float base = (width + height) / 2.0f;
        glm::vec2 mouseDelta = (mousePosition - mMousePositionAtRotationStart) / base;
        const glm::quat rotation1 = glm::quat(glm::vec3(0.0f, -kRotationFactor * mouseDelta.x, 0.0f));
        const glm::quat rotation2 = glm::quat(glm::vec3(-kRotationFactor * mouseDelta.y, 0.0, 0.0f));
        mLookAtOrientation = rotation2 * mLookAtOrientationAtRotationStart * rotation1;
        mLookAtOrientation = glm::normalize(mLookAtOrientation);
        handled = true;
    }
    return handled;
}

void NvkCamera::onNextFrame(double time) {
    float timeDeltaSeconds = static_cast<float>(time - mPreviousFrameTime);
    mPreviousFrameTime = time;

    const glm::mat4 orientation = glm::mat4_cast(mLookAtOrientation);
    const glm::vec3 forward = -glm::vec3(orientation[0][2], orientation[1][2], orientation[2][2]);
    const glm::vec3 right = glm::vec3(orientation[0][0], orientation[1][0], orientation[2][0]);
    const glm::vec3 up = glm::cross(right, forward);

    glm::vec3 direction(0.0f);

    if (mMoveOrder.forward) {
        direction += forward;
    }
    if (mMoveOrder.backward) {
        direction -= forward;
    }
    if (mMoveOrder.left) {
        direction -= right;
    }
    if (mMoveOrder.right) {
        direction += right;
    }
    if (mMoveOrder.up) {
        direction += up;
    }
    if (mMoveOrder.down) {
        direction -= up;
    }
    if (mMoveOrder.faster) {
        direction *= kFasterFactor;
    }

    if (direction == glm::vec3(0)) {
        // decelerate naturally according to the damping value
        mVelocity -= mVelocity * std::min(kDeceleration * timeDeltaSeconds, 1.0f);
    } else {
        // acceleration
        mVelocity += direction * kAcceleration * timeDeltaSeconds;
        const float maxSpeed = mMoveOrder.faster ? kMaxSpeed * kFasterFactor : kMaxSpeed;
        if (glm::length(mVelocity) > maxSpeed) {
            mVelocity = glm::normalize(mVelocity) * maxSpeed;
        }
    }

    mPosition += mVelocity * timeDeltaSeconds;
}

glm::mat4 NvkCamera::getModelViewMatrix() const {
    const glm::mat4 t = glm::translate(glm::mat4(1.0f), -mPosition);
    const glm::mat4 r = glm::mat4_cast(mLookAtOrientation);
    return r * t;
}

void NvkCamera::reset() {
    mPosition = mOriginalPosition;
    mLookAtOrientation = glm::lookAt(mOriginalPosition, mOriginalTarget, mOriginalUp);
    mVelocity = glm::vec3(0.0f);
}

void NvkCamera::resetUp() {
    glm::mat4 orientation = glm::mat4_cast(mLookAtOrientation);
    glm::vec3 dir = -glm::vec3(orientation[0][2], orientation[1][2], orientation[2][2]);
    mLookAtOrientation = glm::lookAt(mPosition, mPosition + dir, mOriginalUp);
    mVelocity = glm::vec3(0.0f);
}
