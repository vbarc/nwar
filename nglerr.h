#pragma once

#define NGLCHKERR nglCheckError(__FILE__, __LINE__)

void nglCheckError(const char* file, int line);
