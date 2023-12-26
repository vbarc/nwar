#pragma once

#include <algorithm>

class NglCamera {
public:
    NglCamera() = default;
    NglCamera(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up)
        : cameraPosition_(pos), cameraOrientation_(glm::lookAt(pos, target, up)), up_(up) {}

    void update(double deltaSeconds, const glm::vec2& mousePos, bool mousePressed) {
        if (mousePressed) {
            const glm::vec2 delta = mousePos - mousePos_;
            const glm::quat deltaQuat = glm::quat(glm::vec3(-mouseSpeed_ * delta.y, mouseSpeed_ * delta.x, 0.0f));
            cameraOrientation_ = deltaQuat * cameraOrientation_;
            cameraOrientation_ = glm::normalize(cameraOrientation_);
            setUpVector(up_);
        }
        mousePos_ = mousePos;

        const glm::mat4 v = glm::mat4_cast(cameraOrientation_);

        const glm::vec3 forward = -glm::vec3(v[0][2], v[1][2], v[2][2]);
        const glm::vec3 right = glm::vec3(v[0][0], v[1][0], v[2][0]);
        const glm::vec3 up = glm::cross(right, forward);

        glm::vec3 accel(0.0f);

        if (movement_.forward_)
            accel += forward;
        if (movement_.backward_)
            accel -= forward;

        if (movement_.left_)
            accel -= right;
        if (movement_.right_)
            accel += right;

        if (movement_.up_)
            accel += up;
        if (movement_.down_)
            accel -= up;

        if (movement_.fastSpeed_)
            accel *= fastCoef_;

        if (accel == glm::vec3(0)) {
            // decelerate naturally according to the damping value
            moveSpeed_ -= moveSpeed_ * std::min((1.0f / damping_) * static_cast<float>(deltaSeconds), 1.0f);
        } else {
            // acceleration
            moveSpeed_ += accel * acceleration_ * static_cast<float>(deltaSeconds);
            const float maxSpeed = movement_.fastSpeed_ ? maxSpeed_ * fastCoef_ : maxSpeed_;
            if (glm::length(moveSpeed_) > maxSpeed)
                moveSpeed_ = glm::normalize(moveSpeed_) * maxSpeed;
        }

        cameraPosition_ += moveSpeed_ * static_cast<float>(deltaSeconds);
    }

    glm::mat4 getViewMatrix() const {
        const glm::mat4 t = glm::translate(glm::mat4(1.0f), -cameraPosition_);
        const glm::mat4 r = glm::mat4_cast(cameraOrientation_);
        return r * t;
    }

    glm::vec3 getPosition() const {
        return cameraPosition_;
    }

    void setPosition(const glm::vec3& pos) {
        cameraPosition_ = pos;
    }

    void resetMousePosition(const glm::vec2& p) {
        mousePos_ = p;
    };

    void setUpVector(const glm::vec3& up) {
        const glm::mat4 view = getViewMatrix();
        const glm::vec3 dir = -glm::vec3(view[0][2], view[1][2], view[2][2]);
        cameraOrientation_ = glm::lookAt(cameraPosition_, cameraPosition_ + dir, up);
    }

    void reset(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up) {
        cameraPosition_ = pos;
        cameraOrientation_ = glm::lookAt(pos, target, up);
        moveSpeed_ = glm::vec3(0.0f);
    }

public:
    struct Movement {
        bool forward_ = false;
        bool backward_ = false;
        bool left_ = false;
        bool right_ = false;
        bool up_ = false;
        bool down_ = false;
        //
        bool fastSpeed_ = false;
    } movement_;

public:
    float mouseSpeed_ = 4.0f;
    float acceleration_ = 150.0f;
    float damping_ = 0.2f;
    float maxSpeed_ = 10.0f;
    float fastCoef_ = 10.0f;

private:
    glm::vec2 mousePos_ = glm::vec2(0);
    glm::vec3 cameraPosition_ = glm::vec3(0.0f, 10.0f, 10.0f);
    glm::quat cameraOrientation_ = glm::quat(glm::vec3(0));
    glm::vec3 moveSpeed_ = glm::vec3(0.0f);
    glm::vec3 up_ = glm::vec3(0.0f, 0.0f, 1.0f);
};
