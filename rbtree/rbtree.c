/*
* Модуль: Red-Black Tree для поискового индекса.
 *
 * Ответственный: Ваня.
 *
 * Назначение:
 * Красно-чёрное дерево хранит пары:
 *
 *     term -> posting list
 *
 * Используется как один из трёх вариантов backend-структуры
 * для инвертированного индекса.
 *
 * Сейчас файл содержит заглушки.
 * Они нужны, чтобы проект собирался до полной реализации дерева.
 *
 * Что нужно реализовать:
 * 1. Создание дерева и sentinel-узла nil.
 * 2. Вставку нового ключа.
 * 3. Добавление posting-записи, если ключ уже существует.
 * 4. Балансировку после вставки.
 * 5. Поиск по ключу.
 * 6. In-order обход.
 * 7. Полное освобождение памяти, включая nil.
 *
 * Важно:
 * Сигнатуры функций менять нельзя.
 * Эти функции вызываются из index/index.c.
 */

#include "rbtree.h"

#include <stdlib.h>
#include <string.h>

static RBNode* createNode(
    RBTree* tree,
    const char* key,
    int doc_id,
    const char* title
) {
    RBNode* node = (RBNode*)malloc(sizeof(RBNode));
    if (!node) return NULL;

    node->key = strdup(key);
    if (!node->key) {
        free(node);
        return NULL;
    }

    node->postings = createPostingList();
    if (!node->postings) {
        free(node->key);
        free(node);
        return NULL;
    }

    appendPosting(node->postings, doc_id, title);

    node->color = RB_RED;

    node->left = tree->nil;
    node->right = tree->nil;
    node->parent = tree->nil;

    return node;
}

static void leftRotate(RBTree* tree, RBNode* x) {
    RBNode* y = x->right;

    x->right = y->left;

    if (y->left != tree->nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;

    if (x->parent == tree->nil) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

static void rightRotate(RBTree* tree, RBNode* y) {
    RBNode* x = y->left;

    y->left = x->right;

    if (x->right != tree->nil) {
        x->right->parent = y;
    }

    x->parent = y->parent;

    if (y->parent == tree->nil) {
        tree->root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }

    x->right = y;
    y->parent = x;
}

static void insertFixup(RBTree* tree, RBNode* z) {
    while (z->parent->color == RB_RED) {

        if (z->parent == z->parent->parent->left) {

            RBNode* y = z->parent->parent->right;

            if (y->color == RB_RED) {

                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;

                z = z->parent->parent;

            } else {

                if (z == z->parent->right) {
                    z = z->parent;
                    leftRotate(tree, z);
                }

                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;

                rightRotate(tree, z->parent->parent);
            }

        } else {

            RBNode* y = z->parent->parent->left;

            if (y->color == RB_RED) {

                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;

                z = z->parent->parent;

            } else {

                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(tree, z);
                }

                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;

                leftRotate(tree, z->parent->parent);
            }
        }
    }

    tree->root->color = RB_BLACK;
}

RBTree* createRBTree(void) {
    RBTree* tree = (RBTree*)malloc(sizeof(RBTree));
    if (!tree) return NULL;

    tree->nil = (RBNode*)malloc(sizeof(RBNode));
    if (!tree->nil) {
        free(tree);
        return NULL;
    }

    tree->nil->color = RB_BLACK;
    tree->nil->left = tree->nil;
    tree->nil->right = tree->nil;
    tree->nil->parent = tree->nil;

    tree->nil->key = NULL;
    tree->nil->postings = NULL;

    tree->root = tree->nil;
    tree->size = 0;

    return tree;
}

static void freeNodeRecursive(RBTree* tree, RBNode* node) {
    if (node == tree->nil) return;

    freeNodeRecursive(tree, node->left);
    freeNodeRecursive(tree, node->right);

    free(node->key);
    vectorFree(node->postings);

    free(node);
}

void freeRBTree(RBTree* tree) {
    if (!tree) return;

    freeNodeRecursive(tree, tree->root);

    free(tree->nil);
    free(tree);
}

void rbInsert(RBTree* tree, const char* key, int doc_id, const char* title) {
    if (!tree || !key || !title) return;

    RBNode* y = tree->nil;
    RBNode* x = tree->root;

    while (x != tree->nil) {

        y = x;

        int cmp = strcmp(key, x->key);

        if (cmp == 0) {
            appendPosting(x->postings, doc_id, title);
            return;
        }

        if (cmp < 0) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    RBNode* z = createNode(tree, key, doc_id, title);
    if (!z) return;

    z->parent = y;

    if (y == tree->nil) {
        tree->root = z;
    } else if (strcmp(z->key, y->key) < 0) {
        y->left = z;
    } else {
        y->right = z;
    }

    insertFixup(tree, z);

    tree->size++;
}

Vector* rbSearch(const RBTree* tree, const char* key) {
    if (!tree || !key) return NULL;

    RBNode* current = tree->root;

    while (current != tree->nil) {

        int cmp = strcmp(key, current->key);

        if (cmp == 0) {
            return current->postings;
        }

        if (cmp < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    return NULL;
}

static void traverseRecursive(
    const RBTree* tree,
    RBNode* node,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    if (node == tree->nil) return;

    traverseRecursive(tree, node->left, visit, ctx);

    visit(node->key, node->postings, ctx);

    traverseRecursive(tree, node->right, visit, ctx);
}

void rbTraverse(
    const RBTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    if (!tree || !visit) return;

    traverseRecursive(tree, tree->root, visit, ctx);
}