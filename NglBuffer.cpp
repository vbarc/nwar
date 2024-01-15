#include "NglBuffer.h"

#include "nglassert.h"
#include "nglerr.h"

static GLuint create() {
    GLuint name;
    glCreateBuffers(1, &name);
    NGL_CHECK_ERRORS;
    NGL_ASSERT(name);
    return name;
}

NglBuffer::NglBuffer() : mName(create()) {}

NglBuffer::~NglBuffer() {
    glDeleteBuffers(1, &mName);
    NGL_CHECK_ERRORS;
}

NglBuffer::operator GLuint() const {
    return mName;
}
