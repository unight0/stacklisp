#include "utils.h"

#include <stdio.h>

char *readfile(const char *filename, size_t *sz) {
    FILE *f = fopen(filename, "rb");
    char *contents = NULL;

    // Error occured
    PERRET(f == NULL, NULL);

    while(!feof(f)) {
	contents = realloc(contents, ++(*sz));
	contents[*sz-1] = fgetc(f);
    }

    // Remove EOF

    contents = realloc(contents, --(*sz));

    fclose(f);
    
    return contents;
}
