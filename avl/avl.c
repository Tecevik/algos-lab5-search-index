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

AVLTree* createAVLTree(void) {
    AVLTree* tree = malloc(sizeof(AVLTree));
    if (!tree) return NULL;

    tree->root = NULL;
    tree->size = 0;

    return tree;
}

void freeAVLTree(AVLTree* tree) {
    /*
     * TODO: free all nodes recursively.
     */
    free(tree);
}

void avlInsert(AVLTree* tree, const char* key, int doc_id, const char* title) {
    /*
     * TODO(Коля):
     * Реализовать вставку в AVL.
     *
     * Логика:
     * - если key ещё нет в дереве, создать новый AVLNode;
     * - скопировать key через strdup или собственный аналог;
     * - создать posting list;
     * - добавить туда doc_id/title;
     * - обновить высоты;
     * - выполнить нужные повороты;
     * - увеличить tree->size только при появлении нового ключа.
     *
     * Если key уже есть:
     * - дерево не меняет структуру;
     * - просто добавляем doc_id/title в postings.
     */
    (void)tree;
    (void)key;
    (void)doc_id;
    (void)title;
}

Vector* avlSearch(const AVLTree* tree, const char* key) {
    /*
     * TODO: implement AVL search.
     */
    (void)tree;
    (void)key;
    return NULL;
}

void avlTraverse(
    const AVLTree* tree,
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