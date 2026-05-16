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

RBTree* createRBTree(void) {
    RBTree* tree = malloc(sizeof(RBTree));
    if (!tree) return NULL;

    tree->nil = NULL;
    tree->root = NULL;
    tree->size = 0;

    return tree;
}

void freeRBTree(RBTree* tree) {
    /*
     * TODO: free all nodes and sentinel.
     */
    free(tree);
}

void rbInsert(RBTree* tree, const char* key, int doc_id, const char* title) {
    /*
     * TODO(Ваня):
     * Реализовать вставку в Red-Black Tree.
     *
     * Требования:
     * - root всегда должен быть чёрным;
     * - nil/sentinel должен использоваться как чёрный лист;
     * - не должно быть двух красных узлов подряд;
     * - black-height должен сохраняться.
     *
     * Если key уже есть:
     * - не создаём новый узел;
     * - добавляем doc_id/title в postings существующего узла.
     */
    (void)tree;
    (void)key;
    (void)doc_id;
    (void)title;
}

Vector* rbSearch(const RBTree* tree, const char* key) {
    /*
     * TODO: implement Red-Black search.
     */
    (void)tree;
    (void)key;
    return NULL;
}

void rbTraverse(
    const RBTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    /*
     * TODO: implement in-order traversal.
     */
    (void)tree;
    (void)visit;
    (void)ctx;
}