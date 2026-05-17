# Fuzzy Search / Плавающий поиск

Ответственный: Егор.

## 1. Назначение

Эта часть отвечает за реализацию нечёткого поиска по индексу.

Обычный поиск работает только по точному совпадению токенов:

```bash
./app search --type=avl --index=data/index_avl.txt --json "python list"
```

Если пользователь ошибся в слове, обычный поиск ничего не найдёт:

```bash
./app search --type=avl --index=data/index_avl.txt --json "pyton"
```

Fuzzy search должен находить документы даже при небольших ошибках:

```bash
./app search --type=avl --index=data/index_avl.txt --json --fuzzy --max-dist=2 "pyton"
```

Примеры:

```text
pyton     -> python
memry     -> memory
dictonary -> dictionary
balnced   -> balanced
```

---

## 2. Что нужно реализовать

Нужно добавить второй режим поиска:

1. **Exact search** — уже существующий точный поиск.
2. **Fuzzy search** — нечёткий поиск похожих слов через расстояние Левенштейна.

Важно: fuzzy search не заменяет обычный поиск. Он добавляется отдельным режимом через CLI-флаг.

---

## 3. Требуемые CLI-команды

Обычный поиск должен остаться без изменений:

```bash
./app search --type=avl --index=data/index_avl.txt --json "python list"
```

Новый fuzzy-режим:

```bash
./app search --type=avl --index=data/index_avl.txt --json --fuzzy --max-dist=2 "pyton list"
```

Параметры:

| Флаг | Назначение |
|---|---|
| `--fuzzy` | включает нечёткий поиск |
| `--max-dist=N` | максимальное расстояние Левенштейна |
| `--json` | выводит результат в JSON |
| `--type=avl/rb/btree` | выбирает структуру данных |

Если `--max-dist` не указан, использовать значение по умолчанию:

```text
max_dist = 2
```

---

## 4. Где писать код

Рекомендуемая структура:

```text
index/
├── index.h
├── index.c
├── search.h
├── search.c
├── fuzzy.h
└── fuzzy.c
```

Новые файлы:

```text
index/fuzzy.h
index/fuzzy.c
```

`main.c` нужно только немного расширить: добавить разбор флагов `--fuzzy` и `--max-dist=N`.

---

## 5. Интерфейс `fuzzy.h`

Создать файл `index/fuzzy.h`:

```c
#pragma once

#include "search.h"

#define MAX_TERM_LEN 256

typedef struct {
    char    term[MAX_TERM_LEN];
    int     distance;
    Vector* postings;
} FuzzyCandidate;

int levenshteinDistance(const char* a, const char* b);

Vector* fuzzyFindCandidates(
    Index* idx,
    const char* term,
    int max_distance
);

SearchResults* fuzzySearch(
    Index* idx,
    const char* query,
    int max_distance
);
```

---

## 6. Логика работы fuzzy search

Обычный поиск работает так:

```text
query = "python list"

1. lookupTerm("python")
2. lookupTerm("list")
3. intersectPostings(...)
4. printResultsJSON(...)
```

Fuzzy search работает иначе:

```text
query = "pyton list"

1. Разбить запрос на токены: pyton, list.
2. Для каждого токена обойти весь индекс через traverseIndex().
3. Для каждого терма в индексе посчитать расстояние Левенштейна.
4. Если distance <= max_distance, считать терм кандидатом.
5. Собрать документы из posting list'ов найденных кандидатов.
6. Посчитать score.
7. Вернуть top-10 результатов.
```

---

## 7. Почему нужен `traverseIndex`

Для точного поиска можно сделать:

```c
lookupTerm(idx, "python");
```

Но для fuzzy search это не подходит:

```c
lookupTerm(idx, "pyton");
```

Такого ключа в индексе нет.

Поэтому нужно пройти по всем ключам индекса:

```c
traverseIndex(idx, callback, ctx);
```

И сравнить каждый ключ с пользовательским словом:

```c
distance = levenshteinDistance(query_term, index_term);
```

Fuzzy search зависит от корректной работы обхода у всех трёх деревьев:

```text
avlTraverse
rbTraverse
btreeTraverse
```

---

## 8. Расстояние Левенштейна

Нужно реализовать функцию:

```c
int levenshteinDistance(const char* a, const char* b);
```

Она должна считать минимальное количество операций:

```text
- вставка символа;
- удаление символа;
- замена символа.
```

Примеры:

| a | b | distance |
|---|---|---:|
| `pyton` | `python` | 1 |
| `memry` | `memory` | 1 |
| `cat` | `cut` | 1 |
| `tree` | `tree` | 0 |
| `abc` | `xyz` | 3 |

---

## 9. Поиск кандидатов

Функция:

```c
Vector* fuzzyFindCandidates(
    Index* idx,
    const char* term,
    int max_distance
);
```

Должна вернуть список похожих термов.

Пример:

```text
term = "pyton"
max_distance = 2
```

Если в индексе есть:

```text
python
pytest
list
tree
memory
```

То кандидатами могут стать:

```text
python, distance = 1
```

Каждый кандидат хранит:

```c
typedef struct {
    char    term[MAX_TERM_LEN];
    int     distance;
    Vector* postings;
} FuzzyCandidate;
```

Важно: `postings` нужно брать из найденного терма в индексе. Кандидат должен знать, какие документы соответствуют этому терму.

---

## 10. Ранжирование

Для MVP можно использовать простую формулу:

```text
score = matched_terms * 10 - total_distance
```

Где:

```text
matched_terms  — сколько слов запроса совпало с документом;
total_distance — сумма расстояний Левенштейна по найденным словам.
```

Пример:

```text
query = "pyton list"
```

Документ содержит:

```text
python, list
```

Тогда:

```text
pyton -> python, distance = 1
list  -> list,   distance = 0

matched_terms = 2
total_distance = 1

score = 2 * 10 - 1 = 19
```

Чем больше `score`, тем выше документ в выдаче.

---

## 11. Формат результата

Fuzzy search должен возвращать результат в том же формате, что и обычный поиск:

```json
{
  "total": 1,
  "time_ms": 0.123,
  "results": [
    {
      "doc_id": 1,
      "title": "How to sort a list in Python",
      "score": 19
    }
  ]
}
```

Формат JSON менять нельзя, потому что его использует frontend.

В режиме `--json` в stdout должен попадать только JSON.

Нельзя выводить перед JSON:

```text
Fuzzy search started
Found candidates...
```

Правильно:

```json
{"total":1,"time_ms":0.000,"results":[{"doc_id":1,"title":"How to sort a list in Python","score":19}]}
```

Если нужен debug-вывод, писать его в `stderr`.

---

## 12. Изменения в `main.c`

В `main.c` нужно добавить переменные:

```c
int fuzzy_mode = 0;
int max_dist = 2;
```

При разборе аргументов добавить:

```c
} else if (strcmp(argv[i], "--fuzzy") == 0) {
    fuzzy_mode = 1;
} else if (strncmp(argv[i], "--max-dist=", 11) == 0) {
    max_dist = atoi(argv[i] + 11);
}
```

В `runSearch` нужно передавать параметры:

```c
static void runSearch(
    TreeType type,
    const char* idx_path,
    const char* query,
    int json_out,
    int fuzzy_mode,
    int max_dist
)
```

И выбирать режим поиска:

```c
SearchResults* sr = NULL;

if (fuzzy_mode) {
    sr = fuzzySearch(idx, query, max_dist);
} else {
    sr = search(idx, query);
}
```

Также в `main.c` нужно подключить заголовок:

```c
#include "index/fuzzy.h"
```

---

## 13. Что нельзя ломать

Нельзя менять без согласования:

```text
- формат обычного CLI;
- формат JSON;
- сигнатуры уже существующих функций;
- работу обычного exact search;
- публичные интерфейсы деревьев;
- структуру SearchResults/SearchResult без необходимости.
```

Обычные команды должны продолжать работать:

```bash
./app search --type=avl --index=data/index_avl.txt --json "python list"
```

---

## 14. Сборка

Из корня проекта:

```powershell
gcc -Wall -Wextra -std=c11 -O2 -g -I. -Icommon -o app.exe common/vector.c posting.c avl/avl.c rbtree/rbtree.c btree/btree.c index/index.c index/search.c index/fuzzy.c main.c
```

Важно: после добавления `index/fuzzy.c` его нужно включать в команду сборки.

---

## 15. Проверка exact search

Сначала проверить, что обычный поиск не сломался.

### AVL

```powershell
.\app.exe index --type=avl --data=data/sample/docs.jsonl --index=data/index_avl.txt
.\app.exe search --type=avl --index=data/index_avl.txt --json "python list"
```

Ожидаемо:

```json
{"total":1,"time_ms":0.000,"results":[{"doc_id":1,"title":"How to sort a list in Python","score":2}]}
```

### RB

```powershell
.\app.exe index --type=rb --data=data/sample/docs.jsonl --index=data/index_rb.txt
.\app.exe search --type=rb --index=data/index_rb.txt --json "python list"
```

### B-tree

```powershell
.\app.exe index --type=btree --data=data/sample/docs.jsonl --index=data/index_btree.txt
.\app.exe search --type=btree --index=data/index_btree.txt --json "python list"
```

---

## 16. Проверка fuzzy search

Перед fuzzy-поиском индекс должен быть создан командой `index`.

### AVL

```powershell
.\app.exe index --type=avl --data=data/sample/docs.jsonl --index=data/index_avl.txt
.\app.exe search --type=avl --index=data/index_avl.txt --json --fuzzy --max-dist=2 "pyton"
```

Ожидаемо: должен найти документ с `python`.

```powershell
.\app.exe search --type=avl --index=data/index_avl.txt --json --fuzzy --max-dist=2 "pyton list"
```

Ожидаемо: должен найти документ:

```text
How to sort a list in Python
```

### RB

```powershell
.\app.exe index --type=rb --data=data/sample/docs.jsonl --index=data/index_rb.txt
.\app.exe search --type=rb --index=data/index_rb.txt --json --fuzzy --max-dist=2 "pyton"
.\app.exe search --type=rb --index=data/index_rb.txt --json --fuzzy --max-dist=2 "pyton list"
```

### B-tree

```powershell
.\app.exe index --type=btree --data=data/sample/docs.jsonl --index=data/index_btree.txt
.\app.exe search --type=btree --index=data/index_btree.txt --json --fuzzy --max-dist=2 "pyton"
.\app.exe search --type=btree --index=data/index_btree.txt --json --fuzzy --max-dist=2 "pyton list"
```

---

## 17. Проверочные запросы

Для sample dataset проверить:

| Запрос | Режим | Ожидаемый смысл |
|---|---|---|
| `python` | exact | найти документы с `python` |
| `pyton` | fuzzy | найти документы с `python` |
| `python list` | exact | найти документы с обоими словами |
| `pyton list` | fuzzy | найти документы с `python` и `list` |
| `balnced tree` | fuzzy | найти документы с `balanced tree` |
| `banana` | exact | ничего не найти |
| `banan` | fuzzy | скорее всего ничего не найти, если похожего терма нет |

---

## 18. Проверка для всех деревьев

Результаты для `avl`, `rb`, `btree` должны совпадать по найденным документам.

Пример:

```powershell
.\app.exe search --type=avl --index=data/index_avl.txt --json --fuzzy --max-dist=2 "pyton"
.\app.exe search --type=rb --index=data/index_rb.txt --json --fuzzy --max-dist=2 "pyton"
.\app.exe search --type=btree --index=data/index_btree.txt --json --fuzzy --max-dist=2 "pyton"
```

Все три команды должны находить один и тот же документ.

Время `time_ms` может отличаться.

---

## 19. Возможные проблемы

### Проблема 1. Fuzzy search всегда возвращает пусто

Проверить:

```text
- работает ли traverseIndex();
- реализованы ли traverse-функции во всех деревьях;
- не слишком маленький max_dist;
- правильно ли считается levenshteinDistance();
- есть ли нужные термы в сохранённом индексе.
```

---

### Проблема 2. Exact search сломался

Проверить, что в `main.c` режим выбирается правильно:

```c
if (fuzzy_mode) {
    sr = fuzzySearch(idx, query, max_dist);
} else {
    sr = search(idx, query);
}
```

Без `--fuzzy` должен вызываться обычный `search()`.

---

### Проблема 3. JSON не парсится во frontend

Проверить, что при `--json` stdout содержит только JSON.

Плохо:

```text
Fuzzy search started
{"total":1,"time_ms":0.000,"results":[...]}
```

Хорошо:

```json
{"total":1,"time_ms":0.000,"results":[...]}
```

---

### Проблема 4. На одном дереве работает, на другом нет

Скорее всего, проблема в `traverse` конкретного дерева.

Проверить отдельно:

```text
avlTraverse
rbTraverse
btreeTraverse
```

Fuzzy search зависит от полного обхода индекса.

---

### Проблема 5. Сборка падает после добавления fuzzy search

Проверить, что `index/fuzzy.c` добавлен в команду сборки:

```powershell
gcc -Wall -Wextra -std=c11 -O2 -g -I. -Icommon -o app.exe common/vector.c posting.c avl/avl.c rbtree/rbtree.c btree/btree.c index/index.c index/search.c index/fuzzy.c main.c
```

Также проверить, что в `main.c` подключён заголовок:

```c
#include "index/fuzzy.h"
```

---

## 20. Definition of Done

Часть считается готовой, если:

- [ ] добавлены `index/fuzzy.h` и `index/fuzzy.c`;
- [ ] реализован `levenshteinDistance`;
- [ ] реализован поиск похожих термов через `traverseIndex`;
- [ ] реализован `fuzzyFindCandidates`;
- [ ] реализован `fuzzySearch`;
- [ ] добавлены CLI-флаги `--fuzzy` и `--max-dist=N`;
- [ ] обычный exact search не сломан;
- [ ] fuzzy search работает для AVL;
- [ ] fuzzy search работает для RB;
- [ ] fuzzy search работает для B-tree;
- [ ] JSON-формат совпадает с обычным поиском;
- [ ] в PR приложены команды проверки и примеры вывода.

---

## 21. Что приложить в Pull Request

В описание PR вставить:

```markdown
## Что сделано

- Добавлен fuzzy search через расстояние Левенштейна.
- Добавлены файлы `index/fuzzy.h` и `index/fuzzy.c`.
- Добавлены CLI-флаги `--fuzzy` и `--max-dist=N`.
- Exact search не изменён и продолжает работать.
- Fuzzy search проверен на AVL, RB и B-tree.

## Проверка

Сборка:

```powershell
gcc -Wall -Wextra -std=c11 -O2 -g -I. -Icommon -o app.exe common/vector.c posting.c avl/avl.c rbtree/rbtree.c btree/btree.c index/index.c index/search.c index/fuzzy.c main.c
```

Индексация:

```powershell
.\app.exe index --type=avl --data=data/sample/docs.jsonl --index=data/index_avl.txt
.\app.exe index --type=rb --data=data/sample/docs.jsonl --index=data/index_rb.txt
.\app.exe index --type=btree --data=data/sample/docs.jsonl --index=data/index_btree.txt
```

Fuzzy search:

```powershell
.\app.exe search --type=avl --index=data/index_avl.txt --json --fuzzy --max-dist=2 "pyton"
.\app.exe search --type=rb --index=data/index_rb.txt --json --fuzzy --max-dist=2 "pyton"
.\app.exe search --type=btree --index=data/index_btree.txt --json --fuzzy --max-dist=2 "pyton"
```

Ожидаемо: все три структуры находят документ с `python`.
```

---

## 22. Что не входит в задачу

Не нужно реализовывать:

```text
- autocomplete;
- prefix search;
- regex search;
- BM25;
- морфологический анализ;
- исправление опечаток отдельным словарём;
- подсветку совпадений;
- сложное ранжирование как в Elasticsearch.
```

Нужен один понятный hard-режим:

```text
fuzzy search через Levenshtein distance.
```