/*
* Модуль: Index.
 *
 * Ответственный: Егор.
 *
 * Назначение:
 * Index — это единая обёртка над тремя структурами:
 *
 * - AVL
 * - Red-Black Tree
 * - B-tree
 *
 * Остальной код не должен напрямую работать с avl/rbtree/btree.
 * Вместо этого используется общий интерфейс:
 *
 * - createIndex
 * - insertTerm
 * - lookupTerm
 * - traverseIndex
 * - saveIndex
 * - loadIndex
 * - freeIndex
 *
 * Благодаря этому search.c и main.c не зависят от конкретного дерева.
 *
 * Сейчас часть функций может быть временной заглушкой.
 * Особенно это касается saveIndex/loadIndex/runIndex.
 *
 * Что нужно реализовать:
 * 1. Создание нужного дерева по TreeType.
 * 2. Вставку термов через нужный backend.
 * 3. Поиск термов через нужный backend.
 * 4. Обход индекса.
 * 5. Чтение docs.jsonl.
 * 6. Построение индекса.
 * 7. Сохранение индекса в файл.
 * 8. Загрузку индекса из файла.
 *
 * Важно:
 * Этот слой должен скрывать различия между деревьями.
 */

#include "index.h"

#include "../avl/avl.h"
#include "../rbtree/rbtree.h"
#include "../btree/btree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TreeType parseType(const char* s) {
    if (!s) return TREE_AVL;

    if (strcmp(s, "avl") == 0) {
        return TREE_AVL;
    }

    if (strcmp(s, "rb") == 0 || strcmp(s, "rbtree") == 0 || strcmp(s, "redblack") == 0) {
        return TREE_RB;
    }

    if (strcmp(s, "btree") == 0 || strcmp(s, "b-tree") == 0) {
        return TREE_BTREE;
    }

    fprintf(stderr, "Unknown tree type: %s. Fallback to AVL.\n", s);
    return TREE_AVL;
}

const char* typeName(TreeType type) {
    switch (type) {
        case TREE_AVL:
            return "avl";
        case TREE_RB:
            return "rb";
        case TREE_BTREE:
            return "btree";
        default:
            return "unknown";
    }
}

Index* createIndex(TreeType type) {
    Index* idx = malloc(sizeof(Index));
    if (!idx) return NULL;

    idx->type = type;
    idx->tree = NULL;

    switch (type) {
        case TREE_AVL:
            idx->tree = createAVLTree();
            break;
        case TREE_RB:
            idx->tree = createRBTree();
            break;
        case TREE_BTREE:
            idx->tree = createBTree();
            break;
        default:
            free(idx);
            return NULL;
    }

    if (!idx->tree) {
        free(idx);
        return NULL;
    }

    return idx;
}

void freeIndex(Index* idx) {
    if (!idx) return;

    switch (idx->type) {
        case TREE_AVL:
            freeAVLTree((AVLTree*)idx->tree);
            break;
        case TREE_RB:
            freeRBTree((RBTree*)idx->tree);
            break;
        case TREE_BTREE:
            freeBTree((BTree*)idx->tree);
            break;
        default:
            break;
    }

    free(idx);
}

void insertTerm(Index* idx, const char* term, int doc_id, const char* title) {
    if (!idx || !term) return;

    switch (idx->type) {
        case TREE_AVL:
            avlInsert((AVLTree*)idx->tree, term, doc_id, title);
            break;
        case TREE_RB:
            rbInsert((RBTree*)idx->tree, term, doc_id, title);
            break;
        case TREE_BTREE:
            btreeInsert((BTree*)idx->tree, term, doc_id, title);
            break;
        default:
            break;
    }
}

Vector* lookupTerm(const Index* idx, const char* term) {
    if (!idx || !term) return NULL;

    switch (idx->type) {
        case TREE_AVL:
            return avlSearch((const AVLTree*)idx->tree, term);
        case TREE_RB:
            return rbSearch((const RBTree*)idx->tree, term);
        case TREE_BTREE:
            return btreeSearch((const BTree*)idx->tree, term);
        default:
            return NULL;
    }
}

void indexDocument(
    Index* idx,
    int doc_id,
    const char* title,
    const char** tokens,
    int n_tokens
) {
    if (!idx || !tokens || n_tokens <= 0) return;

    for (int i = 0; i < n_tokens; i++) {
        if (tokens[i] && tokens[i][0] != '\0') {
            insertTerm(idx, tokens[i], doc_id, title);
        }
    }
}

void traverseIndex(
    const Index* idx,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    if (!idx || !visit) return;

    switch (idx->type) {
        case TREE_AVL:
            avlTraverse((const AVLTree*)idx->tree, visit, ctx);
            break;
        case TREE_RB:
            rbTraverse((const RBTree*)idx->tree, visit, ctx);
            break;
        case TREE_BTREE:
            btreeTraverse((const BTree*)idx->tree, visit, ctx);
            break;
        default:
            break;
    }
}

void saveIndex(const Index* idx, const char* path) {
    /*
     * TODO(Егор):
     * Реализовать сохранение индекса в файл.
     *
     * Предлагаемый формат:
     *
     * term<TAB>doc_id:title|doc_id:title|doc_id:title
     *
     * Пример:
     *
     * python    1:How to sort a list|5:Python tips
     *
     * Для сохранения нужно использовать traverseIndex().
     * Он обойдёт дерево и для каждого term вызовет callback.
     *
     * Важно:
     * Формат должен быть одинаковым для AVL, RB и B-tree.
     */
    if (!idx || !path) return;

    FILE* f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "Failed to open index for writing: %s\n", path);
        return;
    }

    fprintf(f, "# index type: %s\n", typeName(idx->type));
    fprintf(f, "# TODO: implement real serialization\n");

    fclose(f);
}

Index* loadIndex(const char* path, TreeType type) {
    /*
     * TODO(Егор):
     * Реализовать загрузку индекса из файла.
     *
     * Логика:
     * - создать пустой Index нужного типа;
     * - открыть файл;
     * - прочитать term и posting list;
     * - для каждой posting-записи вызвать insertTerm();
     *
     * Важно:
     * Загруженный индекс должен работать одинаково для всех трёх деревьев.
     */
    (void)path;
    return createIndex(type);
}

void runIndex(TreeType type, const char* data_path, const char* idx_path) {
    /*
     * TODO(Егор):
     * Реализовать полный pipeline индексации.
     *
     * Логика:
     * 1. Создать Index нужного типа.
     * 2. Открыть docs.jsonl.
     * 3. Для каждой строки получить:
     *    - doc_id
     *    - title
     *    - tokens
     * 4. Вызвать indexDocument().
     * 5. Сохранить индекс через saveIndex().
     * 6. Освободить память.
     *
     * Примечание:
     * JSON можно парсить упрощённо, потому что формат docs.jsonl
     * генерирует наш preprocess.py.
     */
    printf("Indexing started\n");
    printf("type=%s\n", typeName(type));
    printf("data=%s\n", data_path ? data_path : "(null)");
    printf("index=%s\n", idx_path ? idx_path : "(null)");

    Index* idx = createIndex(type);
    if (!idx) {
        fprintf(stderr, "Failed to create index\n");
        return;
    }

    saveIndex(idx, idx_path);
    freeIndex(idx);

    printf("Indexing finished\n");
}