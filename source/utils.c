// Utility Functions
// Author: Evan Cassidy
// Date: 4/23/2026

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

char *ReadFile(const char *file, int *size) {
    FILE *f = fopen(file, "rb");

    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = (char *)malloc(*size);
    fread(data, 1, *size, f);

    fclose(f);
    return data;
}