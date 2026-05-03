# Распределение задач

Этот документ фиксирует предварительное распределение работы по лабораторной.

Главная цель: каждый участник отвечает за отдельный модуль, который можно разрабатывать независимо от остальных.

## Общий принцип

Публичные интерфейсы в `.h`-файлах считаются контрактом.

Без согласования с капитаном не меняем:

- `posting.h`
- `index/index.h`
- `index/search.h`
- `avl/avl.h`
- `rbtree/rbtree.h`
- `btree/btree.h`
- формат JSON-вывода CLI
- формат `docs.jsonl`

## Распределение

| Участник | Задача | Файлы |
|---|---|---|
| Коля | Реализация AVL-дерева | `avl/avl.c`, `avl/tests.c` |
| Ваня | Реализация Red-Black Tree | `rbtree/rbtree.c`, `rbtree/tests.c` |
| Ира | Реализация B-tree | `btree/btree.c`, `btree/tests.c` |
| Егор | Индекс, поиск, сохранение/загрузка, CLI | `index/index.c`, `index/search.c`, `main.c` |
| Артём | Frontend, benchmark, документация, интеграция | `app.py`, `scripts/benchmark.py`, `README.md`, `docs/report.md` |

## Definition of Done

Задача считается готовой, если:

1. Код компилируется через `make`.
2. Соответствующие тесты проходят.
3. Публичные интерфейсы не сломаны.
4. Нет больших файлов датасета в Git.
5. Нет временных debug-print в JSON-режиме.
6. В коде нет очевидных утечек памяти.
7. В PR кратко описано, что было сделано.

## Правило для веток

Каждый работает в отдельной ветке:

```bash
feature/avl
feature/rbtree
feature/btree
feature/index-search
feature/frontend-benchmarks
```

## В main напрямую не пушим. Изменения идут через Pull Request.

---
### Если захотите потыкать заглушки, то:

Собрать всё:
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
```

Запустить проверку заглушек:
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\test_skeleton.ps1
```

---
