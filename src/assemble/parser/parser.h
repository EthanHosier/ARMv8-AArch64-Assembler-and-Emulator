#ifndef PARSER_H
#define PARSER_H

#include "../token/token.h"

typedef enum {
  TREE_REGISTER_DP,
  TREE_IMMEDIATE_DP,
  TREE_SINGLE_DATA_TRANSFER,
  TREE_LOAD_LITERAL,
  TREE_BRANCH
} tree_type;

typedef struct __register_dp{} Register_DP_Tree;
typedef struct __immediate_dp{} Immediate_DP_Tree;
typedef struct __single_data_transfer{} Single_Data_Transfer_Tree;
typedef struct __load_literal{} Load_Literal_Tree;
typedef struct __branch{} Branch_Tree;

typedef struct {
  tree_type type;
  union{
    Register_DP_Tree register_tree;
    Immediate_DP_Tree immediate_tree;
    Single_Data_Transfer_Tree single_tree;
    Load_Literal_Tree load_tree;
    Branch_Tree branch_tree;
  };
} Parser_Tree;

extern Parser_Tree *parse(Token *tokens);

#endif
