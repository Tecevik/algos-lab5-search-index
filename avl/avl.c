/*
* Модуль: AVL-дерево для поискового индекса.
 *
 * Ответственный: Коля.
 *
 * Назначение:
 * AVL хранит пары:
 *
 *     term -> posting list
 *
 * где term — слово из документа,
 * а posting list — список документов, в которых это слово встречается.
 *
 * Сейчас файл содержит заглушки.
 * Заглушки нужны только для того, чтобы весь проект собирался
 * до полной реализации AVL.
 *
 * Что нужно реализовать:
 * 1. Создание пустого AVL-дерева.
 * 2. Вставку нового ключа.
 * 3. Добавление posting-записи, если ключ уже существует.
 * 4. Балансировку AVL после вставки.
 * 5. Поиск по ключу.
 * 6. In-order обход дерева.
 * 7. Полное освобождение памяти.
 *
 * Важно:
 * Сигнатуры функций менять нельзя.
 * Эти функции вызываются из index/index.c.
 */

#include "avl.h"
#include <stdlib.h>
#include <string.h>

static int max(int a, int b) {
    return (a > b) ? a : b;
}

static int getHeight(AVLNode* node) {
    return node ? node->height : 0;
}

static void updateHeight(AVLNode* node) {
    if (node) {
        node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    }
}

static int getBalance(AVLNode* node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

static AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    updateHeight(y);
    updateHeight(x);
    
    return x;
}

static AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    updateHeight(x);
    updateHeight(y);
    
    return y;
}

static AVLNode* balanceNode(AVLNode* node) {
    if (!node) return NULL;  
    int balance = getBalance(node);

    if (balance > 1) {
        if (getBalance(node->left) < 0) {
            node->left = rotateLeft(node->left);
        }
        return rotateRight(node);
    }

    if (balance < -1) {
        if (getBalance(node->right) > 0) {
            node->right = rotateRight(node->right);
        }
        return rotateLeft(node);
    }
    return node;
}

static AVLNode* createNode(const char* key, int doc_id, const char* title) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
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

    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    
    return node;
}

static AVLNode* insertRecursive(AVLNode* node, const char* key, 
                                 int doc_id, const char* title, int* is_new) {
    if (!node) {
        *is_new = 1;
        return createNode(key, doc_id, title);
    }
    
    int cmp = strcmp(key, node->key);
    
    if (cmp < 0) {
        node->left = insertRecursive(node->left, key, doc_id, title, is_new);
    } else if (cmp > 0) {
        node->right = insertRecursive(node->right, key, doc_id, title, is_new);
    } else {
        *is_new = 0;
        appendPosting(node->postings, doc_id, title);
        return node;
    }

    updateHeight(node);

    return balanceNode(node);
}

static void freeNodeRecursive(AVLNode* node) {
    if (!node) return;
    freeNodeRecursive(node->left);
    freeNodeRecursive(node->right);
    free(node->key);
    vectorFree(node->postings);
    free(node);
}

AVLTree* createAVLTree(void) {
    AVLTree* tree = (AVLTree*)malloc(sizeof(AVLTree));
    if (!tree) return NULL;
    tree->root = NULL;
    tree->size = 0;
    
    return tree;
}

void freeAVLTree(AVLTree* tree) {
    if (!tree) return;
    freeNodeRecursive(tree->root);
    free(tree);
}

void avlInsert(AVLTree* tree, const char* key, int doc_id, const char* title) {
    if (!tree || !key || !title) return;
    int is_new = 0;
    tree->root = insertRecursive(tree->root, key, doc_id, title, &is_new);
    
    if (is_new) {
        tree->size++;
    }
}

Vector* avlSearch(const AVLTree* tree, const char* key) {
    if (!tree || !key) return NULL;
    
    AVLNode* current = tree->root;
    
    while (current) {
        int cmp = strcmp(key, current->key);
        if (cmp == 0) {
            return clonePostingList(current->postings);
        } else if (cmp < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    
    return NULL;
}

static void traverseRecursive(AVLNode* node, 
                               void (*visit)(const char* key, Vector* postings, void* ctx),
                               void* ctx) {
    if (!node) return;
    traverseRecursive(node->left, visit, ctx);
    visit(node->key, node->postings, ctx);
    traverseRecursive(node->right, visit, ctx);
}

void avlTraverse(const AVLTree* tree,
                 void (*visit)(const char* key, Vector* postings, void* ctx),
                 void* ctx) {
    if (!tree || !visit) return;
    traverseRecursive(tree->root, visit, ctx);
}

int avlGetHeight(const AVLNode* node) {
    return getHeight((AVLNode*)node);
}

static int isBalancedRecursive(const AVLNode* node) {
    if (!node) return 1;
    int balance = getBalance((AVLNode*)node);
    if (balance < -1 || balance > 1) return 0;
    
    return isBalancedRecursive(node->left) && isBalancedRecursive(node->right);
}

int avlIsBalanced(const AVLNode* node) {
    return isBalancedRecursive(node);
}