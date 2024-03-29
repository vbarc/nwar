#pragma once

#include <string>

#include "nglgl.h"

class NglProgram {
public:
    NglProgram(const NglProgram&) = delete;
    NglProgram& operator=(const NglProgram&) = delete;
    NglProgram(NglProgram&&) noexcept;
    NglProgram& operator=(NglProgram&&) noexcept;
    ~NglProgram();

    void use() const;

    class Builder {
    public:
        Builder& setVertexShader(const char* shaderCode);
        Builder& setGeometryShader(const char* shaderCode);
        Builder& setFragmentShader(const char* shaderCode);

        NglProgram build();

    private:
        static GLuint generateShader(GLenum shaderType, const char* shaderCode, const char* label);

        std::string mVertexShaderCode;
        std::string mGeometryShaderCode;
        std::string mFragmentShaderCode;
    };

private:
    NglProgram(GLuint name);

    GLuint mName;
};
