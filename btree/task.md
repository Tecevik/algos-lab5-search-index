# B-tree

Ответственная: Ира.

Эта папка содержит реализацию B-tree для поискового индекса.

B-tree хранит пары:

```text
term -> posting list
```

В отличие от AVL и Red-Black Tree, один узел B-tree может хранить несколько ключей.

---

## Основные файлы

```text
btree/
├── README.md
├── btree.h
├── btree.c
└── tests.c
```

---

## Что нужно реализовать

В файле `btree.c` нужно реализовать:

- создание дерева;
- создание узлов;
- поиск ключа внутри узла;
- split полного ребёнка;
- вставку в неполный узел;
- вставку в дерево;
- добавление документа в posting list, если ключ уже существует;
- обход дерева в отсортированном порядке;
- полное освобождение памяти.

---

## Публичный интерфейс

Сигнатуры функций в `btree.h` менять нельзя без согласования.

```c
BTree* createBTree(void);
void   freeBTree(BTree* tree);

void    btreeInsert(BTree* tree, const char* key, int doc_id, const char* title);
Vector* btreeSearch(const BTree* tree, const char* key);

void btreeTraverse(
    const BTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
);
```

---

## Параметры B-tree

В `btree.h` задана минимальная степень:

```c
#define BTREE_T 3
```

Это значит:

- максимум ключей в узле: `2 * BTREE_T - 1`;
- максимум детей в узле: `2 * BTREE_T`.

---

## Правило для повторяющихся ключей

Если ключ уже есть в B-tree, нельзя добавлять его повторно.  
Нужно добавить `doc_id/title` в уже существующий posting list.

---

## Тесты

В `tests.c` нужно проверить:

- создание дерева;
- вставку одного ключа;
- вставку нескольких ключей;
- split корня;
- split внутреннего узла;
- поиск существующего ключа;
- поиск несуществующего ключа;
- повторную вставку ключа;
- корректный отсортированный обход;
- освобождение памяти.

---

## Проверка

```
gcc -Wall -Wextra -std=c11 -O2 -g -o test_btree.exe common/vector.c posting.c btree/btree.c btree/tests.c
.\test_btree.exe
```