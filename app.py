from __future__ import annotations

import json
import subprocess
from pathlib import Path

import streamlit as st
import streamlit.components.v1 as components


# =============================================================================
# Paths
# =============================================================================

PROJECT_ROOT = Path(__file__).resolve().parent
APP_EXE = PROJECT_ROOT / "app.exe"

DEFAULT_DATA_PATH = PROJECT_ROOT / "data" / "sample" / "docs.jsonl"
DEMO_DATA_PATH = PROJECT_ROOT / "data" / "sample" / "demo_docs.jsonl"
BENCHMARK_50K_DATA_PATH = PROJECT_ROOT / "data" / "sample" / "benchmark_docs_50000.jsonl"
BENCHMARK_100K_DATA_PATH = PROJECT_ROOT / "data" / "sample" / "benchmark_docs_100000.jsonl"


DATASETS = {
    "Минимальный sample": {
        "path": DEFAULT_DATA_PATH,
        "suffix": "sample",
    },
    "Demo dataset": {
        "path": DEMO_DATA_PATH,
        "suffix": "demo",
    },
    "Benchmark 50k": {
        "path": BENCHMARK_50K_DATA_PATH,
        "suffix": "50000",
    },
    "Benchmark 100k": {
        "path": BENCHMARK_100K_DATA_PATH,
        "suffix": "100000",
    },
}


def get_index_path(tree_type: str, dataset_label: str) -> Path:
    suffix = DATASETS[dataset_label]["suffix"]
    return PROJECT_ROOT / "data" / f"index_{tree_type}_{suffix}.txt"

TREE_LABELS = {
    "avl": "AVL Tree",
    "rb": "Red-Black Tree",
    "btree": "B-tree",
}

TREE_DESCRIPTIONS = {
    "avl": "строгий баланс по высоте",
    "rb": "балансировка через цвета",
    "btree": "несколько ключей в одном узле",
}


# =============================================================================
# CLI helpers
# =============================================================================

def run_command(args: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        args,
        cwd=PROJECT_ROOT,
        text=True,
        encoding="utf-8",
        errors="replace",
        capture_output=True,
        shell=False,
    )


def format_command(args: list[str]) -> str:
    return " ".join(f'"{arg}"' if " " in arg else arg for arg in args)


def build_app() -> subprocess.CompletedProcess[str]:
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


def build_index(
        tree_type: str,
        data_path: Path,
        index_path: Path,
) -> subprocess.CompletedProcess[str]:
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
    args = [
        str(APP_EXE),
        "search",
        f"--type={tree_type}",
        f"--index={index_path.relative_to(PROJECT_ROOT)}",
        "--json",
    ]

    if fuzzy:
        args.extend(["--fuzzy", f"--max-dist={max_dist}"])

    args.append(query)

    result = run_command(args)
    return result, args


def parse_search_json(stdout: str) -> dict:
    return json.loads(stdout)


# =============================================================================
# Streamlit styling
# =============================================================================

st.set_page_config(
    page_title="Search Index Engine",
    page_icon="🟢",
    layout="wide",
)

st.markdown(
    """
<style>
:root {
    --bg: #050509;
    --panel: rgba(18, 18, 28, 0.92);
    --panel-2: rgba(28, 22, 45, 0.88);
    --violet: #8b5cf6;
    --violet-2: #a855f7;
    --lime: #c6ff00;
    --green: #39ff14;
    --muted: #9ca3af;
    --white: #f8fafc;
    --red: #ff2d55;
    --line: rgba(198, 255, 0, 0.25);
}

.stApp {
    background:
        radial-gradient(circle at 10% 10%, rgba(139, 92, 246, 0.28), transparent 28%),
        radial-gradient(circle at 90% 15%, rgba(198, 255, 0, 0.14), transparent 24%),
        radial-gradient(circle at 50% 90%, rgba(168, 85, 247, 0.18), transparent 30%),
        linear-gradient(135deg, #050509 0%, #090914 45%, #050509 100%);
    color: var(--white);
}

.block-container {
    padding-top: 2rem;
    padding-bottom: 3rem;
    max-width: 1280px;
}

[data-testid="stSidebar"] {
    background:
        linear-gradient(180deg, rgba(10, 10, 18, 0.98), rgba(22, 16, 36, 0.98));
    border-right: 1px solid rgba(198, 255, 0, 0.16);
}

h1, h2, h3 {
    letter-spacing: -0.03em;
}

.hero {
    padding: 2rem 2rem 1.6rem 2rem;
    border-radius: 28px;
    background:
        linear-gradient(135deg, rgba(139, 92, 246, 0.35), rgba(10, 10, 18, 0.78) 42%, rgba(198, 255, 0, 0.10)),
        linear-gradient(180deg, rgba(255,255,255,0.05), rgba(255,255,255,0.01));
    border: 1px solid rgba(198, 255, 0, 0.20);
    box-shadow: 0 0 40px rgba(139, 92, 246, 0.18);
    margin-bottom: 1.4rem;
}

.hero-title {
    font-size: 3.2rem;
    line-height: 0.95;
    font-weight: 850;
    margin: 0;
}

.hero-subtitle {
    color: var(--muted);
    font-size: 1.05rem;
    margin-top: 0.9rem;
}

.badge-row {
    display: flex;
    gap: 0.55rem;
    flex-wrap: wrap;
    margin-top: 1.1rem;
}

.badge {
    padding: 0.35rem 0.7rem;
    border-radius: 999px;
    border: 1px solid rgba(198, 255, 0, 0.25);
    background: rgba(198, 255, 0, 0.08);
    color: #eaff8f;
    font-size: 0.82rem;
    font-weight: 700;
}

.panel {
    padding: 1.15rem 1.25rem;
    border-radius: 24px;
    background: var(--panel);
    border: 1px solid rgba(255, 255, 255, 0.08);
    box-shadow: 0 18px 45px rgba(0, 0, 0, 0.24);
    margin-bottom: 1rem;
}

.panel-accent {
    border: 1px solid rgba(198, 255, 0, 0.25);
    box-shadow: 0 0 25px rgba(198, 255, 0, 0.08);
}

.metric-card {
    padding: 1rem 1.1rem;
    border-radius: 22px;
    background:
        linear-gradient(135deg, rgba(139, 92, 246, 0.18), rgba(198, 255, 0, 0.07));
    border: 1px solid rgba(255,255,255,0.08);
}

.metric-label {
    color: var(--muted);
    font-size: 0.85rem;
    margin-bottom: 0.25rem;
}

.metric-value {
    color: var(--white);
    font-size: 1.8rem;
    font-weight: 800;
}

.result-card {
    padding: 1rem 1.2rem;
    border-radius: 22px;
    background:
        linear-gradient(135deg, rgba(18, 18, 28, 0.96), rgba(32, 26, 52, 0.90));
    border: 1px solid rgba(198, 255, 0, 0.20);
    margin-bottom: 0.75rem;
}

.result-title {
    color: var(--white);
    font-size: 1.05rem;
    font-weight: 750;
    margin-bottom: 0.35rem;
}

.result-meta {
    color: var(--muted);
    font-size: 0.88rem;
}

.toxic {
    color: var(--lime);
    font-weight: 800;
}

.violet {
    color: var(--violet-2);
    font-weight: 800;
}

.small-muted {
    color: var(--muted);
    font-size: 0.9rem;
}

/* Streamlit widgets */
.stButton > button {
    border-radius: 18px;
    border: 1px solid rgba(198, 255, 0, 0.35);
    background: linear-gradient(135deg, #8b5cf6, #a855f7 45%, #c6ff00);
    color: #050509;
    font-weight: 850;
    padding: 0.7rem 1rem;
    box-shadow: 0 0 22px rgba(139, 92, 246, 0.28);
}

.stButton > button:hover {
    border: 1px solid rgba(198, 255, 0, 0.75);
    box-shadow: 0 0 34px rgba(198, 255, 0, 0.20);
    transform: translateY(-1px);
}

.stTextInput > div > div > input {
    border-radius: 16px;
}

.stSelectbox div[data-baseweb="select"] > div {
    border-radius: 16px;
}

.stCodeBlock {
    border-radius: 18px;
}
</style>
""",
    unsafe_allow_html=True,
)


# =============================================================================
# Header
# =============================================================================

st.markdown(
    """
<div class="hero">
    <div class="hero-title">
        Search Index <span class="toxic">Engine</span>
    </div>
    <div class="hero-subtitle">
        AVL · Red-Black Tree · B-tree · Exact Search · Fuzzy Search
    </div>
    <div class="badge-row">
        <div class="badge">C core</div>
        <div class="badge">Streamlit demo</div>
        <div class="badge">Levenshtein</div>
        <div class="badge">term → posting list</div>
    </div>
</div>
""",
    unsafe_allow_html=True,
)


# =============================================================================
# Sidebar
# =============================================================================

with st.sidebar:
    st.markdown("## Управление")

    tree_type = st.selectbox(
        "Структура данных",
        options=["avl", "rb", "btree"],
        format_func=lambda x: TREE_LABELS[x],
    )

    st.caption(TREE_DESCRIPTIONS[tree_type])

    st.divider()

    available_dataset_labels = list(DATASETS.keys())

    selected_dataset_label = st.selectbox(
        "Датасет",
        options=available_dataset_labels,
        index=1 if DEMO_DATA_PATH.exists() else 0,
    )

    selected_data_path = DATASETS[selected_dataset_label]["path"]

    custom_data_text = st.text_input(
        "Путь к данным",
        value=str(selected_data_path.relative_to(PROJECT_ROOT)),
    )

    data_path = PROJECT_ROOT / custom_data_text
    index_path = get_index_path(tree_type, selected_dataset_label)

    st.caption(f"Индекс: {index_path.relative_to(PROJECT_ROOT)}")

    st.divider()

    st.markdown("### Режим поиска")

    fuzzy = st.toggle("Fuzzy search", value=True)

    max_dist = st.slider(
        "max-dist",
        min_value=0,
        max_value=3,
        value=2,
        disabled=not fuzzy,
    )

    st.divider()

    st.markdown("### Сервис")

    build_clicked = st.button("Собрать app.exe", use_container_width=True)
    index_clicked = st.button("Построить индекс", use_container_width=True)


# =============================================================================
# Build / index actions
# =============================================================================

if build_clicked:
    with st.spinner("Собираю C-приложение..."):
        result = build_app()

    if result.returncode == 0:
        st.success("app.exe собран")
    else:
        st.error("Ошибка сборки")
        st.code(result.stderr or result.stdout, language="text")

if index_clicked:
    if not APP_EXE.exists():
        st.error("app.exe не найден. Сначала собери C-приложение.")
    elif not data_path.exists():
        st.error(f"Файл данных не найден: {data_path}")
    else:
        with st.spinner(f"Строю индекс: {TREE_LABELS[tree_type]}..."):
            result = build_index(tree_type, data_path, index_path)

        if result.returncode == 0:
            st.success(f"Индекс построен: {index_path.relative_to(PROJECT_ROOT)}")
            with st.expander("Лог индексации"):
                st.code(result.stdout, language="text")
        else:
            st.error("Ошибка индексации")
            st.code(result.stderr or result.stdout, language="text")


# =============================================================================
# Main search UI
# =============================================================================

left, right = st.columns([2.2, 1])

with left:
    st.markdown('<div class="panel panel-accent">', unsafe_allow_html=True)

    default_query = "pyton list" if fuzzy else "python list"

    query = st.text_input(
        "Поисковый запрос",
        value=default_query,
        placeholder="Например: python list или pyton list",
    )

    search_clicked = st.button("Искать", type="primary", use_container_width=True)

    st.markdown("</div>", unsafe_allow_html=True)

with right:
    st.markdown(
        f"""
<div class="panel">
    <div class="small-muted">Текущая конфигурация</div>
    <br>
    <b>Tree:</b> <span class="toxic">{TREE_LABELS[tree_type]}</span><br>
    <b>Mode:</b> <span class="violet">{"Fuzzy" if fuzzy else "Exact"}</span><br>
    <b>max-dist:</b> {max_dist if fuzzy else "—"}<br>
    <b>Dataset:</b> {selected_dataset_label}<br>
</div>
""",
        unsafe_allow_html=True,
    )


# =============================================================================
# Search action
# =============================================================================

if search_clicked:
    if not APP_EXE.exists():
        st.error("app.exe не найден. Нажми «Собрать app.exe».")
        st.stop()

    if not index_path.exists():
        st.error("Файл индекса не найден. Нажми «Построить индекс».")
        st.stop()

    if not query.strip():
        st.error("Введите поисковый запрос.")
        st.stop()

    with st.spinner("Ищу..."):
        result, command = run_search(
            tree_type=tree_type,
            index_path=index_path,
            query=query.strip(),
            fuzzy=fuzzy,
            max_dist=max_dist,
        )

    st.markdown("### Команда")
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

    c1, c2, c3 = st.columns(3)

    with c1:
        st.markdown(
            f"""
<div class="metric-card">
    <div class="metric-label">Найдено документов</div>
    <div class="metric-value">{total}</div>
</div>
""",
            unsafe_allow_html=True,
        )

    with c2:
        st.markdown(
            f"""
<div class="metric-card">
    <div class="metric-label">Время поиска, ms</div>
    <div class="metric-value">{time_ms:.3f}</div>
</div>
""",
            unsafe_allow_html=True,
        )

    with c3:
        st.markdown(
            f"""
<div class="metric-card">
    <div class="metric-label">Режим</div>
    <div class="metric-value">{"Fuzzy" if fuzzy else "Exact"}</div>
</div>
""",
            unsafe_allow_html=True,
        )

    st.markdown("### Результаты")

    if not results:
        st.info("Ничего не найдено.")
    else:
        for i, item in enumerate(results, start=1):
            title = item.get("title", "—")
            doc_id = item.get("doc_id", "—")
            score = item.get("score", "—")

            st.markdown(
                f"""
<div class="result-card">
    <div class="result-title">{i}. {title}</div>
    <div class="result-meta">
        doc_id = <span class="toxic">{doc_id}</span>
        &nbsp; · &nbsp;
        score = <span class="violet">{score}</span>
    </div>
</div>
""",
                unsafe_allow_html=True,
            )

    with st.expander("Raw JSON"):
        st.code(result.stdout, language="json")


# =============================================================================
# Examples
# =============================================================================

st.markdown("### Быстрые сценарии для защиты")

ex1, ex2, ex3 = st.columns(3)

with ex1:
    st.markdown(
        """
<div class="panel">
<b>Exact search</b><br><br>
<span class="small-muted">
python<br>
python list<br>
red black tree<br>
database index
</span>
</div>
""",
        unsafe_allow_html=True,
    )

with ex2:
    st.markdown(
        """
<div class="panel">
<b>Fuzzy search</b><br><br>
<span class="small-muted">
pyton<br>
pyton list<br>
balnced tree<br>
databse index
</span>
</div>
""",
        unsafe_allow_html=True,
    )

with ex3:
    st.markdown(
        """
<div class="panel">
<b>Проверка max-dist</b><br><br>
<span class="small-muted">
max-dist=0: pyton → пусто<br>
max-dist=1: pyton → python<br>
max-dist=2: pyton lisst → python list
</span>
</div>
""",
        unsafe_allow_html=True,
    )

# =============================================================================
# Educational Tree Visualizer
# =============================================================================

def _svg_tree(nodes: list[dict], edges: list[tuple[str, str]], note: str) -> str:
    """
    Рисует учебное бинарное дерево в SVG.

    nodes:
        [
            {
                "id": "engine",
                "label": "engine",
                "x": 500,
                "y": 90,
                "color": "#8b5cf6"
            }
        ]

    edges:
        [("engine", "architecture")]
    """
    node_by_id = {node["id"]: node for node in nodes}

    edge_svg = ""
    for parent_id, child_id in edges:
        parent = node_by_id[parent_id]
        child = node_by_id[child_id]

        edge_svg += f"""
        <line
            x1="{parent["x"]}" y1="{parent["y"] + 30}"
            x2="{child["x"]}" y2="{child["y"] - 30}"
            stroke="rgba(198,255,0,0.42)"
            stroke-width="3"
        />
        """

    node_svg = ""
    for node in nodes:
        color = node.get("color", "#8b5cf6")
        label = node["label"]

        node_svg += f"""
        <g>
            <circle
                cx="{node["x"]}" cy="{node["y"]}" r="44"
                fill="{color}"
                stroke="rgba(255,255,255,0.75)"
                stroke-width="2"
                filter="url(#glow)"
            />
            <text
                x="{node["x"]}" y="{node["y"] + 5}"
                text-anchor="middle"
                font-size="17"
                font-family="Inter, Arial"
                font-weight="800"
                fill="#050509"
            >
                {label}
            </text>
        </g>
        """

    return f"""
    <div class="panel panel-accent">
        <svg viewBox="0 0 1000 420" width="100%" height="420">
            <defs>
                <filter id="glow" x="-50%" y="-50%" width="200%" height="200%">
                    <feGaussianBlur stdDeviation="5" result="coloredBlur"/>
                    <feMerge>
                        <feMergeNode in="coloredBlur"/>
                        <feMergeNode in="SourceGraphic"/>
                    </feMerge>
                </filter>
            </defs>

            <rect
                x="0" y="0"
                width="1000" height="420"
                rx="26"
                fill="rgba(5,5,9,0.55)"
            />

            {edge_svg}
            {node_svg}

            <text
                x="500" y="385"
                text-anchor="middle"
                font-size="18"
                font-family="Inter, Arial"
                font-weight="700"
                fill="#c6ff00"
            >
                {note}
            </text>
        </svg>
    </div>
    """


def _svg_btree(boxes: list[dict], edges: list[tuple[str, str]], note: str) -> str:
    """
    Рисует учебное B-tree в SVG.

    boxes:
        [
            {
                "id": "root",
                "keys": ["frontend"],
                "x": 500,
                "y": 90,
                "w": 240
            }
        ]
    """
    box_by_id = {box["id"]: box for box in boxes}

    edge_svg = ""
    for parent_id, child_id in edges:
        parent = box_by_id[parent_id]
        child = box_by_id[child_id]

        edge_svg += f"""
        <line
            x1="{parent["x"]}" y1="{parent["y"] + 34}"
            x2="{child["x"]}" y2="{child["y"] - 34}"
            stroke="rgba(198,255,0,0.42)"
            stroke-width="3"
        />
        """

    box_svg = ""
    for box in boxes:
        keys = box["keys"]
        key_text = "  |  ".join(keys)
        width = box.get("w", 320)
        x = box["x"] - width / 2
        y = box["y"] - 34

        box_svg += f"""
        <g filter="url(#glow)">
            <rect
                x="{x}" y="{y}"
                width="{width}" height="68"
                rx="18"
                fill="rgba(139,92,246,0.95)"
                stroke="rgba(255,255,255,0.72)"
                stroke-width="2"
            />
            <text
                x="{box["x"]}" y="{box["y"] + 6}"
                text-anchor="middle"
                font-size="18"
                font-family="Inter, Arial"
                font-weight="850"
                fill="#050509"
            >
                [{key_text}]
            </text>
        </g>
        """

    return f"""
    <div class="panel panel-accent">
        <svg viewBox="0 0 1000 420" width="100%" height="420">
            <defs>
                <filter id="glow" x="-50%" y="-50%" width="200%" height="200%">
                    <feGaussianBlur stdDeviation="5" result="coloredBlur"/>
                    <feMerge>
                        <feMergeNode in="coloredBlur"/>
                        <feMergeNode in="SourceGraphic"/>
                    </feMerge>
                </filter>
            </defs>

            <rect
                x="0" y="0"
                width="1000" height="420"
                rx="26"
                fill="rgba(5,5,9,0.55)"
            />

            {edge_svg}
            {box_svg}

            <text
                x="500" y="385"
                text-anchor="middle"
                font-size="18"
                font-family="Inter, Arial"
                font-weight="700"
                fill="#c6ff00"
            >
                {note}
            </text>
        </svg>
    </div>
    """

def render_svg(svg_html: str, height: int = 470) -> None:
    components.html(
        f"""
<!doctype html>
<html>
<head>
<style>
    body {{
        margin: 0;
        padding: 0;
        background: transparent;
        font-family: Inter, Arial, sans-serif;
    }}

    .panel {{
        padding: 0.25rem;
        border-radius: 24px;
        background: rgba(18, 18, 28, 0.92);
        border: 1px solid rgba(255, 255, 255, 0.08);
        box-shadow: 0 18px 45px rgba(0, 0, 0, 0.24);
    }}

    .panel-accent {{
        border: 1px solid rgba(198, 255, 0, 0.25);
        box-shadow: 0 0 25px rgba(198, 255, 0, 0.08);
    }}
</style>
</head>
<body>
    {svg_html}
</body>
</html>
""",
        height=height,
        scrolling=False,
    )

AVL_DEMO = {
    1: {
        "note": 'insert "search": первый узел становится корнем',
        "nodes": [
            {
                "id": "search",
                "label": "search",
                "x": 500,
                "y": 110,
                "color": "#c6ff00",
            },
        ],
        "edges": [],
    },
    2: {
        "note": 'insert "engine": engine меньше search, уходит влево',
        "nodes": [
            {
                "id": "search",
                "label": "search",
                "x": 500,
                "y": 90,
                "color": "#8b5cf6",
            },
            {
                "id": "engine",
                "label": "engine",
                "x": 350,
                "y": 220,
                "color": "#c6ff00",
            },
        ],
        "edges": [
            ("search", "engine"),
        ],
    },
    3: {
        "note": 'insert "architecture": возникает LL-дисбаланс, AVL делает правый поворот',
        "nodes": [
            {
                "id": "engine",
                "label": "engine",
                "x": 500,
                "y": 90,
                "color": "#c6ff00",
            },
            {
                "id": "architecture",
                "label": "arch",
                "x": 330,
                "y": 235,
                "color": "#8b5cf6",
            },
            {
                "id": "search",
                "label": "search",
                "x": 670,
                "y": 235,
                "color": "#8b5cf6",
            },
        ],
        "edges": [
            ("engine", "architecture"),
            ("engine", "search"),
        ],
    },
    4: {
        "note": 'insert "index": уходит в правое поддерево, баланс ещё нормальный',
        "nodes": [
            {
                "id": "engine",
                "label": "engine",
                "x": 500,
                "y": 80,
                "color": "#c6ff00",
            },
            {
                "id": "architecture",
                "label": "arch",
                "x": 320,
                "y": 215,
                "color": "#8b5cf6",
            },
            {
                "id": "search",
                "label": "search",
                "x": 680,
                "y": 215,
                "color": "#8b5cf6",
            },
            {
                "id": "index",
                "label": "index",
                "x": 585,
                "y": 335,
                "color": "#a855f7",
            },
        ],
        "edges": [
            ("engine", "architecture"),
            ("engine", "search"),
            ("search", "index"),
        ],
    },
    5: {
        "note": 'insert "frontend": правое поддерево перестраивается, чтобы высота оставалась логарифмической',
        "nodes": [
            {
                "id": "engine",
                "label": "engine",
                "x": 500,
                "y": 75,
                "color": "#c6ff00",
            },
            {
                "id": "architecture",
                "label": "arch",
                "x": 300,
                "y": 220,
                "color": "#8b5cf6",
            },
            {
                "id": "index",
                "label": "index",
                "x": 700,
                "y": 220,
                "color": "#c6ff00",
            },
            {
                "id": "frontend",
                "label": "front",
                "x": 590,
                "y": 340,
                "color": "#a855f7",
            },
            {
                "id": "search",
                "label": "search",
                "x": 810,
                "y": 340,
                "color": "#a855f7",
            },
        ],
        "edges": [
            ("engine", "architecture"),
            ("engine", "index"),
            ("index", "frontend"),
            ("index", "search"),
        ],
    },
    6: {
        "note": 'insert "backend": дерево остаётся сбалансированным, высота контролируется',
        "nodes": [
            {
                "id": "engine",
                "label": "engine",
                "x": 500,
                "y": 70,
                "color": "#c6ff00",
            },
            {
                "id": "architecture",
                "label": "arch",
                "x": 285,
                "y": 210,
                "color": "#8b5cf6",
            },
            {
                "id": "index",
                "label": "index",
                "x": 715,
                "y": 210,
                "color": "#c6ff00",
            },
            {
                "id": "backend",
                "label": "back",
                "x": 385,
                "y": 335,
                "color": "#a855f7",
            },
            {
                "id": "frontend",
                "label": "front",
                "x": 610,
                "y": 335,
                "color": "#a855f7",
            },
            {
                "id": "search",
                "label": "search",
                "x": 820,
                "y": 335,
                "color": "#a855f7",
            },
        ],
        "edges": [
            ("engine", "architecture"),
            ("architecture", "backend"),
            ("engine", "index"),
            ("index", "frontend"),
            ("index", "search"),
        ],
    },
}


RBTREE_DEMO = {
    1: {
        "note": 'insert "search": корень всегда чёрный',
        "nodes": [
            {
                "id": "search",
                "label": "search",
                "x": 500,
                "y": 110,
                "color": "#0f172a",
            },
        ],
        "edges": [],
    },
    2: {
        "note": 'insert "engine": новый узел сначала красный',
        "nodes": [
            {
                "id": "search",
                "label": "search",
                "x": 500,
                "y": 90,
                "color": "#0f172a",
            },
            {
                "id": "engine",
                "label": "engine",
                "x": 350,
                "y": 220,
                "color": "#ff2d55",
            },
        ],
        "edges": [
            ("search", "engine"),
        ],
    },
    3: {
        "note": 'insert "architecture": красный под красным запрещён, делаем rotation/recolor',
        "nodes": [
            {
                "id": "engine",
                "label": "engine",
                "x": 500,
                "y": 90,
                "color": "#0f172a",
            },
            {
                "id": "architecture",
                "label": "arch",
                "x": 330,
                "y": 235,
                "color": "#ff2d55",
            },
            {
                "id": "search",
                "label": "search",
                "x": 670,
                "y": 235,
                "color": "#ff2d55",
            },
        ],
        "edges": [
            ("engine", "architecture"),
            ("engine", "search"),
        ],
    },
    4: {
        "note": 'insert "index": появляется красный узел, затем возможна перекраска',
        "nodes": [
            {
                "id": "engine",
                "label": "engine",
                "x": 500,
                "y": 80,
                "color": "#0f172a",
            },
            {
                "id": "architecture",
                "label": "arch",
                "x": 320,
                "y": 215,
                "color": "#0f172a",
            },
            {
                "id": "search",
                "label": "search",
                "x": 680,
                "y": 215,
                "color": "#0f172a",
            },
            {
                "id": "index",
                "label": "index",
                "x": 585,
                "y": 335,
                "color": "#ff2d55",
            },
        ],
        "edges": [
            ("engine", "architecture"),
            ("engine", "search"),
            ("search", "index"),
        ],
    },
    5: {
        "note": 'insert "frontend": снова красный конфликт, RBTree исправляет цветами и поворотом',
        "nodes": [
            {
                "id": "engine",
                "label": "engine",
                "x": 500,
                "y": 75,
                "color": "#0f172a",
            },
            {
                "id": "architecture",
                "label": "arch",
                "x": 300,
                "y": 220,
                "color": "#0f172a",
            },
            {
                "id": "index",
                "label": "index",
                "x": 700,
                "y": 220,
                "color": "#0f172a",
            },
            {
                "id": "frontend",
                "label": "front",
                "x": 590,
                "y": 340,
                "color": "#ff2d55",
            },
            {
                "id": "search",
                "label": "search",
                "x": 810,
                "y": 340,
                "color": "#ff2d55",
            },
        ],
        "edges": [
            ("engine", "architecture"),
            ("engine", "index"),
            ("index", "frontend"),
            ("index", "search"),
        ],
    },
    6: {
        "note": 'insert "backend": родитель чёрный, конфликтов цвета нет',
        "nodes": [
            {
                "id": "engine",
                "label": "engine",
                "x": 500,
                "y": 70,
                "color": "#0f172a",
            },
            {
                "id": "architecture",
                "label": "arch",
                "x": 285,
                "y": 210,
                "color": "#0f172a",
            },
            {
                "id": "index",
                "label": "index",
                "x": 715,
                "y": 210,
                "color": "#0f172a",
            },
            {
                "id": "backend",
                "label": "back",
                "x": 385,
                "y": 335,
                "color": "#ff2d55",
            },
            {
                "id": "frontend",
                "label": "front",
                "x": 610,
                "y": 335,
                "color": "#ff2d55",
            },
            {
                "id": "search",
                "label": "search",
                "x": 820,
                "y": 335,
                "color": "#ff2d55",
            },
        ],
        "edges": [
            ("engine", "architecture"),
            ("architecture", "backend"),
            ("engine", "index"),
            ("index", "frontend"),
            ("index", "search"),
        ],
    },
}


BTREE_DEMO = {
    1: {
        "note": 'insert "search": первый ключ попадает в корневой узел',
        "boxes": [
            {
                "id": "root",
                "keys": ["search"],
                "x": 500,
                "y": 120,
                "w": 260,
            },
        ],
        "edges": [],
    },
    2: {
        "note": 'insert "engine": ключи внутри узла хранятся отсортированно',
        "boxes": [
            {
                "id": "root",
                "keys": ["engine", "search"],
                "x": 500,
                "y": 120,
                "w": 360,
            },
        ],
        "edges": [],
    },
    3: {
        "note": 'insert "architecture": один узел может хранить несколько ключей',
        "boxes": [
            {
                "id": "root",
                "keys": ["architecture", "engine", "search"],
                "x": 500,
                "y": 120,
                "w": 500,
            },
        ],
        "edges": [],
    },
    4: {
        "note": 'insert "index": B-tree всё ещё держит ключи в одном узле',
        "boxes": [
            {
                "id": "root",
                "keys": ["architecture", "engine", "index", "search"],
                "x": 500,
                "y": 120,
                "w": 620,
            },
        ],
        "edges": [],
    },
    5: {
        "note": 'insert "frontend": при BTREE_T=3 в узле может быть до 5 ключей',
        "boxes": [
            {
                "id": "root",
                "keys": ["architecture", "engine", "frontend", "index", "search"],
                "x": 500,
                "y": 120,
                "w": 760,
            },
        ],
        "edges": [],
    },
    6: {
        "note": 'insert "backend": полный корень делится, средний ключ поднимается наверх',
        "boxes": [
            {
                "id": "root",
                "keys": ["frontend"],
                "x": 500,
                "y": 95,
                "w": 240,
            },
            {
                "id": "left",
                "keys": ["architecture", "backend", "engine"],
                "x": 315,
                "y": 255,
                "w": 460,
            },
            {
                "id": "right",
                "keys": ["index", "search"],
                "x": 715,
                "y": 255,
                "w": 330,
            },
        ],
        "edges": [
            ("root", "left"),
            ("root", "right"),
        ],
    },
}


st.markdown("## Tree Visualizer")

st.caption(
    "Учебная визуализация вставки. Это не live-dump из C-памяти, "
    "а отдельная демонстрация принципа балансировки на маленьком наборе ключей."
)

viz_col_1, viz_col_2 = st.columns([1, 1])

with viz_col_1:
    viz_tree = st.selectbox(
        "Что визуализируем",
        options=["avl", "rb", "btree"],
        format_func=lambda x: {
            "avl": "AVL Tree",
            "rb": "Red-Black Tree",
            "btree": "B-tree",
        }[x],
        key="tree_visualizer_select",
    )

with viz_col_2:
    viz_step = st.slider(
        "Шаг вставки",
        min_value=1,
        max_value=6,
        value=3,
        key="tree_visualizer_step",
    )

st.markdown(
    """
<div class="panel">
    <b>Последовательность вставки:</b>
    <span class="toxic">search</span> →
    <span class="toxic">engine</span> →
    <span class="toxic">architecture</span> →
    <span class="toxic">index</span> →
    <span class="toxic">frontend</span> →
    <span class="toxic">backend</span>
</div>
""",
    unsafe_allow_html=True,
)

if viz_tree == "avl":
    state = AVL_DEMO[viz_step]
    render_svg(
        _svg_tree(
            nodes=state["nodes"],
            edges=state["edges"],
            note=state["note"],
        )
    )

elif viz_tree == "rb":
    state = RBTREE_DEMO[viz_step]
    render_svg(
        _svg_tree(
            nodes=state["nodes"],
            edges=state["edges"],
            note=state["note"],
        )
    )

else:
    state = BTREE_DEMO[viz_step]
    render_svg(
        _svg_btree(
            boxes=state["boxes"],
            edges=state["edges"],
            note=state["note"],
        )
    )


