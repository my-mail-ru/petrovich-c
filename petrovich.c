/**
 * @file petrovich.c
 *
 * @copyright Copyright (c) Mail.Ru Group, 2016. All rights reserved.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <yaml.h>

#include "petrovich.h"

typedef enum {
        FIRST_NAME,
        MIDDLE_NAME,
        LAST_NAME,
        NAME_KIND_COUNT,
} name_kind_t;

typedef struct {
        int xxx;
} rules_set_t;

struct petr_context {
        rules_set_t rules[NAME_KIND_COUNT];
};

int petr_init_from_file(const char *path, petr_context_t **ctx)
{
        int rc = -1;
        FILE *fp = fopen(path, "r");
        if (!fp)
                goto out;
        fseek(fp, 0, SEEK_END);
        uint64_t sz = ftell(fp);
        if (!sz)
                goto close_file;
        fseek(fp, 0, SEEK_SET);
        char *buf = (char *)malloc(sz);
        if (!buf)
                goto close_file;
        uint64_t cnt_read = fread(buf, 1, sz, fp);
        if (cnt_read != sz)
                goto free_mem;
        rc = petr_init_from_string(buf, sz, ctx);
free_mem:
        free(buf);
close_file:
        fclose(fp);
out:
        return rc;
}

int petr_init_from_string(const char *data, size_t len, petr_context_t **ctx)
{
        int rc = -1;
        yaml_parser_t parser;
        if (!yaml_parser_initialize(&parser))
                goto out;
        yaml_parser_set_input_string(&parser, (const unsigned char *)data, len);
        yaml_document_t document;
        if (!yaml_parser_load(&parser, &document))
                goto del_parser;
        (void)ctx;
// del_doc:
        yaml_document_delete(&document);
del_parser:
        yaml_parser_delete(&parser);
out:
        return rc;
}

void petr_free_context(petr_context_t *ctx)
{
        free(ctx);
}
