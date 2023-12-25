#include "nglerr.h"

#include <cstdio>
#include <cstdlib>

#include "nglgl.h"
#include "ngllog.h"

void nglCheckErrors(const char* file, int line) {
    bool errorDetected = false;
    while (true) {
        GLenum error = glGetError();
        if (error == GL_NO_ERROR) {
            break;
        }
        const char* errorStr;
        switch (error) {
            case GL_INVALID_ENUM:
                errorStr = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                errorStr = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                errorStr = "GL_INVALID_OPERATION";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                errorStr = "GL_OUT_OF_MEMORY";
                break;
            case GL_STACK_UNDERFLOW:
                errorStr = "GL_STACK_UNDERFLOW";
                break;
            case GL_STACK_OVERFLOW:
                errorStr = "GL_STACK_OVERFLOW";
                break;
            default:
                errorStr = "UNKNOWN";
                break;
        }
        errorDetected = true;
        NGL_LOGE("OpenGL error: %s(%d) at %s:%d", errorStr, error, file, line);
    }
    if (errorDetected) {
        exit(EXIT_FAILURE);
    }
}
