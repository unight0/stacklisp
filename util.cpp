#include <fstream>
#include <cstring>
#include "util.hpp"


size_t file_size(std::ifstream *f) {
    f->seekg(0,std::ios::end);
    size_t sz = f->tellg();
    f->seekg(0);
    return sz;
}

char *readfile(const char *filename, size_t *sz_buff) {
    using namespace std;
    ifstream file(filename, ios::binary | ios::in);
    char *contents = NULL;

    // Error occured
    if (!file.is_open()) {
	printf("Unable to open file '%s': %s\n", filename, strerror(errno));
	return NULL;
    }

    // Allocate buffer of appropriate size
    *sz_buff = file_size(&file);
    contents = (char*)malloc(*sz_buff);

    // Read file into buffer
    file.read(contents, *sz_buff);

    /*
    for (size_t i = 0; i < fsz; i++) {
	putchar(contents[i]);
    }
    */

    return contents;
}

bool is_big_endian() {
    int v = 1;
    // Little endian
    if (*(char*) &v == 1) return false;
    
    return true;
}
