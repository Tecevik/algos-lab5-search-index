/*
 * Модуль: B-tree для поискового индекса.
 *
 * Ответственная: Ира.
 *
 * Назначение:
 * B-tree хранит пары:
 *
 *     term -> posting list
 *
 * где term — слово из документа,
 * а posting list — список документов, в которых это слово встречается.
 *
 * В отличие от AVL и Red-Black, один узел B-tree хранит
 * несколько ключей и несколько указателей на детей.
 *
 * Что реализовано:
 * 1. Создание пустого B-tree.
 * 2. Создание узлов.
 * 3. Поиск ключа внутри узла.
 * 4. Split полного ребёнка.
 * 5. Insert non-full.
 * 6. Добавление posting-записи, если ключ уже существует.
 * 7. Обход дерева в отсортированном порядке.
 * 8. Полное освобождение памяти.
 *
 * Важно:
 * Сигнатуры функций менять нельзя.
 * Эти функции вызываются из index/index.c.
 */

#include "btree.h"

#include <stdlib.h>
#include <string.h>

static char* strDuplicate(const char* src) {
    if (!src) return NULL;

    size_t len = strlen(src);
    char* copy = (char*)malloc(len + 1);
    if (!copy) return NULL;

    memcpy(copy, src, len + 1);
    return copy;
}

static BTreeNode* createNode(int is_leaf) {
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (!node) return NULL;

    node->n = 0;
    node->is_leaf = is_leaf;

    for (int i = 0; i < BTREE_MAX_KEYS; i++) {
        node->keys[i] = NULL;
        node->postings[i] = NULL;
    }

    for (int i = 0; i < BTREE_MAX_CH; i++) {
        node->children[i] = NULL;
    }

    return node;
}

static int findKeyIndex(const BTreeNode* node, const char* key) {
    int i = 0;

    while (i < node->n && strcmp(key, node->keys[i]) > 0) {
        i++;
    }

    return i;
}

static Vector* searchNode(const BTreeNode* node, const char* key) {
    if (!node || !key) return NULL;

    int i = findKeyIndex(node, key);

    if (i < node->n && strcmp(key, node->keys[i]) == 0) {
        return node->postings[i];
    }

    if (node->is_leaf) {
        return NULL;
    }

    return searchNode(node->children[i], key);
}

static int createKeyWithPosting(
    BTreeNode* node,
    int index,
    const char* key,
    int doc_id,
    const char* title
) {
    node->keys[index] = strDuplicate(key);
    if (!node->keys[index]) {
        return 0;
    }

    node->postings[index] = createPostingList();
    if (!node->postings[index]) {
        free(node->keys[index]);
        node->keys[index] = NULL;
        return 0;
    }

    appendPosting(node->postings[index], doc_id, title);

    return 1;
}

static int splitChild(BTreeNode* parent, int child_index) {
    if (!parent) return 0;

    BTreeNode* left = parent->children[child_index];
    if (!left) return 0;

    BTreeNode* right = createNode(left->is_leaf);
    if (!right) return 0;

    right->n = BTREE_T - 1;

    for (int j = 0; j < BTREE_T - 1; j++) {
        right->keys[j] = left->keys[j + BTREE_T];
        right->postings[j] = left->postings[j + BTREE_T];

        left->keys[j + BTREE_T] = NULL;
        left->postings[j + BTREE_T] = NULL;
    }

    if (!left->is_leaf) {
        for (int j = 0; j < BTREE_T; j++) {
            right->children[j] = left->children[j + BTREE_T];
            left->children[j + BTREE_T] = NULL;
        }
    }

    for (int j = parent->n; j >= child_index + 1; j--) {
        parent->children[j + 1] = parent->children[j];
    }

    parent->children[child_index + 1] = right;

    for (int j = parent->n - 1; j >= child_index; j--) {
        parent->keys[j + 1] = parent->keys[j];
        parent->postings[j + 1] = parent->postings[j];
    }

    parent->keys[child_index] = left->keys[BTREE_T - 1];
    parent->postings[child_index] = left->postings[BTREE_T - 1];

    left->keys[BTREE_T - 1] = NULL;
    left->postings[BTREE_T - 1] = NULL;

    left->n = BTREE_T - 1;
    parent->n++;

    return 1;
}

static int insertNonFull(
    BTreeNode* node,
    const char* key,
    int doc_id,
    const char* title
) {
    if (!node || !key || !title) return 0;

    int i = node->n - 1;

    if (node->is_leaf) {
        while (i >= 0 && strcmp(key, node->keys[i]) < 0) {
            node->keys[i + 1] = node->keys[i];
            node->postings[i + 1] = node->postings[i];
            i--;
        }

        if (!createKeyWithPosting(node, i + 1, key, doc_id, title)) {
            return 0;
        }

        node->n++;
        return 1;
    }

    while (i >= 0 && strcmp(key, node->keys[i]) < 0) {
        i--;
    }

    i++;

    if (node->children[i]->n == BTREE_MAX_KEYS) {
        if (!splitChild(node, i)) {
            return 0;
        }

        if (strcmp(key, node->keys[i]) > 0) {
            i++;
        }
    }

    return insertNonFull(node->children[i], key, doc_id, title);
}

static void freeNodeRecursive(BTreeNode* node) {
    if (!node) return;

    if (!node->is_leaf) {
        for (int i = 0; i <= node->n; i++) {
            freeNodeRecursive(node->children[i]);
        }
    }

    for (int i = 0; i < node->n; i++) {
        free(node->keys[i]);
        vectorFree(node->postings[i]);
    }

    free(node);
}

static void traverseRecursive(
    BTreeNode* node,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    if (!node || !visit) return;

    int i;

    for (i = 0; i < node->n; i++) {
        if (!node->is_leaf) {
            traverseRecursive(node->children[i], visit, ctx);
        }

        visit(node->keys[i], node->postings[i], ctx);
    }

    if (!node->is_leaf) {
        traverseRecursive(node->children[i], visit, ctx);
    }
}

BTree* createBTree(void) {
    BTree* tree = (BTree*)malloc(sizeof(BTree));
    if (!tree) return NULL;

    tree->root = NULL;
    tree->size = 0;

    return tree;
}

void freeBTree(BTree* tree) {
    if (!tree) return;

    freeNodeRecursive(tree->root);
    free(tree);
}

void btreeInsert(BTree* tree, const char* key, int doc_id, const char* title) {
    if (!tree || !key || !title) return;

    if (!tree->root) {
        tree->root = createNode(1);
        if (!tree->root) return;
    }

    Vector* existing = searchNode(tree->root, key);

    if (existing) {
        appendPosting(existing, doc_id, title);
        return;
    }

    BTreeNode* root = tree->root;

    if (root->n == BTREE_MAX_KEYS) {
        BTreeNode* new_root = createNode(0);
        if (!new_root) return;

        new_root->children[0] = root;
        tree->root = new_root;

        if (!splitChild(new_root, 0)) {
            tree->root = root;
            free(new_root);
            return;
        }

        if (!insertNonFull(new_root, key, doc_id, title)) {
            return;
        }
    } else {
        if (!insertNonFull(root, key, doc_id, title)) {
            return;
        }
    }

    tree->size++;
}

Vector* btreeSearch(const BTree* tree, const char* key) {
    if (!tree || !key) return NULL;

    Vector* postings = searchNode(tree->root, key);

    if (!postings) {
        return NULL;
    }

    return clonePostingList(postings);
}

void btreeTraverse(
    const BTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    if (!tree || !visit) return;

    traverseRecursive(tree->root, visit, ctx);
}