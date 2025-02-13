#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>

size_t file_size(std::ifstream *f);
char *readfile(const char *filename, size_t *sz_buff);
bool is_big_endian();

#endif
