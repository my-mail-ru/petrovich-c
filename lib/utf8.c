/// @file utf8.c
///
/// @copyright Copyright (c) Mail.Ru Group, 2016. All rights reserved. MIT License.

#include <stdint.h>

#include "utf8.h"

/// Magic constants for get_codepoint
static const uint32_t g_offsets_for_utf8[4] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL };

/// Read one codepoint from a UTF-8 string
///
/// @param buf   Buffer containing the string
/// @param cp    Destination for codepoint
/// @return      Number of bytes read. 0 in case of error
static size_t get_codepoint(cbuf_t buf, uint32_t *cp)
{
        if (buf.len == 0)
                return 0;

        uint8_t lead = buf.data[0];
        size_t extra_len;
        if (lead < 0x80)
                extra_len = 0;
        else if ((lead >> 5) == 0x6)
                extra_len = 1;
        else if ((lead >> 4) == 0xe)
                extra_len = 2;
        else if ((lead >> 3) == 0x1e)
                extra_len = 3;
        else
                return 0;

        if (buf.len < extra_len + 1)
                return 0;

        uint32_t ch = 0;
        const char *source = buf.data;
        switch (extra_len) {
                case 3: ch += *source++; ch <<= 6;
                case 2: ch += *source++; ch <<= 6;
                case 1: ch += *source++; ch <<= 6;
                case 0: ch += *source++;
        }

        ch -= g_offsets_for_utf8[extra_len];
        *cp = ch;
        return extra_len + 1;
}

/// Convert a Russian unicode codepoint to lowercase. Return unchanged condepoint for all other languages.
static uint32_t rus_lowercase(uint32_t cp)
{
        if (cp >= 0x410 && cp <= 0x042F)
                return cp + 0x20;
        if (cp == 0x401)
                return 0x451;
        return cp;
}

/// Compare two Russian UTF-8 strings case-insensitive (other languages will be case-sensitive)
///
/// @returns    True, if strings are equal
bool rus_utf8_streq(cbuf_t s1, cbuf_t s2)
{
        while (s1.len != 0 && s2.len != 0) {
                uint32_t cp1, cp2;
                size_t l1 = get_codepoint(s1, &cp1);
                size_t l2 = get_codepoint(s2, &cp2);
                if (l1 == 0 && l2 == 0)
                        break;
                if (l1 == 0 || l2 == 0)
                        return false;
                if (rus_lowercase(cp1) != rus_lowercase(cp2))
                        return false;
                s1.data += l1;
                s1.len -= l1;
                s2.data += l2;
                s2.len -= l2;
        }
        return s1.len == 0 && s2.len == 0;
}
