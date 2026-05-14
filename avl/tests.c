#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "avl.h"

static void printNode(const char* key, Vector* postings, void* ctx) {
    int* count = (int*)ctx;
    (*count)++;
    printf("    [%d] %s (%zu docs)\n", *count, key, postings->size);
}

static void test_create_tree(void) {
    printf("Тест1: Создание дерева\n");
    
    AVLTree* tree = createAVLTree();
    assert(tree != NULL);
    assert(tree->root == NULL);
    assert(tree->size == 0);
    
    freeAVLTree(tree);
    printf("Пройдено\n");
}

static void test_single_insert(void) {
    printf("Тест2: Одно вхождение\n");
    
    AVLTree* tree = createAVLTree();
    avlInsert(tree, "python", 1, "How to learn Python?");
    
    assert(tree->size == 1);
    assert(tree->root != NULL);
    assert(strcmp(tree->root->key, "python") == 0);
    assert(tree->root->height == 1);
    assert(tree->root->left == NULL);
    assert(tree->root->right == NULL);
    assert(tree->root->postings->size == 1);
    
    Vector* result = avlSearch(tree, "python");
    assert(result != NULL);
    assert(result->size == 1);
    vectorFree(result);
    
    freeAVLTree(tree);
    printf("Пройдено\n");
}

static void test_multiple_inserts(void) {
    printf("Тест3: Несколько вхождений\n");
    
    AVLTree* tree = createAVLTree();
    
    avlInsert(tree, "python", 1, "Python tutorial");
    avlInsert(tree, "java", 2, "Java programming");
    avlInsert(tree, "cpp", 3, "C++ basics");
    
    assert(tree->size == 3);
    
    Vector* pl1 = avlSearch(tree, "python");
    assert(pl1 != NULL);
    assert(pl1->size == 1);
    vectorFree(pl1);
    
    Vector* pl2 = avlSearch(tree, "java");
    assert(pl2 != NULL);
    assert(pl2->size == 1);
    vectorFree(pl2);
    
    Vector* pl3 = avlSearch(tree, "cpp");
    assert(pl3 != NULL);
    assert(pl3->size == 1);
    vectorFree(pl3);
    
    freeAVLTree(tree);
    printf("Пройдено\n");
}

static void test_search_nonexistent(void) {
    printf("Тест4: Поиск несуществующего ключа\n");
    
    AVLTree* tree = createAVLTree();
    avlInsert(tree, "python", 1, "Python tutorial");
    
    Vector* result = avlSearch(tree, "javascript");
    assert(result == NULL);
    
    freeAVLTree(tree);
    printf("Пройдено\n");
}

static void test_duplicate_key(void) {
    printf("Тест5: Дубликаты\n");
    
    AVLTree* tree = createAVLTree();
    
    avlInsert(tree, "python", 1, "Python tutorial");
    avlInsert(tree, "python", 2, "Advanced Python");
    avlInsert(tree, "python", 3, "Python tips");
    
    assert(tree->size == 1);
    assert(tree->root->postings->size == 3);
    
    Vector* result = avlSearch(tree, "python");
    assert(result->size == 3);
    vectorFree(result);
    
    freeAVLTree(tree);
    printf("Пройдено\n");
}

static void test_traversal(void) {
    printf("Тест6: Перемещение по порядку\n");
    
    AVLTree* tree = createAVLTree();
    
    avlInsert(tree, "banana", 1, "Banana");
    avlInsert(tree, "apple", 2, "Apple");
    avlInsert(tree, "cherry", 3, "Cherry");
    
    int counter = 0;
    avlTraverse(tree, printNode, &counter);
    assert(counter == 3);
    
    freeAVLTree(tree);
    printf("Пройдено\n");
}

static void test_balance_right_rotation(void) {
    printf("Тест7: Балансировка правого поворота\n");
    
    AVLTree* tree = createAVLTree();

    avlInsert(tree, "zebra", 1, "Zebra");
    avlInsert(tree, "monkey", 2, "Monkey");
    avlInsert(tree, "ant", 3, "Ant");
    
    assert(avlIsBalanced(tree->root));
    assert(tree->root != NULL);
    assert(strcmp(tree->root->key, "monkey") == 0);
    
    freeAVLTree(tree);
    printf("Пройдено\n");
}

static void test_balance_left_rotation(void) {
    printf("Тест8: Балансировка левого поворота\n");
    
    AVLTree* tree = createAVLTree();

    avlInsert(tree, "ant", 1, "Ant");
    avlInsert(tree, "monkey", 2, "Monkey");
    avlInsert(tree, "zebra", 3, "Zebra");
    
    assert(avlIsBalanced(tree->root));
    assert(tree->root != NULL);
    assert(strcmp(tree->root->key, "monkey") == 0);
    
    freeAVLTree(tree);
    printf("Пройдено\n");
}

static void test_balance_left_right(void) {
    printf("Тест9: Балансировка левого-правого поворота\n");
    
    AVLTree* tree = createAVLTree();
    
    avlInsert(tree, "zebra", 1, "Zebra");
    avlInsert(tree, "ant", 2, "Ant");
    avlInsert(tree, "monkey", 3, "Monkey");
    
    assert(avlIsBalanced(tree->root));
    assert(tree->root != NULL);
    assert(strcmp(tree->root->key, "monkey") == 0);
    
    freeAVLTree(tree);
    printf("Пройдено\n");
}

static void test_balance_right_left(void) {
    printf("Тест10: Балансировка правого-левого поворота\n");
    
    AVLTree* tree = createAVLTree();
    
    avlInsert(tree, "ant", 1, "Ant");
    avlInsert(tree, "zebra", 2, "Zebra");
    avlInsert(tree, "monkey", 3, "Monkey");
    
    assert(avlIsBalanced(tree->root));
    assert(tree->root != NULL);
    assert(strcmp(tree->root->key, "monkey") == 0);
    
    freeAVLTree(tree);
    printf("Пройдено\n");
}

static void test_large_insertions(void) {
    printf("Тест11: Огромное количество вхождений\n");
    
    AVLTree* tree = createAVLTree();
    
    /* Вставка 500 элементов */
    for (int i = 0; i < 500; i++) {
        char key[20];
        char title[50];
        sprintf(key, "word_%d", i);
        sprintf(title, "Title %d", i);
        avlInsert(tree, key, i, title);
    }
    
    assert(tree->size == 500);
    assert(avlIsBalanced(tree->root));
    
    int height = avlGetHeight(tree->root);
    printf("      (height=%d, log2~9)\n", height);
    assert(height < 20); /* AVL height < 1.44 * log2(n) ≈ 13 для 500 */
    
    freeAVLTree(tree);
    printf("Пройдено\n");
}

/* ========== Основная функция ========== */

int main(void) {
    printf("\nЗАПУСК ТЕСТОВ\n\n");
    
    test_create_tree();
    test_single_insert();
    test_multiple_inserts();
    test_search_nonexistent();
    test_duplicate_key();
    test_traversal();
    test_balance_right_rotation();
    test_balance_left_rotation();
    test_balance_left_right();
    test_balance_right_left();
    test_large_insertions();
    
    printf("\nВСЕ ТЕСТЫ ПРОЙДЕНЫ\n");
    return 0;
}