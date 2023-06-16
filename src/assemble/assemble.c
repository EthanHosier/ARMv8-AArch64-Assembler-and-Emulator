#include "assemble.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    char str[] = "ldr x20, [x5, #8]";
    ArrayList *tokens = tokenize(str);
    print_ArrayList_elements(tokens);

    char line[] = "ldr x1 [x2]!";
    int number_of_lines = 1;
    char **lines =
            malloc(number_of_lines * sizeof(char *)); //readLines(file_name);
    lines[0] = line;
    ArrayList *token_lines = create_ArrayList(NULL, &free_ArrayList);
    for (int i = 0; i < number_of_lines; i++) {// maybe works
        ArrayList *tokens = tokenize(lines[i]);
        add_ArrayList_element(token_lines, tokens);
        print_ArrayList_elements(tokens);
    }
    free(lines);
    TreeMap *label_identifiers = first_pass(token_lines);
    ArrayList *trees = second_pass(token_lines, label_identifiers);
    free_ArrayList(token_lines);
    free_map(label_identifiers);
    // TODO: Make map of Parser_Tree.type to function pointers that do final step
    for (int i = 0; i < trees->size; i++) {
        Parser_Tree *tree = get_ArrayList_element(trees, i);
        switch (tree->type) {
            case Type_dotInt_b_bCond:
                break;
            case Type_br:
                break;
            case Type_cmp_cmn_neg_negs_IMM_movk_movn_movz_ldrlit:
                break;
            case Type_cmp_cmn_neg_negs_REG_tst_mov_mvn:
                break;
            case Type_add_sub_adds_subs_IMM:
                break;
            case Type_add_sub_adds_subs_REG_mul_mneg_logical:
                break;
            case Type_madd_msub:
                break;
            case Type_ldr_str_preIndex_postIndex_unsignedOffset:
                break;
            case Type_ldr_str_regOffset:
                break;
            case Type_nop:
                break;
            default:
                fprintf(stderr, "Invalid parser tree!");
                exit(1);
        }
    }
    free_ArrayList(trees);
    return EXIT_SUCCESS;
}
