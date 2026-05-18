from __future__ import annotations

import json
import subprocess
from pathlib import Path

import streamlit as st


# ====== Базовые пути ======

PROJECT_ROOT = Path(__file__).resolve().parent

APP_EXE = PROJECT_ROOT / "app.exe"

DATA_PATH = PROJECT_ROOT / "data" / "sample" / "docs.jsonl"

INDEX_PATHS = {
    "avl": PROJECT_ROOT / "data" / "index_avl.txt",
    "rb": PROJECT_ROOT / "data" / "index_rb.txt",
    "btree": PROJECT_ROOT / "data" / "index_btree.txt",
}

TREE_LABELS = {
    "avl": "AVL Tree",
    "rb": "Red-Black Tree",
    "btree": "B-tree",
}


# ====== Вспомогательные функции ======

def format_command(args: list[str]) -> str:
    """
    Делает команду удобной для отображения в интерфейсе.
    """
    return " ".join(f'"{arg}"' if " " in arg else arg for arg in args)


def run_command(args: list[str]) -> subprocess.CompletedProcess[str]:
    """
    Запускает C-приложение из корня проекта.

    shell=False используется специально:
    так безопаснее и меньше проблем с кавычками в запросах.
    """
    return subprocess.run(
        args,
        cwd=PROJECT_ROOT,
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
        shell=False,
    )


def build_app() -> subprocess.CompletedProcess[str]:
    """
    Собирает app.exe.

    Важно: index/fuzzy.c обязательно должен быть в команде сборки,
    иначе fuzzy search не подключится.
    """
    args = [
        "gcc",
        "-Wall",
        "-Wextra",
        "-std=c11",
        "-O2",
        "-g",
        "-I.",
        "-Icommon",
        "-o",
        "app.exe",
        "common/vector.c",
        "posting.c",
        "avl/avl.c",
        "rbtree/rbtree.c",
        "btree/btree.c",
        "index/index.c",
        "index/search.c",
        "index/fuzzy.c",
        "main.c",
    ]

    return run_command(args)


def build_index(tree_type: str, data_path: Path, index_path: Path) -> subprocess.CompletedProcess[str]:
    """
    Запускает индексацию для выбранного дерева.
    """
    args = [
        str(APP_EXE),
        "index",
        f"--type={tree_type}",
        f"--data={data_path.relative_to(PROJECT_ROOT)}",
        f"--index={index_path.relative_to(PROJECT_ROOT)}",
    ]

    return run_command(args)


def run_search(
        tree_type: str,
        index_path: Path,
        query: str,
        fuzzy: bool,
        max_dist: int,
) -> tuple[subprocess.CompletedProcess[str], list[str]]:
    """
    Запускает поиск.

    Обычный режим:
        app.exe search --type=avl --index=data/index_avl.txt --json "python list"

    Fuzzy-режим:
        app.exe search --type=avl --index=data/index_avl.txt --json --fuzzy --max-dist=2 "pyton list"
    """
    args = [
        str(APP_EXE),
        "search",
        f"--type={tree_type}",
        f"--index={index_path.relative_to(PROJECT_ROOT)}",
        "--json",
    ]

    if fuzzy:
        args.append("--fuzzy")
        args.append(f"--max-dist={max_dist}")

    args.append(query)

    result = run_command(args)
    return result, args


def parse_search_json(stdout: str) -> dict:
    """
    Парсит JSON, который возвращает C-программа.

    В режиме --json stdout должен содержать только JSON.
    """
    return json.loads(stdout)


# ====== Интерфейс ======

st.set_page_config(
    page_title="Search Index Demo",
    page_icon="🔎",
    layout="wide",
)

st.title("Search Index Demo")
st.caption("AVL / Red-Black Tree / B-tree · Exact Search · Fuzzy Search")

with st.sidebar:
    st.header("Настройки")

    tree_type = st.selectbox(
        "Структура данных",
        options=["avl", "rb", "btree"],
        format_func=lambda x: TREE_LABELS[x],
    )

    index_path = INDEX_PATHS[tree_type]

    st.divider()

    st.subheader("Сборка")

    if st.button("Собрать app.exe"):
        with st.spinner("Сборка C-приложения..."):
            result = build_app()

        if result.returncode == 0:
            st.success("app.exe успешно собран")
        else:
            st.error("Ошибка сборки")
            st.code(result.stderr or result.stdout, language="text")

    st.divider()

    st.subheader("Индексация")

    data_path_input = st.text_input(
        "Путь к docs.jsonl",
        value=str(DATA_PATH.relative_to(PROJECT_ROOT)),
    )

    custom_data_path = PROJECT_ROOT / data_path_input

    st.text_input(
        "Файл индекса",
        value=str(index_path.relative_to(PROJECT_ROOT)),
        disabled=True,
    )

    if st.button("Построить индекс"):
        if not APP_EXE.exists():
            st.error("app.exe не найден. Сначала собери C-приложение.")
        elif not custom_data_path.exists():
            st.error(f"Файл данных не найден: {custom_data_path}")
        else:
            with st.spinner(f"Индексация через {TREE_LABELS[tree_type]}..."):
                result = build_index(tree_type, custom_data_path, index_path)

            if result.returncode == 0:
                st.success("Индекс построен")
                st.code(result.stdout, language="text")
            else:
                st.error("Ошибка индексации")
                st.code(result.stderr or result.stdout, language="text")

    st.divider()

    st.subheader("Режим поиска")

    fuzzy = st.toggle("Fuzzy search", value=False)

    max_dist = st.slider(
        "max-dist",
        min_value=0,
        max_value=3,
        value=2,
        disabled=not fuzzy,
    )


# ====== Основная зона ======

col_left, col_right = st.columns([2, 1])

with col_left:
    query = st.text_input(
        "Поисковый запрос",
        value="pyton list" if fuzzy else "python list",
        placeholder="Например: python list или pyton list",
    )

with col_right:
    st.write("")
    st.write("")
    search_clicked = st.button("Искать", type="primary", use_container_width=True)


if search_clicked:
    if not APP_EXE.exists():
        st.error("app.exe не найден. Сначала нажми «Собрать app.exe» в боковой панели.")
        st.stop()

    if not index_path.exists():
        st.error(
            f"Файл индекса не найден: {index_path.relative_to(PROJECT_ROOT)}. "
            "Сначала нажми «Построить индекс»."
        )
        st.stop()

    if not query.strip():
        st.error("Введите поисковый запрос.")
        st.stop()

    with st.spinner("Выполняется поиск..."):
        result, command = run_search(
            tree_type=tree_type,
            index_path=index_path,
            query=query.strip(),
            fuzzy=fuzzy,
            max_dist=max_dist,
        )

    st.subheader("Команда")

    st.code(format_command(command), language="powershell")

    if result.returncode != 0:
        st.error("Ошибка поиска")
        st.code(result.stderr or result.stdout, language="text")
        st.stop()

    try:
        data = parse_search_json(result.stdout)
    except json.JSONDecodeError:
        st.error("C-программа вернула невалидный JSON")
        st.write("stdout:")
        st.code(result.stdout, language="text")
        st.write("stderr:")
        st.code(result.stderr, language="text")
        st.stop()

    total = data.get("total", 0)
    time_ms = data.get("time_ms", 0.0)
    results = data.get("results", [])

    metric_col_1, metric_col_2, metric_col_3 = st.columns(3)

    with metric_col_1:
        st.metric("Найдено документов", total)

    with metric_col_2:
        st.metric("Время поиска, ms", time_ms)

    with metric_col_3:
        st.metric("Режим", "Fuzzy" if fuzzy else "Exact")

    st.subheader("Результаты")

    if not results:
        st.info("Ничего не найдено.")
    else:
        st.dataframe(
            results,
            use_container_width=True,
            hide_index=True,
        )

    with st.expander("Raw JSON"):
        st.code(result.stdout, language="json")


# ====== Подсказки ======

with st.expander("Примеры запросов"):
    st.markdown(
        """
### Exact search

```text
python
python list
tree
banana
        Fuzzy search
pyton
pyton list
pyton lisst
balnced tree
tre
        
        Ожидаемое поведение
python в exact search находит   документ с Python.
pyton в exact search ничего не находит.
pyton в fuzzy search находит python.
--max-dist=0 требует полного совпадения.
--max-dist=1 или --max-dist=2 допускает опечатки.
"""
    )