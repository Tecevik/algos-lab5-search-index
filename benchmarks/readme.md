# Benchmarks

Эта папка предназначена для результатов экспериментов.

Здесь не реализуется логика поиска.  
Benchmark-скрипты должны запускать готовое CLI-приложение `./app`.

## Что сравниваем

Сравниваются три структуры данных:

- AVL;
- Red-Black Tree;
- B-tree.

## Основные метрики

### 1. Время построения индекса

Проверяем, сколько времени занимает команда индексации:

```bash
./app index --type=avl --data=data/processed/docs_50000.jsonl --index=data/index_avl_50000.txt
```

Планируемые размеры датасета:

- 50 000 документов;
- 200 000 документов;
- 500 000 документов.

Пример таблицы результатов:

```csv
docs_count,tree_type,time_sec
50000,avl,1.42
50000,rb,1.21
50000,btree,0.97
```

---

### 2. Время поиска

Проверяем среднее время поиска по набору запросов.

Отдельно сравниваем запросы длины:

- 1 слово;
- 2 слова;
- 3 слова.

Пример команды:

```bash
./app search --type=avl --index=data/index_avl_50000.txt --json "python list"
```

Пример таблицы результатов:

```csv
query_words,tree_type,avg_time_ms,queries_count
1,avl,0.08,1000
1,rb,0.09,1000
1,btree,0.06,1000
2,avl,0.14,1000
2,rb,0.15,1000
2,btree,0.11,1000
```

---

### 3. Использование памяти

Проверяем максимальное потребление памяти во время индексации или поиска.

В C-коде это можно измерять через `getrusage`.

Пример таблицы результатов:

```csv
docs_count,tree_type,memory_mb
50000,avl,78
50000,rb,81
50000,btree,65
500000,avl,690
500000,rb,720
500000,btree,540
```

---

## Структура папки

```text
benchmarks/
├── README.md
├── results/
│   └── .gitkeep
└── plots/
    └── .gitkeep
```

## Что лежит в `results/`

В `results/` будут лежать таблицы с результатами экспериментов:

```text
benchmarks/results/index_time.csv
benchmarks/results/search_time.csv
benchmarks/results/memory_usage.csv
```

Эти файлы можно добавить ближе к финальной версии проекта, когда появятся реальные результаты.

## Что лежит в `plots/`

В `plots/` будут лежать графики:

```text
benchmarks/plots/index_time.png
benchmarks/plots/search_time.png
benchmarks/plots/memory_usage.png
```

Эти графики потом можно вставить в README, отчёт или презентацию.

## Важно

Benchmark-слой не должен напрямую вызывать функции из C-кода.

Правильно:

```bash
./app search --type=avl --index=data/index_avl.txt --json "python list"
```

Неправильно:

```text
вызывать avlSearch(), rbSearch() или btreeSearch() напрямую из benchmark-скрипта
```

Причина: benchmark должен проверять проект так же, как им будет пользоваться frontend и демонстрация.