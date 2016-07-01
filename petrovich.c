/**
 * @file petrovich.c
 *
 * @copyright Copyright (c) Mail.Ru Group, 2016. All rights reserved.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <yaml.h>

#include "petrovich.h"

#define NAME_KIND_COUNT         (NAME_LAST + 1)
#define GENDERS_COUNT           (GEND_ANDROGYNOUS + 1)
#define CASES_COUNT             (CASE_PREPOSITIONAL + 1)

#define debug_err(...)          fprintf(stderr, __VA_ARGS__)

typedef struct {
        char *data;
        size_t len;
} buf_t;

typedef struct {
        const char *data;
        size_t len;
} cbuf_t;

typedef struct {
        int xxx;
} rules_set_t;

struct petr_context {
        yaml_document_t yaml;
        rules_set_t rules[NAME_KIND_COUNT][GENDERS_COUNT];
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

/**
 * Load rules for a single name kind
 */
static int load_name_kind(petr_context_t *ctx, const yaml_node_t *node, rules_set_t *dest)
{
        (void)ctx;
        (void)node;
        (void)dest;
        return 0;
}

/**
 * Load all rules into ctx
 */
static int load_yaml(petr_context_t *ctx)
{
        const yaml_node_t *root = yaml_document_get_root_node(&ctx->yaml);
        if (!root)
                return -1;
        if (root->type != YAML_MAPPING_NODE)
                return -1;

        bool init_rules[NAME_KIND_COUNT] = { 0 };
        for (yaml_node_pair_t *p = root->data.mapping.pairs.start; p != root->data.mapping.pairs.end; p++) {
                yaml_node_t *key_node = yaml_document_get_node(&ctx->yaml, p->key);
                yaml_node_t *val_node = yaml_document_get_node(&ctx->yaml, p->value);
                if (key_node == NULL || val_node == NULL)
                        break;
                if (key_node->type != YAML_SCALAR_NODE) {
                        debug_err("invalid key");
                        return ERR_INVALID_RULES;
                }
                const char *key = (const char *)key_node->data.scalar.value;
                size_t key_len = key_node->data.scalar.length;
                petr_name_kind_t kind;
                if (strncmp("firstname", key, key_len) == 0) {
                        kind = NAME_FIRST;
                } else if (strncmp("middlename", key, key_len) == 0) {
                        kind = NAME_MIDDLE;
                } else if (strncmp("lastname", key, key_len) == 0) {
                        kind = NAME_LAST;
                } else {
                        debug_err("unexpected kind '%.*s'", (int)key_len, key);
                        return ERR_INVALID_RULES;
                }
                int rc = load_name_kind(ctx, val_node, ctx->rules[kind]);
                if (rc != 0)
                        return rc;
                init_rules[kind] = true;
        }
        for (int i = 0; i < NAME_KIND_COUNT; i++) {
                if (!init_rules[i]) {
                        debug_err("missing rules %d", i);
                        return ERR_INVALID_RULES;
                }
        }
        return 0;
}

int petr_init_from_string(const char *data, size_t len, petr_context_t **pctx)
{
        petr_context_t *ctx = (petr_context_t *)calloc(sizeof(petr_context_t), 1);
        if (!ctx)
                goto out;
        *pctx = ctx;
        yaml_parser_t parser;
        if (!yaml_parser_initialize(&parser))
                goto del_ctx;

        yaml_parser_set_input_string(&parser, (const unsigned char *)data, len);
        if (!yaml_parser_load(&parser, &ctx->yaml))
                goto del_parser;
        if (load_yaml(ctx) != 0)
                goto del_parser;
        return 0;
del_parser:
        yaml_parser_delete(&parser);
del_ctx:
        petr_free_context(*pctx);
out:
        return -1;
}

void petr_free_context(petr_context_t *ctx)
{
        free(ctx);
}

static int do_inflect(rules_set_t *rules, cbuf_t name, petr_case_t dest_case, buf_t dest, size_t *dest_len)
{
        (void)rules;
        (void)name;
        (void)dest_case;
        (void)dest;
        *dest_len = 0;
        return 0;
}

int petr_inflect(petr_context_t *pctx, const char *data, size_t len, petr_name_kind_t kind, petr_gender_t gender,
                 petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len)
{
        rules_set_t *rules = &pctx->rules[kind][gender];
        buf_t dest_buf = { dest, dest_buf_size };
        cbuf_t name = { data, len };
        return do_inflect(rules, name, dest_case, dest_buf, dest_len);
}

int petr_inflect_first_name(petr_context_t *pctx, const char *data, size_t len, petr_gender_t gender,
                            petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len)
{
        return petr_inflect(pctx, data, len, NAME_FIRST, gender, dest_case, dest, dest_buf_size, dest_len);
}

int petr_inflect_middle_name(petr_context_t *pctx, const char *data, size_t len, petr_gender_t gender,
                             petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len)
{
        return petr_inflect(pctx, data, len, NAME_MIDDLE, gender, dest_case, dest, dest_buf_size, dest_len);
}

int petr_inflect_last_name(petr_context_t *pctx, const char *data, size_t len, petr_gender_t gender,
                           petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len)
{
        return petr_inflect(pctx, data, len, NAME_LAST, gender, dest_case, dest, dest_buf_size, dest_len);
}
