/**
 * @file utf8.h
 *
 * @copyright Copyright (c) Mail.Ru Group, 2016. All rights reserved.
 */

#ifndef UTF8_H
#define UTF8_H

#include <stdbool.h>
#include "buffer.h"

/**
 * Remove one UTF-8 codepoint from the end of the string.
 *
 * @returns Length of the result.
 */
static inline size_t pop_one_codepoint(cbuf_t str)
{
        do {
                if (str.len == 0)
                        return 0;
                str.len--;
        } while ((str.data[str.len - 1] & 0xC0) == 0x80);
        return str.len ? str.len - 1 : 0;
}

static inline size_t pop_n_codepoints(cbuf_t str, size_t n)
{
        for (size_t i = 0; i < n; i++)
                str.len = pop_one_codepoint(str);
        return str.len;
}

static inline size_t count_codepoints(cbuf_t str)
{
        size_t cnt = 0;
        while (str.len != 0) {
                str.len = pop_one_codepoint(str);
                cnt++;
        }
        return cnt;
}

bool rus_utf8_streq(cbuf_t s1, cbuf_t s2);

#endif
