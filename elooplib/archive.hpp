#pragma once

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

int compress(FILE *, FILE *, int);
int decompress(FILE *, FILE *);