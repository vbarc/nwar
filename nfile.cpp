#include "nfile.h"

#include <fstream>

#include "nglassert.h"

std::vector<char> nReadFile(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    NGL_VERIFY(file.is_open());
    size_t size = file.tellg();
    std::vector<char> result(size);
    file.seekg(0);
    file.read(result.data(), size);
    return result;
}
