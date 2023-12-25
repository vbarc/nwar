#pragma once

#include <string>

#include "nglgl.h"

class NglProgram {
public:
    ~NglProgram();
    NglProgram(const NglProgram&) = delete;
    NglProgram& operator=(const NglProgram&) = delete;
    NglProgram(NglProgram&&) noexcept;
    NglProgram& operator=(NglProgram&&) noexcept;

    void use();

    operator GLuint() const;

    class Builder {
    public:
        Builder& setVertexShader(const char* shaderCode);
        Builder& setFragmentShader(const char* shaderCode);

        NglProgram build();

    private:
        static GLuint generateShader(GLenum shaderType, const char* shaderCode, const char* label);

        std::string mVertexShaderCode;
        std::string mFragmentShaderCode;
    };

private:
    NglProgram(GLuint name);

    GLuint mName;
};
