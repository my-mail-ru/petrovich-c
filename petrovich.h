/**
 * @file petrovich.h
 *
 * @copyright Copyright (c) Mail.Ru Group, 2016. All rights reserved.
 */

#ifndef PETROVICH_H
#define PETROVICH_H

#include <stddef.h>

typedef enum {
        CASE_NOMINATIVE,
        CASE_GENITIVE,
        CASE_DATIVE,
        CASE_ACCUSATIVE,
        CASE_INSTRUMENTAL,
        CASE_PREPOSITIONAL
} petr_case_t;

typedef enum {
        GEND_NONE,
        GEND_MALE,
        GEND_FEMALE,
        GEND_ANDROGYNOUS
} petr_gender_t;

typedef struct petr_context petr_context_t;

int petr_init_from_file(const char *path, petr_context_t **ctx);
int petr_init_from_string(const char *data, size_t len, petr_context_t **ctx);
void petr_free_context(petr_context_t *ctx);

int petr_inflect_first_name(petr_context_t *pctx,
                            const char *data, size_t len,
                            petr_gender_t gender, petr_case_t dest_case,
                            char *dest, size_t dest_buf_size, size_t *dest_len);

#endif
