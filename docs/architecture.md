# Архитектура проекта

Проект реализует учебную поисковую систему на основе инвертированного индекса.

Главная идея инвертированного индекса:

```text
слово -> список документов, где это слово встречается
```

Пример:

```text
python -> [(1, "How to sort a list in Python"), (5, "Python tips")]
list   -> [(1, "How to sort a list in Python"), (7, "C linked list")]
tree   -> [(3, "Balanced binary tree rotations")]
```

В проекте один и тот же поисковый индекс можно хранить в трёх разных структурах данных:

- AVL-дерево;
- Red-Black Tree;
- B-tree.

Это нужно, чтобы сравнить их по скорости индексации, скорости поиска и потреблению памяти.

---

## 1. Общая схема слоёв

Проект делится на несколько слоёв.

```text
app.py / benchmark.py
        ↓
main.c
        ↓
index/search.c
        ↓
index/index.c
        ↓
AVL / Red-Black / B-tree
        ↓
posting list / vector
```

Каждый слой отвечает только за свою часть.

---

## 2. Frontend-слой

Основной файл:

```text
app.py
```

`app.py` отвечает за веб-интерфейс на Streamlit.

Важно: frontend не реализует поиск сам. Он только вызывает скомпилированную C-программу `./app`.

Пример вызова:

```bash
./app search --type=avl --json "python list"
```

После этого `app.py` читает JSON из `stdout` и отображает результаты в браузере.

---

## 3. Benchmark-слой

Предполагаемые файлы:

```text
scripts/benchmark.py
benchmarks/results/
benchmarks/plots/
```

Benchmark-скрипты нужны для сравнения трёх структур:

- время построения индекса;
- время поиска;
- потребление памяти.

Benchmark-слой тоже не должен напрямую вызывать функции из C-кода. Он должен работать через CLI-команды.

Пример:

```bash
./app index --type=avl --data=data/processed/docs_50000.jsonl --index=data/index_avl_50000.txt
./app search --type=avl --index=data/index_avl_50000.txt --json "python list"
```

---

## 4. CLI-слой

Основной файл:

```text
main.c
```

`main.c` — это точка входа в C-приложение.

Он отвечает за:

- разбор аргументов командной строки;
- выбор режима работы;
- выбор типа дерева;
- вызов нужных функций из `index.c` и `search.c`.

`main.c` не должен реализовывать:

- AVL;
- Red-Black Tree;
- B-tree;
- поиск;
- сериализацию;
- парсинг датасета.

Он только связывает CLI с остальными слоями.

---

## 5. Поддерживаемые CLI-команды

### Индексация

```bash
./app index --type=<avl|rb|btree> --data=PATH --index=PATH
```

Пример:

```bash
./app index --type=avl --data=data/processed/docs.jsonl --index=data/index_avl.txt
```

Что должна делать команда:

1. Создать индекс выбранного типа.
2. Прочитать `docs.jsonl`.
3. Добавить документы в индекс.
4. Сохранить индекс в файл.

---

### Поиск

```bash
./app search --type=<avl|rb|btree> --index=PATH [--json] "query"
```

Пример:

```bash
./app search --type=rb --index=data/index_rb.txt --json "python list"
```

Что должна делать команда:

1. Загрузить индекс выбранного типа.
2. Разбить запрос на токены.
3. Найти документы.
4. Вывести результат в текстовом формате или JSON.

---

## 6. Search-слой

Основные файлы:

```text
index/search.h
index/search.c
```

Search-слой отвечает за логику поиска.

Он должен делать:

1. Токенизацию пользовательского запроса.
2. Получение posting list для каждого токена.
3. Пересечение posting list'ов.
4. Формирование top-10 результатов.
5. Подсчёт времени поиска.
6. Вывод результата в текстовом формате.
7. Вывод результата в JSON-формате.

---

## 7. Семантика поиска

Базовая семантика поиска — AND.

То есть запрос:

```text
python list
```

означает:

```text
найти документы, где есть и "python", и "list"
```

Пример:

```text
python -> [1, 2, 5, 8]
list   -> [2, 5, 9]

результат -> [2, 5]
```

---

## 8. Важное ограничение Search-слоя

`search.c` не должен напрямую обращаться к конкретным деревьям.

Нельзя:

```c
avlSearch(...)
rbSearch(...)
btreeSearch(...)
```

Нужно:

```c
lookupTerm(idx, term)
```

Так `search.c` остаётся независимым от того, какая структура используется внутри.

---

## 9. Index-слой

Основные файлы:

```text
index/index.h
index/index.c
```

Index-слой — это единая обёртка над тремя структурами данных:

- AVL;
- Red-Black Tree;
- B-tree.

Он скрывает различия между деревьями от остального проекта.

---

## 10. Основной контракт Index-слоя

Остальной код должен работать с индексом через эти функции:

```c
Index* createIndex(TreeType type);

void insertTerm(Index* idx, const char* term, int doc_id, const char* title);

Vector* lookupTerm(const Index* idx, const char* term);

void indexDocument(
    Index* idx,
    int doc_id,
    const char* title,
    const char** tokens,
    int n_tokens
);

void traverseIndex(
    const Index* idx,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
);

void saveIndex(const Index* idx, const char* path);

Index* loadIndex(const char* path, TreeType type);

void freeIndex(Index* idx);
```

---

## 11. Зачем нужен Index-слой

Без Index-слоя остальному коду пришлось бы знать, с каким деревом он работает:

```c
if (type == TREE_AVL) {
    avlInsert(...);
} else if (type == TREE_RB) {
    rbInsert(...);
} else {
    btreeInsert(...);
}
```

Это плохо, потому что такая логика начала бы дублироваться в разных местах.

С Index-слоем всё сводится к одному вызову:

```c
insertTerm(idx, term, doc_id, title);
```

А уже внутри `index.c` выбирается нужная структура.

---

## 12. Слой деревьев

Основные папки:

```text
avl/
rbtree/
btree/
```

Каждое дерево хранит отображение:

```text
term -> posting list
```

Где:

- `term` — слово из документа;
- `posting list` — список документов, где это слово встречается.

---

## 13. Общий контракт деревьев

Каждая структура должна поддерживать одинаковый набор операций.

### AVL

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

### Red-Black Tree

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

### B-tree

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

## 14. Правило для одинаковых ключей

Если ключа ещё нет в дереве, нужно создать новый узел или новую запись в узле.

Пример:

```text
python
```

Если `python` ещё нет в дереве:

```text
создаём новый ключ "python"
создаём новый posting list
добавляем туда документ
```

Если `python` уже есть в дереве:

```text
не создаём новый ключ
не меняем структуру дерева
просто добавляем документ в существующий posting list
```

---

## 15. Posting list

Основные файлы:

```text
posting.h
posting.c
```

Posting list — это список документов, в которых встречается конкретный терм.

Элемент posting list:

```c
typedef struct {
    int  doc_id;
    char title[MAX_TITLE_LEN];
} PostingEntry;
```

Пример:

```text
term = "python"

posting list:
[
    { doc_id: 1, title: "How to sort a list in Python" },
    { doc_id: 5, title: "Python tips" },
    { doc_id: 9, title: "Python memory usage" }
]
```

---

## 16. Правило против дубликатов в posting list

Если одно и то же слово встречается в одном документе несколько раз, документ не должен несколько раз попадать в posting list.

Пример документа:

```text
Python python python list sort
```

Для терма `python` должна быть только одна запись:

```text
python -> [(doc_id=1, title="...")]
```

А не так:

```text
python -> [
    (doc_id=1, title="..."),
    (doc_id=1, title="..."),
    (doc_id=1, title="...")
]
```

---

## 17. Vector

Основные файлы:

```text
common/vector.h
common/vector.c
```

`Vector` — это общий динамический массив.

Он используется для:

- posting list;
- результатов поиска;
- кандидатов fuzzy search, если он будет реализован;
- вспомогательных списков.

Базовые операции:

```c
Vector* createVector(size_t elem_size);
void    vectorFree(Vector* vector);

int     appendVectorItem(Vector* vector, const void* item);
void*   getVectorItem(Vector* vector, size_t index);
const void* getVectorItemConst(const Vector* vector, size_t index);

int     vectorReserve(Vector* vector, size_t new_capacity);
```

---

## 18. Формат `docs.jsonl`

Файл `docs.jsonl` содержит документы после препроцессинга.

Один документ — одна JSON-строка.

Пример:

```json
{"doc_id":1,"title":"How to sort a list in Python","tokens":["python","list","sort"]}
```

Поля:

| Поле | Значение |
|---|---|
| `doc_id` | числовой идентификатор документа |
| `title` | заголовок документа |
| `tokens` | список токенов документа |

Этот файл генерируется скриптом препроцессинга.

---

## 19. Формат JSON-вывода поиска

В режиме `--json` программа должна выводить только валидный JSON.

Пример:

```json
{
  "total": 47,
  "time_ms": 3.2,
  "results": [
    {
      "doc_id": 1234,
      "title": "How to sort a list?",
      "score": 3
    }
  ]
}
```

Поля:

| Поле | Значение |
|---|---|
| `total` | общее количество найденных документов |
| `time_ms` | время поиска в миллисекундах |
| `results` | top-10 результатов |
| `doc_id` | идентификатор документа |
| `title` | заголовок документа |
| `score` | оценка релевантности |

---

## 20. Важное правило для JSON-режима

В режиме `--json` нельзя печатать в `stdout` ничего, кроме JSON.

Нельзя:

```text
DEBUG: loading index...
{"total": 47, "time_ms": 3.2, "results": []}
```

Можно:

```json
{"total":47,"time_ms":3.2,"results":[]}
```

Если нужны debug-сообщения, их нужно писать в `stderr`.

Пример:

```c
fprintf(stderr, "DEBUG: loading index...\n");
```

---

## 21. Предлагаемый формат сохранённого индекса

Формат сохранения индекса должен быть одинаковым для всех деревьев.

Предлагаемый простой формат:

```text
term<TAB>doc_id:title|doc_id:title|doc_id:title
```

Пример:

```text
python    1:How to sort a list in Python|5:Python tips|9:Python memory usage
list      1:How to sort a list in Python|7:C linked list
tree      3:Balanced binary tree rotations
```

При загрузке:

1. Создаётся пустой индекс нужного типа.
2. Файл читается построчно.
3. Из каждой строки извлекается `term`.
4. Для каждой posting-записи вызывается `insertTerm`.

---

## 22. Ограничения формата сохранения

Так как в `title` могут быть специальные символы, формат сохранения может потребовать escaping.

Минимально допустимый вариант для MVP:

- заменить `\t`, `\n`, `\r`, `|` на пробелы при сохранении;
- при загрузке считать, что таких символов в title уже нет.

Более аккуратный вариант:

- сохранять индекс в JSONL;
- одна строка — один term;
- posting list хранится как массив объектов.

Пример:

```json
{"term":"python","postings":[{"doc_id":1,"title":"How to sort a list in Python"},{"doc_id":5,"title":"Python tips"}]}
```

Для MVP можно выбрать любой формат, но он должен быть одинаковым для всех трёх деревьев.

---

## 23. Ответственность по модулям

| Участник | Задача | Основные файлы |
|---|---|---|
| Коля | AVL-дерево | `avl/avl.c`, `avl/tests.c` |
| Ваня | Red-Black Tree | `rbtree/rbtree.c`, `rbtree/tests.c` |
| Ира | B-tree | `btree/btree.c`, `btree/tests.c` |
| Егор | Index/Search/serialization/CLI | `index/index.c`, `index/search.c`, `main.c` |
| Артём | Frontend/benchmarks/integration/docs | `app.py`, `scripts/benchmark.py`, `README.md`, `docs/report.md` |

---

## 24. Что нельзя менять без согласования

Без согласования с командой нельзя менять:

- сигнатуры функций в `.h`-файлах;
- формат `docs.jsonl`;
- формат JSON-вывода поиска;
- CLI-команды;
- расположение основных файлов;
- смысл `PostingEntry`;
- смысл `TreeType`.

Причина: эти вещи связывают разные части проекта. Если один человек изменит контракт, у остальных может сломаться код.

---

## 25. Definition of Done

Модуль считается готовым, если:

1. Код компилируется через `make`.
2. Соответствующие unit-тесты проходят.
3. Публичные интерфейсы не изменены без согласования.
4. Нет больших файлов датасета в Git.
5. Нет debug-сообщений в `stdout` в JSON-режиме.
6. Память освобождается корректно.
7. В PR кратко описано, что было сделано.
8. Код не ломает работу других модулей.

---

## 26. Главный принцип проекта

Каждый модуль должен быть максимально независимым.

Деревья не знают про CLI и frontend.

Search не знает про конкретные деревья.

Frontend не знает про внутренности C-кода.

Все части связываются через стабильные интерфейсы.