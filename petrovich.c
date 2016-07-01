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
#define GENDER_COUNT            (GEND_ANDROGYNOUS + 1)
#define CASE_COUNT              (CASE_PREPOSITIONAL + 1)

#ifdef NDEBUG
#define debug_err(...) ((void)(0 && printf(__VA_ARGS__)))
#else
#define debug_err(...)                        \
        do {                                  \
                fprintf(stderr, __VA_ARGS__); \
                abort();                      \
        } while (0)
#endif

typedef struct {
        char *data;
        size_t len;
} buf_t;

typedef struct {
        const char *data;
        size_t len;
} cbuf_t;

typedef struct {
        size_t cnt_remove;
        cbuf_t add_suffix;
} mod_t;

typedef struct {
        size_t num_matches;
        cbuf_t *match;
        mod_t mods[CASE_COUNT - 1];
        bool first_word;
} mod_rule_t;

typedef struct {
        size_t num_rules;
        mod_rule_t *rules;
} mod_rule_arr_t;

typedef struct {
        petr_gender_t gender;
        const yaml_node_t *test;
        const yaml_node_t *mods;
        const yaml_node_t *tags;
} yaml_mod_rules_t;

typedef struct {
        mod_rule_arr_t suffixes;
        mod_rule_arr_t exceptions;
} rules_set_t;

struct petr_context {
        yaml_document_t yaml;
        rules_set_t sets[NAME_KIND_COUNT][GENDER_COUNT];
};

int petr_init_from_file(const char *path, petr_context_t **ctx)
{
        int rc = -1;
        FILE *fp = fopen(path, "r");
        if (!fp) {
                debug_err("failed to open rules file");
                goto out;
        }
        fseek(fp, 0, SEEK_END);
        uint64_t sz = ftell(fp);
        if (!sz) {
                debug_err("empty rules file");
                goto close_file;
        }
        fseek(fp, 0, SEEK_SET);
        char *buf = (char *)malloc(sz);
        if (!buf)
                goto close_file;
        uint64_t cnt_read = fread(buf, 1, sz, fp);
        if (cnt_read != sz) {
                debug_err("failed to read rules");
                goto free_mem;
        }
        rc = petr_init_from_string(buf, sz, ctx);
free_mem:
        free(buf);
close_file:
        fclose(fp);
out:
        return rc;
}

/**
 * Parse one rule set
 */
static int parse_mod_rules(petr_context_t *ctx, const yaml_node_t *node, yaml_mod_rules_t *dest)
{
        bool have_gender = false;
        if (node->type != YAML_MAPPING_NODE) {
                debug_err("node is not a mapping");
                return ERR_INVALID_RULES;
        }

        memset(dest, 0, sizeof(yaml_mod_rules_t));
        for (yaml_node_pair_t *p = node->data.mapping.pairs.start; p != node->data.mapping.pairs.top; p++) {
                yaml_node_t *key_node = yaml_document_get_node(&ctx->yaml, p->key);
                yaml_node_t *val_node = yaml_document_get_node(&ctx->yaml, p->value);
                if (key_node->type != YAML_SCALAR_NODE) {
                        debug_err("invalid key");
                        return ERR_INVALID_RULES;
                }

                const char *key = (const char *)key_node->data.scalar.value;
                size_t key_len = key_node->data.scalar.length;
                if (strncmp("gender", key, key_len) == 0) {
                        if (val_node->type != YAML_SCALAR_NODE) {
                                debug_err("gender is not a scalar");
                                return ERR_INVALID_RULES;
                        }
                        const char *val = (const char *)val_node->data.scalar.value;
                        size_t val_len = val_node->data.scalar.length;
                        if (strncmp("male", val, val_len) == 0) {
                                dest->gender = GEND_MALE;
                        } else if (strncmp("female", val, val_len) == 0) {
                                dest->gender = GEND_FEMALE;
                        } else if (strncmp("androgynous", val, val_len) == 0) {
                                dest->gender = GEND_ANDROGYNOUS;
                        } else {
                                debug_err("invalid gender '%.*s'", (int)val_len, val);
                                return ERR_INVALID_RULES;
                        }
                        have_gender = true;
                } else if (strncmp("test", key, key_len) == 0) {
                        dest->test = val_node;
                } else if (strncmp("mods", key, key_len) == 0) {
                        dest->mods = val_node;
                } else if (strncmp("tags", key, key_len) == 0) {
                        dest->tags = val_node;
                } else {
                        debug_err("unexpected key '%.*s'", (int)key_len, key);
                        return ERR_INVALID_RULES;
                }
        }

        if (!have_gender || dest->test == NULL || dest->mods == NULL) {
                debug_err("mandatory element missing");
                return ERR_INVALID_RULES;
        }
        return 0;
}

/**
 * Load data from one yaml_mod_rules_t instance
 */
static int load_mod_rules(petr_context_t *ctx, const yaml_mod_rules_t *parsed_node, mod_rule_t *dest)
{
        const yaml_node_t *test = parsed_node->test;
        if (test->type != YAML_SEQUENCE_NODE) {
                debug_err("test node is not an array");
                return ERR_INVALID_RULES;
        }

        size_t num_test = test->data.sequence.items.top - test->data.sequence.items.start;
        if (num_test == 0) {
                debug_err("test array is empty");
                return ERR_INVALID_RULES;
        }

        dest->num_matches = num_test;
        dest->match = calloc(sizeof(cbuf_t), num_test);
        if (dest->match == NULL) {
                debug_err("allocation failed");
                return ERR_NOMEM;
        }

        cbuf_t *dest_buf = dest->match;
        for (yaml_node_item_t *p = test->data.sequence.items.start; p != test->data.sequence.items.top;
             p++, dest_buf++) {
                yaml_node_t *node = yaml_document_get_node(&ctx->yaml, *p);
                if (node->type != YAML_SCALAR_NODE) {
                        debug_err("invalid node type");
                        return ERR_INVALID_RULES;
                }

                dest_buf->data = (const char *)node->data.scalar.value;
                dest_buf->len = node->data.scalar.length;
        }

        const yaml_node_t *mods = parsed_node->mods;
        if (mods->type != YAML_SEQUENCE_NODE) {
                debug_err("mods node is not an array");
                return ERR_INVALID_RULES;
        }
        size_t num_mods = mods->data.sequence.items.top - mods->data.sequence.items.start;
        if (num_mods != CASE_COUNT - 1) {
                debug_err("invalid number of mods %zu", num_mods);
                return ERR_INVALID_RULES;
        }

        mod_t *dest_mod = dest->mods;
        for (yaml_node_item_t *p = mods->data.sequence.items.start; p != mods->data.sequence.items.top;
             p++, dest_mod++) {
                yaml_node_t *node = yaml_document_get_node(&ctx->yaml, *p);
                if (node->type != YAML_SCALAR_NODE) {
                        debug_err("invalid node type");
                        return ERR_INVALID_RULES;
                }
                cbuf_t buf;
                buf.data = (const char *)node->data.scalar.value;
                buf.len = node->data.scalar.length;
                if (buf.len == 1 && buf.data[0] == '.') {
                        /* '.' means 'no change needed */
                        continue;
                }
                size_t minus_count;
                for (minus_count = 0; minus_count < buf.len && buf.data[minus_count] == '-'; minus_count++)
                        ;
                dest_mod->cnt_remove = minus_count;
                dest_mod->add_suffix.data = buf.data + minus_count;
                dest_mod->add_suffix.len = buf.len - minus_count;
        }

        const yaml_node_t *tags = parsed_node->tags;
        if (tags == NULL)
                return 0;

        if (tags->type != YAML_SEQUENCE_NODE) {
                debug_err("tags node is not an array");
                return ERR_INVALID_RULES;
        }

        for (yaml_node_item_t *p = tags->data.sequence.items.start; p != tags->data.sequence.items.top; p++) {
                yaml_node_t *node = yaml_document_get_node(&ctx->yaml, *p);
                if (node->type != YAML_SCALAR_NODE) {
                        debug_err("invalid node type");
                        return ERR_INVALID_RULES;
                }
                const char *val = (const char *)node->data.scalar.value;
                size_t val_len = node->data.scalar.length;
                if (strncmp("first_word", val, val_len) == 0) {
                        dest->first_word = true;
                } else {
                        debug_err("invalid tag '%.*s'", (int)val_len, val);
                        return ERR_INVALID_RULES;
                }
        }
        return 0;
}

/**
 * Load rules for a single (kind, gender) pair
 */
static int load_gender(petr_context_t *ctx, petr_gender_t gender, const yaml_node_t *node, mod_rule_arr_t *dest)
{
        if (node->type != YAML_SEQUENCE_NODE) {
                debug_err("node is not an array");
                return ERR_INVALID_RULES;
        }

        /* Count how many items to allocate. */
        size_t cnt_items = 0;
        for (yaml_node_item_t *p = node->data.sequence.items.start; p != node->data.sequence.items.top; p++) {
                yaml_node_t *node = yaml_document_get_node(&ctx->yaml, *p);
                yaml_mod_rules_t parsed_node;
                int rc = parse_mod_rules(ctx, node, &parsed_node);
                if (rc != 0)
                        return rc;
                if (parsed_node.gender == gender)
                        cnt_items++;
        }

        dest->rules = calloc(sizeof(mod_rule_t), cnt_items);
        if (!dest->rules) {
                debug_err("allocation failed");
                return ERR_NOMEM;
        }

        dest->num_rules = cnt_items;
        mod_rule_t *dest_item = dest->rules;
        /* Iterate again and load. */
        for (yaml_node_item_t *p = node->data.sequence.items.start; p != node->data.sequence.items.top; p++) {
                yaml_node_t *node = yaml_document_get_node(&ctx->yaml, *p);
                yaml_mod_rules_t parsed_node;
                parse_mod_rules(ctx, node, &parsed_node);
                if (parsed_node.gender != gender)
                        continue;
                int rc = load_mod_rules(ctx, &parsed_node, dest_item);
                if (rc != 0)
                        return rc;
                dest_item++;
        }

        return 0;
}

/**
 * Load rules for a single name kind
 */
static int load_name_kind(petr_context_t *ctx, const yaml_node_t *node, rules_set_t *dest)
{
        if (node->type != YAML_MAPPING_NODE) {
                debug_err("node is not a mapping");
                return ERR_INVALID_RULES;
        }

        for (yaml_node_pair_t *p = node->data.mapping.pairs.start; p != node->data.mapping.pairs.top; p++) {
                yaml_node_t *key_node = yaml_document_get_node(&ctx->yaml, p->key);
                yaml_node_t *val_node = yaml_document_get_node(&ctx->yaml, p->value);
                if (key_node->type != YAML_SCALAR_NODE) {
                        debug_err("invalid key");
                        return ERR_INVALID_RULES;
                }

                const char *key = (const char *)key_node->data.scalar.value;
                size_t key_len = key_node->data.scalar.length;
                bool is_suffixes;
                if (strncmp("exceptions", key, key_len) == 0) {
                        is_suffixes = false;
                } else if (strncmp("suffixes", key, key_len) == 0) {
                        is_suffixes = true;
                } else {
                        debug_err("unexpected kind '%.*s'", (int)key_len, key);
                        return ERR_INVALID_RULES;
                }

                for (int gender = 0; gender < GENDER_COUNT; gender++) {
                        rules_set_t *set = &dest[gender];
                        int rc = load_gender(ctx, gender, val_node, is_suffixes ? &set->suffixes : &set->exceptions);
                        if (rc != 0)
                                return rc;
                }
        }
        return 0;
}

/**
 * Load all rules into ctx
 */
static int load_yaml(petr_context_t *ctx)
{
        const yaml_node_t *root = yaml_document_get_root_node(&ctx->yaml);
        if (!root) {
                debug_err("empty root node");
                return ERR_INVALID_RULES;
        }
        if (root->type != YAML_MAPPING_NODE) {
                debug_err("root node is not a mapping");
                return ERR_INVALID_RULES;
        }

        /* Which rules sets are initialized. */
        bool init_rules[NAME_KIND_COUNT] = { 0 };

        /* Iterate over the root mapping node. */
        for (yaml_node_pair_t *p = root->data.mapping.pairs.start; p != root->data.mapping.pairs.top; p++) {
                yaml_node_t *key_node = yaml_document_get_node(&ctx->yaml, p->key);
                yaml_node_t *val_node = yaml_document_get_node(&ctx->yaml, p->value);
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

                int rc = load_name_kind(ctx, val_node, ctx->sets[kind]);
                if (rc != 0)
                        return rc;

                init_rules[kind] = true;
        }

        /* Check that the YAML document indeed contains all 3 rules sets. */
        for (int i = 0; i < NAME_KIND_COUNT; i++) {
                if (!init_rules[i]) {
                        debug_err("missing rules %d", i);
                        return ERR_INVALID_RULES;
                }
        }
        return 0;
}

#ifndef NDEBUG
static void dump_rule(const mod_rule_t *rule, FILE *fp)
{
        fprintf(fp, "        %zu matches:", rule->num_matches);
        for (size_t i = 0; i < rule->num_matches; i++)
                fprintf(fp, " '%.*s'", (int)rule->match[i].len, rule->match[i].data);
        fprintf(fp, "\n        mods:");
        for (size_t i = 0; i < CASE_COUNT - 1; i++) {
                const mod_t *mod = &rule->mods[i];
                fprintf(fp, " -%zu+'%.*s'", mod->cnt_remove, (int)mod->add_suffix.len, mod->add_suffix.data);
        }
        fprintf(fp, "\n");
}

static void dump_rules(const mod_rule_arr_t *arr, FILE *fp)
{
        for (size_t i = 0; i < arr->num_rules; i++) {
                fprintf(fp, "      rule %zu\n", i);
                const mod_rule_t *rule = &arr->rules[i];
                dump_rule(rule, fp);
        }
}

__attribute__((used))
static void dump_context(const petr_context_t *ctx, FILE *fp)
{
        for (int kind = 0; kind < NAME_KIND_COUNT; kind++) {
                switch (kind) {
                case NAME_FIRST:
                        fprintf(fp, "first name\n");
                        break;
                case NAME_MIDDLE:
                        fprintf(fp, "middle name\n");
                        break;
                case NAME_LAST:
                        fprintf(fp, "last name\n");
                        break;
                }

                for (int gender = 0; gender < GENDER_COUNT; gender++) {
                        switch (gender) {
                        case GEND_MALE:
                                fprintf(fp, "  male\n");
                                break;
                        case GEND_FEMALE:
                                fprintf(fp, "  female\n");
                                break;
                        case GEND_ANDROGYNOUS:
                                fprintf(fp, "  androgynous\n");
                                break;
                        }
                        fprintf(fp, "    exceptions\n");
                        dump_rules(&ctx->sets[kind][gender].exceptions, fp);
                        fprintf(fp, "    suffixes\n");
                        dump_rules(&ctx->sets[kind][gender].suffixes, fp);
                }
        }
}
#endif

int petr_init_from_string(const char *data, size_t len, petr_context_t **pctx)
{
        petr_context_t *ctx = (petr_context_t *)calloc(sizeof(petr_context_t), 1);
        if (!ctx)
                goto out;
        *pctx = ctx;
        yaml_parser_t parser;
        if (!yaml_parser_initialize(&parser))
                goto free_ctx;

        yaml_parser_set_input_string(&parser, (const unsigned char *)data, len);
        if (!yaml_parser_load(&parser, &ctx->yaml))
                goto del_parser;
        if (load_yaml(ctx) != 0) {
                yaml_parser_delete(&parser);
                petr_free_context(ctx);
                return -1;
        }
        yaml_parser_delete(&parser);
        return 0;
del_parser:
        yaml_parser_delete(&parser);
free_ctx:
        free(*pctx);
out:
        return -1;
}

static void free_rules_arr(mod_rule_arr_t *arr)
{
        if (!arr->rules)
                return;

        for (size_t i = 0; i < arr->num_rules; i++)
                free(arr->rules[i].match);
        free(arr->rules);
}

void petr_free_context(petr_context_t *ctx)
{
        for (size_t i = 0; i < NAME_KIND_COUNT; i++) {
                for (size_t j = 0; j < GENDER_COUNT; j++) {
                        rules_set_t *rules = &ctx->sets[i][j];
                        free_rules_arr(&rules->exceptions);
                        free_rules_arr(&rules->suffixes);
                }
        }
        yaml_document_delete(&ctx->yaml);
        free(ctx);
}

/**
 * Try to match the name against rules array
 *
 * @param arr Rules array
 * @param full_match If true, match full name, otherwise match ending
 * @param name Name string
 * @returns Matched rule, or NULL if not found
 */
static const mod_rule_t *match_rules(const mod_rule_arr_t *arr, bool full_match, cbuf_t name)
{
        for (size_t i = 0; i < arr->num_rules; i++) {
                const mod_rule_t *rule = &arr->rules[i];
                for (size_t j = 0; j < rule->num_matches; j++) {
                        cbuf_t match = rule->match[j];
                        if (match.len > name.len)
                                continue;
                        if (full_match && match.len != name.len)
                                continue;
                        if (memcmp(name.data + name.len - match.len, match.data, match.len) == 0)
                                return rule;
                }
        }
        return NULL;
}

static int copy_buf(cbuf_t src, buf_t dest, size_t *dest_len)
{
        if (dest.len < src.len + 1)
                return ERR_BUF;

        memcpy(dest.data, src.data, src.len);
        *dest_len = src.len;
        dest.data[src.len] = '\0';
        return 0;
}

static int append_buf(cbuf_t src, buf_t dest, size_t *dest_len)
{
        if (dest.len < src.len + *dest_len + 1)
                return ERR_BUF;

        memcpy(dest.data + *dest_len, src.data, src.len);
        (*dest_len) += src.len;
        dest.data[*dest_len] = '\0';
        return 0;
}

/**
 * Remove one UTF-8 codepoint from the end of the string.
 *
 * @returns Length of the result.
 */
static size_t pop_one_codepoint(cbuf_t str)
{
        while (str.len != 0 && (str.data[str.len - 1] & 0xC0) == 0x80)
                str.len--;
        return str.len;
}

static int apply_rule(const mod_t *mod, cbuf_t name, buf_t dest, size_t *dest_len)
{
        cbuf_t trimmed = name;
        for (size_t i = 0; i < mod->cnt_remove; i++)
                trimmed.len = pop_one_codepoint(trimmed);

        int rc = copy_buf(trimmed, dest, dest_len);
        if (rc != 0)
                return rc;
        return append_buf(mod->add_suffix, dest, dest_len);
}

static int do_inflect(const rules_set_t *rules, cbuf_t name, petr_case_t dest_case, buf_t dest, size_t *dest_len)
{
        if (dest_case == CASE_NOMINATIVE)
                return copy_buf(name, dest, dest_len);

        /* First try to search in exceptions. */
        const mod_rule_t *rule = match_rules(&rules->exceptions, true, name);
        /* If not found, search in suffixes. */
        if (rule == NULL)
                rule = match_rules(&rules->suffixes, false, name);
        /* If not found, copy as-is. */
        if (rule == NULL)
                return copy_buf(name, dest, dest_len);

        return apply_rule(&rule->mods[dest_case - 1], name, dest, dest_len);
}

int petr_inflect(const petr_context_t *ctx, const char *data, size_t len, petr_name_kind_t kind, petr_gender_t gender,
                 petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len)
{
        const rules_set_t *rules = &ctx->sets[kind][gender];
        buf_t dest_buf = { dest, dest_buf_size };
        cbuf_t name = { data, len };
        return do_inflect(rules, name, dest_case, dest_buf, dest_len);
}

int petr_inflect_first_name(const petr_context_t *ctx, const char *data, size_t len, petr_gender_t gender,
                            petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len)
{
        return petr_inflect(ctx, data, len, NAME_FIRST, gender, dest_case, dest, dest_buf_size, dest_len);
}

int petr_inflect_middle_name(const petr_context_t *ctx, const char *data, size_t len, petr_gender_t gender,
                             petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len)
{
        return petr_inflect(ctx, data, len, NAME_MIDDLE, gender, dest_case, dest, dest_buf_size, dest_len);
}

int petr_inflect_last_name(const petr_context_t *ctx, const char *data, size_t len, petr_gender_t gender,
                           petr_case_t dest_case, char *dest, size_t dest_buf_size, size_t *dest_len)
{
        return petr_inflect(ctx, data, len, NAME_LAST, gender, dest_case, dest, dest_buf_size, dest_len);
}
