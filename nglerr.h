#pragma once

#define NGL_CHECK_ERRORS nglCheckErrors(__FILE__, __LINE__)

void nglCheckErrors(const char* file, int line);
