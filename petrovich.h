/// @file petrovich.h
///
/// @copyright Copyright (c) Mail.Ru Group, 2016. All rights reserved. MIT License.

#ifndef PETROVICH_H
#define PETROVICH_H

#include <stddef.h>

typedef enum {
        ERR_INVALID_RULES       = -1,
        ERR_NOMEM               = -2,
        ERR_BUF                 = -3,
} petr_error_t;

typedef enum {
        NAME_FIRST,
        NAME_MIDDLE,
        NAME_LAST,
} petr_name_kind_t;

typedef enum {
        CASE_NOMINATIVE,
        CASE_GENITIVE,
        CASE_DATIVE,
        CASE_ACCUSATIVE,
        CASE_INSTRUMENTAL,
        CASE_PREPOSITIONAL
} petr_case_t;

typedef enum {
        GEND_MALE,
        GEND_FEMALE,
        GEND_ANDROGYNOUS
} petr_gender_t;

typedef struct petr_context petr_context_t;

int petr_init_from_file(const char *path, petr_context_t **ctx);
int petr_init_from_string(const char *data, size_t len, petr_context_t **ctx);
void petr_free_context(petr_context_t *ctx);

int petr_inflect(const petr_context_t *ctx, const char *data, size_t len, petr_name_kind_t kind, petr_gender_t gender,
                 petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len);
int petr_inflect_first_name(const petr_context_t *ctx, const char *data, size_t len, petr_gender_t gender,
                            petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len);
int petr_inflect_middle_name(const petr_context_t *ctx, const char *data, size_t len, petr_gender_t gender,
                             petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len);
int petr_inflect_last_name(const petr_context_t *ctx, const char *data, size_t len, petr_gender_t gender,
                           petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len);

#endif
