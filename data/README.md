# Data

Эта папка предназначена для датасета, обработанных документов и сохранённых индексов.

Большие файлы датасета не хранятся в Git.

---

## Ожидаемая структура

```text
data/
├── README.md
├── sample/
│   └── docs.jsonl
├── processed/
│   └── docs.jsonl
├── Questions.csv
├── Answers.csv
├── Tags.csv
├── index_avl.txt
├── index_rb.txt
└── index_btree.txt
```

---

## Что коммитим в Git

В Git можно коммитить:

```text
data/README.md
data/sample/docs.jsonl
```

`sample/docs.jsonl` нужен для быстрой проверки проекта без большого датасета.

---

## Что НЕ коммитим в Git

В Git не нужно коммитить:

```text
data/Questions.csv
data/Answers.csv
data/Tags.csv
data/processed/
data/index_*.txt
```

Причины:

- исходный датасет большой;
- обработанные данные можно пересоздать;
- индексы можно пересобрать;
- у разных участников могут быть разные локальные версии датасета.

---

## Исходный датасет

Используется Stack Overflow dataset.

Ожидаемые исходные файлы:

```text
data/Questions.csv
data/Answers.csv
data/Tags.csv
```

Для MVP нужен только файл:

```text
data/Questions.csv
```

Поля, которые используются из `Questions.csv`:

| Поле | Назначение |
|---|---|
| `Id` | идентификатор документа |
| `Title` | заголовок вопроса |
| `Body` | текст вопроса |

---

## Зачем нужен preprocessing

C-программа не читает `Questions.csv` напрямую.

Сначала Python-скрипт превращает сырой CSV в простой формат `docs.jsonl`.

Общий pipeline:

```text
Questions.csv
     ↓
scripts/preprocess.py
     ↓
data/processed/docs.jsonl
     ↓
./app index
     ↓
data/index_avl.txt / data/index_rb.txt / data/index_btree.txt
```

---

## Запуск preprocessing

Пример запуска:

```bash
python scripts/preprocess.py --input data/Questions.csv --output data/processed/docs.jsonl
```

Для ограничения количества документов:

```bash
python scripts/preprocess.py --input data/Questions.csv --output data/processed/docs_50000.jsonl --limit 50000
```

---

## Формат `docs.jsonl`

Файл `docs.jsonl` содержит по одному документу на строку.

Пример:

```json
{"doc_id":1,"title":"How to sort a list in Python","tokens":["python","list","sort"]}
```

Поля:

| Поле | Значение |
|---|---|
| `doc_id` | числовой идентификатор документа |
| `title` | заголовок вопроса |
| `tokens` | список токенов из `Title + Body` |

---

## Sample dataset

Для быстрой проверки без полного датасета есть маленький файл:

```text
data/sample/docs.jsonl
```

Пример содержимого:

```json
{"doc_id":1,"title":"How to sort a list in Python","tokens":["python","list","sort"]}
```

Этот файл можно использовать для:

- проверки парсинга;
- минимального end-to-end теста;
- отладки индексации;
- демонстрации структуры `docs.jsonl`.

---

## Сохранённые индексы

После запуска индексации появляются файлы:

```text
data/index_avl.txt
data/index_rb.txt
data/index_btree.txt
```

Примеры команд:

```bash
./app index --type=avl --data=data/processed/docs.jsonl --index=data/index_avl.txt
./app index --type=rb --data=data/processed/docs.jsonl --index=data/index_rb.txt
./app index --type=btree --data=data/processed/docs.jsonl --index=data/index_btree.txt
```

Эти файлы не нужно коммитить, потому что они генерируются автоматически.

---

## Важно

Папка `data/` содержит локальные данные.

Если у одного участника нет полного датасета, проект всё равно должен запускаться на `data/sample/docs.jsonl`.