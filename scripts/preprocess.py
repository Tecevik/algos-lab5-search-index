"""
Препроцессинг датасета Stack Overflow для инвертированного индекса.

Ответственный: Артём.

Назначение:
Этот скрипт превращает сырой Questions.csv в docs.jsonl,
который потом читает C-программа.

Почему preprocessing вынесен в Python:
- CSV и HTML удобнее чистить в Python;
- C-код должен заниматься индексом, а не грязным парсингом текста;
- результат preprocessing имеет простой формат JSONL.

Пример запуска:

    python scripts/preprocess.py --input data/Questions.csv --output data/processed/docs.jsonl

Для быстрой проверки на части датасета:

    python scripts/preprocess.py --input data/Questions.csv --output data/processed/docs_50000.jsonl --limit 50000
"""

import argparse
import csv
import html
import json
import re
import sys
from pathlib import Path


def tokenize(text: str) -> list[str]:
    """
    Превращает сырой текст в список токенов.

    Что делаем:
    1. Декодируем HTML-сущности.
    2. Удаляем HTML-теги.
    3. Приводим к нижнему регистру.
    4. Оставляем только буквы, цифры и подчёркивания.
    5. Убираем слишком короткие токены.
    6. Убираем дубликаты токенов внутри одного документа.
    """
    text = html.unescape(text)
    text = re.sub(r"<[^>]+>", " ", text)
    text = text.lower()
    text = re.sub(r"[^\w\s]", " ", text)

    tokens = [w for w in text.split() if len(w) > 2]

    # Сохраняем порядок, но убираем дубликаты внутри документа.
    return list(dict.fromkeys(tokens))


def preprocess(input_path: Path, output_path: Path, limit: int | None) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    count = 0

    with open(input_path, encoding="utf-8", newline="") as f, \
            open(output_path, "w", encoding="utf-8") as out:

        reader = csv.DictReader(f)

        for row in reader:
            if limit is not None and count >= limit:
                break

            raw_id = row.get("Id", "0")
            title = row.get("Title", "") or ""
            body = row.get("Body", "") or ""

            try:
                doc_id = int(raw_id)
            except ValueError:
                continue

            tokens = tokenize(title + " " + body)

            if not tokens:
                continue

            doc = {
                "doc_id": doc_id,
                "title": title,
                "tokens": tokens,
            }

            out.write(json.dumps(doc, ensure_ascii=False) + "\n")
            count += 1

            if count % 10_000 == 0:
                print(f"Обработано документов: {count}", file=sys.stderr)

    print(f"Готово. Всего документов: {count}", file=sys.stderr)
    print(f"Результат: {output_path}", file=sys.stderr)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Препроцессинг Stack Overflow Questions.csv в docs.jsonl"
    )

    parser.add_argument(
        "--input",
        required=True,
        type=Path,
        help="Путь к data/Questions.csv",
    )

    parser.add_argument(
        "--output",
        required=True,
        type=Path,
        help="Путь к выходному docs.jsonl",
    )

    parser.add_argument(
        "--limit",
        type=int,
        default=None,
        help="Максимальное количество документов для обработки",
    )

    args = parser.parse_args()

    if not args.input.exists():
        print(f"Ошибка: файл не найден: {args.input}", file=sys.stderr)
        sys.exit(1)

    preprocess(args.input, args.output, args.limit)


if __name__ == "__main__":
    main()