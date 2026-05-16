/*
 * Unit-тесты для B-tree.
 *
 * Проверяем:
 * 1. Создание дерева.
 * 2. Вставку одного ключа.
 * 3. Вставку нескольких ключей.
 * 4. Split корня.
 * 5. Split внутреннего узла.
 * 6. Поиск существующего ключа.
 * 7. Поиск несуществующего ключа.
 * 8. Повторную вставку ключа.
 * 9. Корректный отсортированный обход.
 * 10. Освобождение памяти.
 */

#include "btree.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char keys[512][64];
    int  count;
} TraverseCtx;

static void collectKeys(const char* key, Vector* postings, void* ctx) {
    (void)postings;

    TraverseCtx* tctx = (TraverseCtx*)ctx;

    strncpy(tctx->keys[tctx->count], key, 63);
    tctx->keys[tctx->count][63] = '\0';
    tctx->count++;
}

static void validateNode(const BTreeNode* node, int is_root) {
    if (!node) return;

    if (is_root) {
        assert(node->n >= 0);
        assert(node->n <= BTREE_MAX_KEYS);
    } else {
        assert(node->n >= BTREE_T - 1);
        assert(node->n <= BTREE_MAX_KEYS);
    }

    for (int i = 1; i < node->n; i++) {
        assert(strcmp(node->keys[i - 1], node->keys[i]) < 0);
    }

    if (!node->is_leaf) {
        for (int i = 0; i <= node->n; i++) {
            assert(node->children[i] != NULL);
            validateNode(node->children[i], 0);
        }
    }
}

static void test_create_tree(void) {
    BTree* tree = createBTree();

    assert(tree != NULL);
    assert(tree->root == NULL);
    assert(tree->size == 0);

    freeBTree(tree);
}

static void test_insert_one_key(void) {
    BTree* tree = createBTree();

    btreeInsert(tree, "python", 1, "Python question");

    assert(tree->root != NULL);
    assert(tree->size == 1);
    assert(tree->root->n == 1);
    assert(strcmp(tree->root->keys[0], "python") == 0);

    Vector* found = btreeSearch(tree, "python");

    assert(found != NULL);
    assert(getPostingListSize(found) == 1);

    PostingEntry* entry = getPostingEntry(found, 0);

    assert(entry != NULL);
    assert(entry->doc_id == 1);
    assert(strcmp(entry->title, "Python question") == 0);

    vectorFree(found);
    freeBTree(tree);
}

static void test_insert_many_keys(void) {
    BTree* tree = createBTree();

    btreeInsert(tree, "python", 1, "Python question");
    btreeInsert(tree, "java", 2, "Java question");
    btreeInsert(tree, "c", 3, "C question");
    btreeInsert(tree, "rust", 4, "Rust question");
    btreeInsert(tree, "go", 5, "Go question");

    assert(tree->size == 5);
    validateNode(tree->root, 1);

    Vector* found = NULL;

    found = btreeSearch(tree, "python");
    assert(found != NULL);
    vectorFree(found);

    found = btreeSearch(tree, "java");
    assert(found != NULL);
    vectorFree(found);

    found = btreeSearch(tree, "c");
    assert(found != NULL);
    vectorFree(found);

    found = btreeSearch(tree, "rust");
    assert(found != NULL);
    vectorFree(found);

    found = btreeSearch(tree, "go");
    assert(found != NULL);
    vectorFree(found);

    freeBTree(tree);
}

static void test_root_split(void) {
    BTree* tree = createBTree();

    btreeInsert(tree, "a", 1, "A");
    btreeInsert(tree, "b", 2, "B");
    btreeInsert(tree, "c", 3, "C");
    btreeInsert(tree, "d", 4, "D");
    btreeInsert(tree, "e", 5, "E");

    assert(tree->root != NULL);
    assert(tree->root->is_leaf == 1);
    assert(tree->root->n == BTREE_MAX_KEYS);

    btreeInsert(tree, "f", 6, "F");

    assert(tree->size == 6);
    assert(tree->root->is_leaf == 0);
    assert(tree->root->n == 1);
    assert(tree->root->children[0] != NULL);
    assert(tree->root->children[1] != NULL);

    validateNode(tree->root, 1);

    Vector* found = btreeSearch(tree, "f");
    assert(found != NULL);
    vectorFree(found);

    freeBTree(tree);
}

static void test_internal_split(void) {
    BTree* tree = createBTree();

    const char* keys[] = {
        "a", "b", "c", "d", "e",
        "f", "g", "h", "i", "j",
        "k", "l", "m", "n", "o",
        "p", "q", "r", "s", "t"
    };

    for (int i = 0; i < 20; i++) {
        btreeInsert(tree, keys[i], i + 1, keys[i]);
    }

    assert(tree->size == 20);
    validateNode(tree->root, 1);

    for (int i = 0; i < 20; i++) {
        Vector* found = btreeSearch(tree, keys[i]);

        assert(found != NULL);
        assert(getPostingListSize(found) == 1);

        vectorFree(found);
    }

    freeBTree(tree);
}

static void test_search_missing_key(void) {
    BTree* tree = createBTree();

    btreeInsert(tree, "python", 1, "Python question");
    btreeInsert(tree, "java", 2, "Java question");

    Vector* found = btreeSearch(tree, "missing");

    assert(found == NULL);

    freeBTree(tree);
}

static void test_duplicate_key(void) {
    BTree* tree = createBTree();

    btreeInsert(tree, "python", 1, "First Python question");
    btreeInsert(tree, "python", 2, "Second Python question");

    assert(tree->size == 1);

    Vector* found = btreeSearch(tree, "python");

    assert(found != NULL);
    assert(getPostingListSize(found) == 2);

    PostingEntry* first = getPostingEntry(found, 0);
    PostingEntry* second = getPostingEntry(found, 1);

    assert(first != NULL);
    assert(second != NULL);

    assert(first->doc_id == 1);
    assert(second->doc_id == 2);

    assert(strcmp(first->title, "First Python question") == 0);
    assert(strcmp(second->title, "Second Python question") == 0);

    vectorFree(found);
    freeBTree(tree);
}

static void test_duplicate_same_document_is_ignored(void) {
    BTree* tree = createBTree();

    btreeInsert(tree, "python", 1, "Python question");
    btreeInsert(tree, "python", 1, "Python question duplicate");

    assert(tree->size == 1);

    Vector* found = btreeSearch(tree, "python");

    assert(found != NULL);
    assert(getPostingListSize(found) == 1);

    vectorFree(found);
    freeBTree(tree);
}

static void test_sorted_traverse(void) {
    BTree* tree = createBTree();

    btreeInsert(tree, "python", 1, "Python");
    btreeInsert(tree, "java", 2, "Java");
    btreeInsert(tree, "c", 3, "C");
    btreeInsert(tree, "rust", 4, "Rust");
    btreeInsert(tree, "go", 5, "Go");
    btreeInsert(tree, "bash", 6, "Bash");
    btreeInsert(tree, "kotlin", 7, "Kotlin");
    btreeInsert(tree, "ruby", 8, "Ruby");

    TraverseCtx ctx;
    ctx.count = 0;

    btreeTraverse(tree, collectKeys, &ctx);

    assert(ctx.count == tree->size);

    for (int i = 1; i < ctx.count; i++) {
        assert(strcmp(ctx.keys[i - 1], ctx.keys[i]) < 0);
    }

    freeBTree(tree);
}

static void test_search_returns_clone(void) {
    BTree* tree = createBTree();

    btreeInsert(tree, "python", 1, "Python question");

    Vector* found = btreeSearch(tree, "python");
    assert(found != NULL);

    appendPosting(found, 999, "Fake external mutation");

    Vector* found_again = btreeSearch(tree, "python");
    assert(found_again != NULL);

    assert(getPostingListSize(found) == 2);
    assert(getPostingListSize(found_again) == 1);

    vectorFree(found);
    vectorFree(found_again);
    freeBTree(tree);
}

int main(void) {
    printf("RUNNING B-TREE TESTS\n\n");

    printf("Test 1: create tree...\n");
    test_create_tree();

    printf("Test 2: insert one key...\n");
    test_insert_one_key();

    printf("Test 3: insert many keys...\n");
    test_insert_many_keys();

    printf("Test 4: root split...\n");
    test_root_split();

    printf("Test 5: internal split...\n");
    test_internal_split();

    printf("Test 6: missing key search...\n");
    test_search_missing_key();

    printf("Test 7: duplicate key...\n");
    test_duplicate_key();

    printf("Test 8: duplicate same document...\n");
    test_duplicate_same_document_is_ignored();

    printf("Test 9: sorted traversal...\n");
    test_sorted_traverse();

    printf("Test 10: search returns clone...\n");
    test_search_returns_clone();

    printf("\nALL B-TREE TESTS PASSED\n");

    return 0;
}