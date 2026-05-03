# AVL

Ответственный: Коля.

Эта папка содержит реализацию AVL-дерева для поискового индекса.

AVL хранит пары:

```text
term -> posting list
```

Где:

- `term` — слово из документа;
- `posting list` — список документов, где это слово встречается.

---

## Основные файлы

```text
avl/
├── README.md
├── avl.h
├── avl.c
└── tests.c
```

---

## Что нужно реализовать

В файле `avl.c` нужно реализовать:

- создание пустого дерева;
- вставку ключа;
- добавление документа в posting list, если ключ уже существует;
- поиск по ключу;
- AVL-балансировку после вставки;
- in-order обход;
- полное освобождение памяти.

---

## Публичный интерфейс

Сигнатуры функций в `avl.h` менять нельзя без согласования.

```c
AVLTree* createAVLTree(void);
void     freeAVLTree(AVLTree* tree);

void    avlInsert(AVLTree* tree, const char* key, int doc_id, const char* title);
Vector* avlSearch(const AVLTree* tree, const char* key);

void avlTraverse(
    const AVLTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
);
```

---

## Правило для повторяющихся ключей

Если ключа ещё нет в дереве, нужно создать новый узел.

Если ключ уже есть, новый узел создавать нельзя.  
Нужно только добавить `doc_id/title` в существующий posting list.

---

## Тесты

В `tests.c` нужно проверить:

- создание дерева;
- вставку одного ключа;
- вставку нескольких ключей;
- поиск существующего ключа;
- поиск несуществующего ключа;
- повторную вставку того же ключа;
- корректность обхода;
- базовые случаи балансировки.

---

## Проверка

```
gcc -Wall -Wextra -std=c11 -O2 -g -o test_avl.exe common/vector.c posting.c avl/avl.c avl/tests.c
.\test_avl.exe
```
```