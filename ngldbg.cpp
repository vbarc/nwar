#include "ngldbg.h"

#include <cstdio>

#include "nglerr.h"
#include "nglgl.h"
#include "ngllog.h"

#define __NGL_DEBUG_ENABLED

static void APIENTRY debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                          const GLchar* message, const void* userParam);

void nglPrepareDebugIfNecessary() {
#ifndef __NGL_DEBUG_ENABLED
    if (true) {
        return;
    }
#endif
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
}

void nglEnableDebugIfNecessary() {
#ifndef __NGL_DEBUG_ENABLED
    if (true) {
        return;
    }
#endif
    glEnable(GL_DEBUG_OUTPUT);
    NGL_CHECK_ERRORS;
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    NGL_CHECK_ERRORS;
    glDebugMessageCallback(debugMessageCallback, nullptr);
    NGL_CHECK_ERRORS;
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    NGL_CHECK_ERRORS;
}

static void APIENTRY debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/,
                                          const GLchar* message, const void* /*userParam*/) {
    const char* sourceStr;
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            sourceStr = "GL_DEBUG_SOURCE_API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceStr = "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceStr = "GL_DEBUG_SOURCE_SHADER_COMPILER";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceStr = "GL_DEBUG_SOURCE_THIRD_PARTY";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceStr = "GL_DEBUG_SOURCE_APPLICATION";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            sourceStr = "GL_DEBUG_SOURCE_OTHER";
            break;
        default:
            sourceStr = "UNKNOWN";
            break;
    }

    const char* typeStr;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            typeStr = "GL_DEBUG_TYPE_ERROR";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            typeStr = "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            typeStr = "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            typeStr = "GL_DEBUG_TYPE_PORTABILITY";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            typeStr = "GL_DEBUG_TYPE_PERFORMANCE";
            break;
        case GL_DEBUG_TYPE_MARKER:
            typeStr = "GL_DEBUG_TYPE_MARKER";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            typeStr = "GL_DEBUG_TYPE_PUSH_GROUP";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            typeStr = "GL_DEBUG_TYPE_POP_GROUP";
            break;
        case GL_DEBUG_TYPE_OTHER:
            typeStr = "GL_DEBUG_TYPE_OTHER";
            break;
        default:
            typeStr = "UNKNOWN";
            break;
    }

    const char* severityStr;
    switch (severity) {
        case GL_DEBUG_SEVERITY_LOW:
            severityStr = "GL_DEBUG_SEVERITY_LOW";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severityStr = "GL_DEBUG_SEVERITY_MEDIUM";
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            severityStr = "GL_DEBUG_SEVERITY_HIGH";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severityStr = "GL_DEBUG_SEVERITY_NOTIFICATION";
            break;
        default:
            severityStr = "UNKNOWN";
            break;
    }
    NGL_LOGE("OpenGL debug error: %s|%s|%s|%d:\n==v==\n%s==^==", severityStr, typeStr, sourceStr, id, message);
}
