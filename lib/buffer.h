/// @file buffer.h
///
/// @copyright Copyright (c) Mail.Ru Group, 2016. All rights reserved. MIT License.

#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <string.h>
#include "petrovich.h"  // For error codes

/// Mutable string buffer
typedef struct {
        char *data;
        size_t len;
} buf_t;

/// Constant string buffer
typedef struct {
        const char *data;
        size_t len;
} cbuf_t;

/// Copy \c src buffer to \c dest
static inline int copy_buf(cbuf_t src, buf_t dest, size_t *dest_len)
{
        if (dest.len < src.len + 1)
                return ERR_BUF;

        memcpy(dest.data, src.data, src.len);
        *dest_len = src.len;
        dest.data[src.len] = '\0';
        return 0;
}

/// Append \c src buffer to \c dest
static inline int append_buf(cbuf_t src, buf_t dest, size_t *dest_len)
{
        if (dest.len < src.len + *dest_len + 1)
                return ERR_BUF;

        memcpy(dest.data + *dest_len, src.data, src.len);
        (*dest_len) += src.len;
        dest.data[*dest_len] = '\0';
        return 0;
}

#endif
