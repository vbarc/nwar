#include "NglVertexArray.h"

#include "nglassert.h"
#include "nglerr.h"

static GLuint create() {
    GLuint name;
    glCreateVertexArrays(1, &name);
    NGL_CHECK_ERRORS;
    NGL_ASSERT(name);
    return name;
}

NglVertexArray::NglVertexArray() : mName(create()) {}

NglVertexArray::~NglVertexArray() {
    glDeleteVertexArrays(1, &mName);
    NGL_CHECK_ERRORS;
}

NglVertexArray::operator GLuint() const {
    return mName;
}
