#pragma once
#  define SET_BINARY_MODE(file)

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"

static constexpr size_t chunk = 16384;
int compress(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[chunk];
    unsigned char out[chunk];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    
            assert(ret != Z_STREAM_ERROR); 
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     

    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);       

    (void)deflateEnd(&strm);
    return Z_OK;
}