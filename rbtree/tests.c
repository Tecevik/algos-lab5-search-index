#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rbtree.h"

#define ASSERT(cond, msg)                     \
    do {                                      \
        if (!(cond)) {                        \
            fprintf(stderr,                   \
                "ASSERT FAILED: %s\n", msg);  \
            exit(1);                          \
        }                                     \
    } while (0)

static int checkRedViolation(const RBTree* tree, RBNode* node) {
    if (node == tree->nil) {
        return 1;
    }

    if (node->color == RB_RED) {

        if (node->left->color == RB_RED) {
            return 0;
        }

        if (node->right->color == RB_RED) {
            return 0;
        }
    }

    return checkRedViolation(tree, node->left)
        && checkRedViolation(tree, node->right);
}

static int checkBlackHeight(
    const RBTree* tree,
    RBNode* node
) {
    if (node == tree->nil) {
        return 1;
    }

    int left = checkBlackHeight(tree, node->left);
    int right = checkBlackHeight(tree, node->right);

    ASSERT(left == right, "black height mismatch");

    return left + (node->color == RB_BLACK ? 1 : 0);
}

typedef struct {
    const char* expected[32];
    int index;
} TraverseCtx;

static void visitCheckOrder(
    const char* key,
    Vector* postings,
    void* ctx
) {
    (void)postings;

    TraverseCtx* tctx = (TraverseCtx*)ctx;

    ASSERT(
        strcmp(key, tctx->expected[tctx->index]) == 0,
        "in-order traversal broken"
    );

    tctx->index++;
}

static void testCreateTree(void) {
    RBTree* tree = createRBTree();

    ASSERT(tree != NULL, "tree create failed");
    ASSERT(tree->nil != NULL, "nil not created");
    ASSERT(tree->root == tree->nil, "root must equal nil");
    ASSERT(tree->nil->color == RB_BLACK, "nil must be black");
    ASSERT(tree->size == 0, "initial size must be 0");

    freeRBTree(tree);

    printf("[OK] testCreateTree\n");
}

static void testInsertSearch(void) {
    RBTree* tree = createRBTree();

    rbInsert(tree, "cat", 1, "doc1");

    Vector* postings = rbSearch(tree, "cat");

    ASSERT(postings != NULL, "search failed");
    ASSERT(vectorSize(postings) == 1, "posting size invalid");

    PostingEntry* entry = getPostingEntry(postings, 0);

    ASSERT(entry != NULL, "posting entry null");
    ASSERT(entry->doc_id == 1, "wrong doc id");
    ASSERT(strcmp(entry->title, "doc1") == 0, "wrong title");

    freeRBTree(tree);

    printf("[OK] testInsertSearch\n");
}

static void testDuplicateKey(void) {
    RBTree* tree = createRBTree();

    rbInsert(tree, "cat", 1, "doc1");
    rbInsert(tree, "cat", 2, "doc2");

    ASSERT(tree->size == 1, "duplicate key created new node");

    Vector* postings = rbSearch(tree, "cat");

    ASSERT(postings != NULL, "postings null");
    ASSERT(vectorSize(postings) == 2, "posting append failed");

    PostingEntry* p0 = getPostingEntry(postings, 0);
    PostingEntry* p1 = getPostingEntry(postings, 1);

    ASSERT(p0->doc_id == 1, "wrong posting 0");
    ASSERT(p1->doc_id == 2, "wrong posting 1");

    freeRBTree(tree);

    printf("[OK] testDuplicateKey\n");
}

static void testRootBlack(void) {
    RBTree* tree = createRBTree();

    rbInsert(tree, "d", 1, "d");
    rbInsert(tree, "b", 2, "b");
    rbInsert(tree, "f", 3, "f");
    rbInsert(tree, "a", 4, "a");
    rbInsert(tree, "c", 5, "c");

    ASSERT(
        tree->root->color == RB_BLACK,
        "root is not black"
    );

    freeRBTree(tree);

    printf("[OK] testRootBlack\n");
}

static void testNoRedRedViolation(void) {
    RBTree* tree = createRBTree();

    const char* keys[] = {
        "m", "b", "q", "z", "a",
        "c", "x", "k", "p", "r"
    };

    for (int i = 0; i < 10; i++) {
        rbInsert(tree, keys[i], i, keys[i]);
    }

    ASSERT(
        checkRedViolation(tree, tree->root),
        "red-red violation detected"
    );

    freeRBTree(tree);

    printf("[OK] testNoRedRedViolation\n");
}

static void testBlackHeight(void) {
    RBTree* tree = createRBTree();

    const char* keys[] = {
        "m", "b", "q", "z", "a",
        "c", "x", "k", "p", "r"
    };

    for (int i = 0; i < 10; i++) {
        rbInsert(tree, keys[i], i, keys[i]);
    }

    checkBlackHeight(tree, tree->root);

    freeRBTree(tree);

    printf("[OK] testBlackHeight\n");
}

static void testTraversalOrder(void) {
    RBTree* tree = createRBTree();

    rbInsert(tree, "dog", 1, "d");
    rbInsert(tree, "apple", 2, "a");
    rbInsert(tree, "zebra", 3, "z");
    rbInsert(tree, "cat", 4, "c");

    TraverseCtx ctx = {
        .expected = {
            "apple",
            "cat",
            "dog",
            "zebra"
        },
        .index = 0
    };

    rbTraverse(tree, visitCheckOrder, &ctx);

    ASSERT(ctx.index == 4, "traversal count mismatch");

    freeRBTree(tree);

    printf("[OK] testTraversalOrder\n");
}

static void testSearchMissing(void) {
    RBTree* tree = createRBTree();

    rbInsert(tree, "cat", 1, "doc");

    Vector* result = rbSearch(tree, "missing");

    ASSERT(result == NULL, "missing search must return NULL");

    freeRBTree(tree);

    printf("[OK] testSearchMissing\n");
}

static void testDuplicateDocIgnored(void) {
    RBTree* tree = createRBTree();

    rbInsert(tree, "cat", 1, "doc1");
    rbInsert(tree, "cat", 1, "doc1");

    Vector* postings = rbSearch(tree, "cat");

    ASSERT(postings != NULL, "postings null");
    ASSERT(
        vectorSize(postings) == 1,
        "duplicate doc should be ignored"
    );

    freeRBTree(tree);

    printf("[OK] testDuplicateDocIgnored\n");
}

int main(void) {
    testCreateTree();
    testInsertSearch();
    testDuplicateKey();
    testRootBlack();
    testNoRedRedViolation();
    testBlackHeight();
    testTraversalOrder();
    testSearchMissing();
    testDuplicateDocIgnored();

    printf("\nALL RB-TREE TESTS PASSED\n");

    return 0;
}