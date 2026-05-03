#pragma once

#include "../posting.h"

typedef enum {
    TREE_AVL,
    TREE_RB,
    TREE_BTREE
} TreeType;

typedef struct {
    void*    tree;
    TreeType type;
} Index;

/* CLI helpers */
TreeType    parseType(const char* s);
const char* typeName(TreeType type);

/* Index lifecycle */
Index* createIndex(TreeType type);
void   freeIndex(Index* idx);

/* Basic operations */
void    insertTerm(Index* idx, const char* term, int doc_id, const char* title);
Vector* lookupTerm(const Index* idx, const char* term);

void indexDocument(
    Index* idx,
    int doc_id,
    const char* title,
    const char** tokens,
    int n_tokens
);

void traverseIndex(
    const Index* idx,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
);

/* Persistence */
void   saveIndex(const Index* idx, const char* path);
Index* loadIndex(const char* path, TreeType type);

/* Full indexing pipeline */
void runIndex(TreeType type, const char* data_path, const char* idx_path);