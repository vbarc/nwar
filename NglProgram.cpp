#include "NglProgram.h"

#include "nglassert.h"
#include "nglerr.h"

NglProgram::NglProgram(GLuint name) : mName(name) {
    NGL_ASSERT(name);
}

NglProgram::~NglProgram() {
    if (mName) {
        glDeleteProgram(mName);
    }
}

NglProgram::NglProgram(NglProgram&& otherProgram) noexcept : mName(std::exchange(otherProgram.mName, 0)) {}

NglProgram& NglProgram::operator=(NglProgram&& otherProgram) noexcept {
    if (mName) {
        glDeleteProgram(mName);
    }
    mName = std::exchange(otherProgram.mName, 0);
    return *this;
}

void NglProgram::use() {
    NGL_ASSERT(mName);
    glUseProgram(mName);
}

NglProgram::Builder& NglProgram::Builder::setVertexShader(const char* shaderCode) {
    NGL_ASSERT(shaderCode);
    mVertexShaderCode = shaderCode;
    return *this;
}

NglProgram::Builder& NglProgram::Builder::setFragmentShader(const char* shaderCode) {
    NGL_ASSERT(shaderCode);
    mFragmentShaderCode = shaderCode;
    return *this;
}

NglProgram NglProgram::Builder::build() {
    NGL_ASSERT(!mVertexShaderCode.empty());
    NGL_ASSERT(!mFragmentShaderCode.empty());

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    NGL_CHECK_ERRORS;
    const char* vertexShaderCode = mVertexShaderCode.data();
    glShaderSource(vertexShader, 1, &vertexShaderCode, nullptr);
    NGL_CHECK_ERRORS;
    glCompileShader(vertexShader);
    NGL_CHECK_ERRORS;

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    NGL_CHECK_ERRORS;
    const char* fragmentShaderCode = mFragmentShaderCode.data();
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, nullptr);
    NGL_CHECK_ERRORS;
    glCompileShader(fragmentShader);
    NGL_CHECK_ERRORS;

    GLuint program = glCreateProgram();
    NGL_CHECK_ERRORS;
    glAttachShader(program, vertexShader);
    NGL_CHECK_ERRORS;
    glAttachShader(program, fragmentShader);
    NGL_CHECK_ERRORS;
    glLinkProgram(program);
    NGL_CHECK_ERRORS;

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return NglProgram(program);
}
