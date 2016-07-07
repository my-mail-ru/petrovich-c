/// @file petrovich.h
///
/// @copyright Copyright (c) Mail.Ru Group, 2016. All rights reserved. MIT License.

#ifndef PETROVICH_H
#define PETROVICH_H

#include <stddef.h>

#define PETR_VISIBLE    __attribute__((visibility("default")))

/// Error codes
typedef enum {
        ERR_INVALID_RULES       = -1,   ///< Invalid 'rules.yml' file
        ERR_NOMEM               = -2,   ///< Failed to allocate memory
        ERR_BUF                 = -3,   ///< Output buffer too small
        ERR_FILE                = -4,   ///< Error reading file
} petr_error_t;

/// Type of name
typedef enum {
        NAME_FIRST,
        NAME_MIDDLE,
        NAME_LAST,
} petr_name_kind_t;

/// Grammatical case
typedef enum {
        CASE_NOMINATIVE,
        CASE_GENITIVE,
        CASE_DATIVE,
        CASE_ACCUSATIVE,
        CASE_INSTRUMENTAL,
        CASE_PREPOSITIONAL
} petr_case_t;

/// Grammatical gender
typedef enum {
        GEND_MALE,
        GEND_FEMALE,
        GEND_ANDROGYNOUS
} petr_gender_t;

typedef struct petr_context petr_context_t;

#ifdef __cplusplus
extern "C" {
#endif

PETR_VISIBLE
int petr_init_from_file(const char *path, petr_context_t **pctx);

PETR_VISIBLE
int petr_init_from_string(const char *data, size_t len, petr_context_t **pctx);

PETR_VISIBLE
void petr_free_context(petr_context_t *ctx);

PETR_VISIBLE
int petr_inflect(const petr_context_t *ctx, const char *data, size_t len, petr_name_kind_t kind, petr_gender_t gender,
                 petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len);

PETR_VISIBLE
int petr_inflect_first_name(const petr_context_t *ctx, const char *data, size_t len, petr_gender_t gender,
                            petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len);

PETR_VISIBLE
int petr_inflect_middle_name(const petr_context_t *ctx, const char *data, size_t len, petr_gender_t gender,
                             petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len);

PETR_VISIBLE
int petr_inflect_last_name(const petr_context_t *ctx, const char *data, size_t len, petr_gender_t gender,
                           petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len);

#ifdef __cplusplus
}
#endif

#endif
