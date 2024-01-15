#include "NglProgram.h"

#include <vector>

#include "nglassert.h"
#include "nglerr.h"

NglProgram::NglProgram(GLuint name) : mName(name) {
    NGL_ASSERT(name);
}

NglProgram::NglProgram(NglProgram&& otherProgram) noexcept : mName(std::exchange(otherProgram.mName, 0)) {}

NglProgram& NglProgram::operator=(NglProgram&& otherProgram) noexcept {
    if (mName) {
        glDeleteProgram(mName);
    }
    mName = std::exchange(otherProgram.mName, 0);
    return *this;
}

NglProgram::~NglProgram() {
    if (mName) {
        glDeleteProgram(mName);
    }
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

NglProgram::Builder& NglProgram::Builder::setGeometryShader(const char* shaderCode) {
    NGL_ASSERT(shaderCode);
    mGeometryShaderCode = shaderCode;
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

    GLuint vertexShader = generateShader(GL_VERTEX_SHADER, mVertexShaderCode.c_str(), "mVertexShaderCode");
    GLuint geometryShader =
            !mGeometryShaderCode.empty()
                    ? generateShader(GL_GEOMETRY_SHADER, mGeometryShaderCode.c_str(), "mGeometryShaderCode")
                    : 0;
    GLuint fragmentShader = generateShader(GL_FRAGMENT_SHADER, mFragmentShaderCode.c_str(), "mFragmentShaderCode");

    GLuint program = glCreateProgram();
    NGL_CHECK_ERRORS;
    glAttachShader(program, vertexShader);
    NGL_CHECK_ERRORS;
    if (geometryShader) {
        glAttachShader(program, geometryShader);
        NGL_CHECK_ERRORS;
    }
    glAttachShader(program, fragmentShader);
    NGL_CHECK_ERRORS;
    glLinkProgram(program);
    NGL_CHECK_ERRORS;

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    NGL_CHECK_ERRORS;
    if (linkStatus == GL_FALSE) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        NGL_CHECK_ERRORS;
        std::vector<GLchar> log(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
        NGL_LOGE("Program linking failed:\n==v==\n%s==^==", log.data());
        glDeleteProgram(program);
        abort();
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return NglProgram(program);
}

GLuint NglProgram::Builder::generateShader(GLenum shaderType, const char* shaderCode, const char* label) {
    GLuint shader = glCreateShader(shaderType);
    NGL_CHECK_ERRORS;
    glShaderSource(shader, 1, &shaderCode, nullptr);
    NGL_CHECK_ERRORS;
    glCompileShader(shader);
    NGL_CHECK_ERRORS;

    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    NGL_CHECK_ERRORS;
    if (compileStatus == GL_FALSE) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        NGL_CHECK_ERRORS;
        std::vector<GLchar> log(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        NGL_LOGE("Shader '%s' compilation failed:\n==v==\n%s==^==", label, log.data());
        glDeleteShader(shader);
        abort();
    }
    return shader;
}
