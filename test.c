#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "petrovich.h"

static void print_usage(void)
{
        fprintf(stderr, "Usage: petr_test <type> <gender> name\n"
                        "    type: first | middle | last\n"
                        "    gender: male | female\n");
}

int main(int argc, char **argv)
{
        if (argc != 4) {
                fprintf(stderr, "Invalid number of arguments\n");
                goto err;
        }

        const char *kind_str = argv[1];
        petr_name_kind_t kind;
        if (strcmp(kind_str, "first") == 0) {
                kind = NAME_FIRST;
        } else if (strcmp(kind_str, "middle") == 0) {
                kind = NAME_MIDDLE;
        } else if (strcmp(kind_str, "last") == 0) {
                kind = NAME_LAST;
        } else {
                fprintf(stderr, "Invalid name kind\n");
                goto err;
        }

        petr_gender_t gender;
        const char *gender_str = argv[2];
        if (strcmp(gender_str, "male") == 0) {
                gender = GEND_MALE;
        } else if (strcmp(gender_str, "female") == 0) {
                gender = GEND_FEMALE;
        } else {
                fprintf(stderr, "Invalid gender\n");
                goto err;
        }

        const char *name_str = argv[3];
        size_t name_len = strlen(name_str);

        petr_context_t *ctx;
        assert(petr_init_from_file("rules.yml", &ctx) == 0);
        char buf[1024];
        for (int dest_case = CASE_NOMINATIVE; dest_case <= CASE_PREPOSITIONAL; dest_case++) {
                size_t res_size;
                assert(petr_inflect(ctx, name_str, name_len, kind, gender, dest_case, buf, sizeof(buf), &res_size)
                       == 0);
                printf("%.*s\n", (int)res_size, buf);
        }
        petr_free_context(ctx);
        return 0;
err:
        print_usage();
        return 1;
}
