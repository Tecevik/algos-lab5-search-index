# Red-Black Tree

Ответственный: Ваня.

Эта папка содержит реализацию красно-чёрного дерева для поискового индекса.

Red-Black Tree хранит пары:

```text
term -> posting list
```

---

## Основные файлы

```text
rbtree/
├── README.md
├── rbtree.h
├── rbtree.c
└── tests.c
```

---

## Что нужно реализовать

В файле `rbtree.c` нужно реализовать:

- создание дерева;
- создание sentinel-узла `nil`;
- вставку ключа;
- добавление документа в posting list, если ключ уже существует;
- балансировку после вставки;
- поиск по ключу;
- in-order обход;
- полное освобождение памяти.

---

## Публичный интерфейс

Сигнатуры функций в `rbtree.h` менять нельзя без согласования.

```c
RBTree* createRBTree(void);
void    freeRBTree(RBTree* tree);

void    rbInsert(RBTree* tree, const char* key, int doc_id, const char* title);
Vector* rbSearch(const RBTree* tree, const char* key);

void rbTraverse(
    const RBTree* tree,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
);
```

---

## Основные инварианты Red-Black Tree

Нужно поддерживать свойства:

1. Каждый узел либо красный, либо чёрный.
2. Корень всегда чёрный.
3. Все `nil`-листья чёрные.
4. У красного узла не может быть красного ребёнка.
5. Все пути от узла до `nil`-листьев содержат одинаковое количество чёрных узлов.

---

## Правило для повторяющихся ключей

Если ключ уже есть в дереве, новый узел создавать нельзя.  
Нужно добавить документ в существующий posting list.

---

## Тесты

В `tests.c` нужно проверить:

- создание дерева;
- вставку ключей;
- поиск;
- повторную вставку ключа;
- что корень чёрный;
- что нет двух красных узлов подряд;
- корректность обхода;
- освобождение памяти.

---

## Проверка

```
gcc -Wall -Wextra -std=c11 -O2 -g -o test_rb.exe common/vector.c posting.c rbtree/rbtree.c rbtree/tests.c
.\test_rb.exe
```