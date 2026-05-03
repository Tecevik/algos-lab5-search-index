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
 * В отличие от AVL и Red-Black, один узел B-tree хранит
 * несколько ключей и несколько указателей на детей.
 *
 * Сейчас файл содержит заглушки.
 * Они нужны, чтобы проект собирался до полной реализации B-tree.
 *
 * Что нужно реализовать:
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

BTree* createBTree(void) {
    BTree* tree = malloc(sizeof(BTree));
    if (!tree) return NULL;

    tree->root = NULL;
    tree->size = 0;

    return tree;
}

void freeBTree(BTree* tree) {
    /*
     * TODO: free all nodes recursively.
     */
    free(tree);
}

void btreeInsert(BTree* tree, const char* key, int doc_id, const char* title) {
    /*
     * TODO(Ира):
     * Реализовать вставку в B-tree.
     *
     * Логика:
     * - если дерево пустое, создать root;
     * - если root полный, создать новый root и выполнить split;
     * - вставлять ключ через insertNonFull;
     * - если key уже существует, не дублировать ключ,
     *   а добавить doc_id/title в соответствующий posting list.
     *
     * tree->size увеличивается только при добавлении нового уникального key.
     */
    (void)tree;
    (void)key;
    (void)doc_id;
    (void)title;
}

Vector* btreeSearch(const BTree* tree, const char* key) {
    /*
     * TODO: implement B-tree search.
     */
    (void)tree;
    (void)key;
    return NULL;
}

void btreeTraverse(
    const BTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    /*
     * TODO: implement sorted traversal.
     */
    (void)tree;
    (void)visit;
    (void)ctx;
}