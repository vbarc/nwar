#include "NglCamera.h"

#include "nglgl.h"

constexpr float kAcceleration = 100.0f;
constexpr float kDeceleration = 6.0f;
constexpr float kFasterFactor = 8.0f;
constexpr float kMaxSpeed = 10.0f;
constexpr float kRotationFactor = 2.0f;

NglCamera::NglCamera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
    : mOriginalPosition(position), mOriginalTarget(target), mOriginalUp(up) {
    reset();
}

bool NglCamera::onKeyEvent(int key, int scancode, int action, int mods) {
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
    if (key == GLFW_KEY_SPACE) {
        reset();
        handled = true;
    }
    mMoveOrder.faster = (mods & GLFW_MOD_SHIFT) != 0;
    return handled;
}

bool NglCamera::onMouseButtonEvent(GLFWwindow* window, int button, int action, int mods) {
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

bool NglCamera::onMouseMotionEvent(GLFWwindow* window, double x, double y) {
    bool handled = false;
    if (mRotationActive) {
        glm::vec2 mousePosition = glm::vec2(x, y);
        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);
        float base = (width + height) / 2.0f;
        glm::vec2 mouseDelta = (mousePosition - mMousePositionAtRotationStart) / base;
        const glm::quat rotation = glm::quat(glm::vec3(mouseDelta.y, mouseDelta.x, 0.0f) * -kRotationFactor);
        mLookAtOrientation = rotation * mLookAtOrientationAtRotationStart;
        mLookAtOrientation = glm::normalize(mLookAtOrientation);
        handled = true;
    }
    return handled;
}

void NglCamera::onNextFrame() {
    double time = glfwGetTime();
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

glm::mat4 NglCamera::getViewMatrix() const {
    const glm::mat4 t = glm::translate(glm::mat4(1.0f), -mPosition);
    const glm::mat4 r = glm::mat4_cast(mLookAtOrientation);
    return r * t;
}

void NglCamera::reset() {
    mPosition = mOriginalPosition;
    mLookAtOrientation = glm::lookAt(mOriginalPosition, mOriginalTarget, mOriginalUp);
    mVelocity = glm::vec3(0.0f);
}
