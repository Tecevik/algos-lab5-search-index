# Index and Search

Ответственный: Егор.

Эта папка содержит слой индекса, поиска, сериализации и загрузки индекса.

---

## Основные файлы

```text
index/
├── README.md
├── index.h
├── index.c
├── search.h
└── search.c
```

---

## Назначение `index.c`

`index.c` — это единая обёртка над тремя деревьями:

- AVL;
- Red-Black Tree;
- B-tree.

Остальной код не должен напрямую работать с конкретными деревьями.

Правильно:

```c
insertTerm(idx, term, doc_id, title);
lookupTerm(idx, term);
```

Неправильно:

```c
avlInsert(...);
rbSearch(...);
btreeInsert(...);
```

---

## Что нужно реализовать в `index.c`

- создание индекса нужного типа;
- освобождение индекса;
- вставку терма;
- поиск терма;
- индексацию документа;
- обход индекса;
- сохранение индекса в файл;
- загрузку индекса из файла;
- полный pipeline индексации из `docs.jsonl`.

---

## Назначение `search.c`

`search.c` реализует поиск по уже построенному индексу.

Он должен:

- токенизировать пользовательский запрос;
- получить posting list для каждого токена;
- пересечь posting list'ы;
- сформировать top-10 результатов;
- посчитать время поиска;
- вывести результат в текстовом формате;
- вывести результат в JSON-формате.

---

## Семантика поиска

Базовая семантика — AND.

Запрос:

```text
python list
```

означает:

```text
найти документы, где есть и "python", и "list"
```

---

## JSON-режим

В режиме `--json` программа должна печатать в `stdout` только валидный JSON.

Нельзя:

```text
DEBUG: loading index
{"total":0,"time_ms":0,"results":[]}
```

Можно:

```json
{"total":0,"time_ms":0,"results":[]}
```

Debug-сообщения нужно писать в `stderr`.

---

## Проверка: 

```
gcc -Wall -Wextra -std=c11 -O2 -g -o app.exe common/vector.c posting.c avl/avl.c rbtree/rbtree.c btree/btree.c index/index.c index/search.c main.c
```

Проверка индексации:

```
.\app.exe index --type=avl --data=data/sample/docs.jsonl --index=data/index_avl.txt
```

Проверка поиска:

```
.\app.exe search --type=avl --index=data/index_avl.txt --json "python list"
```

На этапе заглушек поиск может возвращать пустой результат:

```json
{"total":0,"time_ms":0.000,"results":[]}
```

Это нормально. Главное, чтобы программа не падала и выводила валидный JSON.