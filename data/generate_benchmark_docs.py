from pathlib import Path
import argparse, json, random

TOPICS = {
    "python": ["python","list","dict","sort","comprehension","exception","generator","iterator","package","virtualenv"],
    "trees": ["tree","avl","red","black","btree","rotation","height","balance","node","split"],
    "search": ["search","index","query","token","posting","document","rank","score","frontend","engine"],
    "database": ["database","index","query","storage","transaction","table","schema","join","cache","optimizer"],
    "systems": ["memory","pointer","cache","thread","process","kernel","buffer","network","socket","latency"],
    "algorithms": ["algorithm","complexity","asymptotic","amortized","heap","queue","stack","graph","bfs","dfs"],
    "text": ["text","tokenization","levenshtein","fuzzy","exact","typo","distance","string","matching","pattern"],
}
COMMON = ["tutorial","guide","example","problem","solution","implementation","performance","benchmark","analysis","overview"]
SPECIAL = [
    (1, "How to sort a list in Python", ["python","list","sort","algorithm","example"]),
    (2, "Python dictionary key error", ["python","dictionary","dict","key","error","exception"]),
    (3, "Memory leak in C program", ["memory","leak","program","c","pointer"]),
    (4, "Balanced binary tree rotations", ["balanced","binary","tree","rotation","height"]),
    (5, "Red black tree insertion fixup", ["red","black","tree","insertion","fixup","color"]),
    (6, "B-tree node split example", ["btree","tree","node","split","keys","children"]),
    (7, "Database index basics", ["database","index","search","storage","query"]),
    (8, "Inverted index for text search", ["inverted","index","text","search","document"]),
    (9, "Fuzzy search with Levenshtein distance", ["fuzzy","search","levenshtein","distance","typo"]),
    (10, "Exact search with token intersection", ["exact","search","token","intersection","query"]),
]

def generate(path, n, seed=14):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    rng = random.Random(seed + n)
    names = list(TOPICS)
    with path.open("w", encoding="utf-8", newline="\n") as f:
        for doc_id, title, tokens in SPECIAL:
            if doc_id <= n:
                f.write(json.dumps({"doc_id": doc_id, "title": title, "tokens": tokens}, separators=(",",":")) + "\n")
        for doc_id in range(len(SPECIAL)+1, n+1):
            topic = rng.choice(names)
            other = rng.choice(names)
            tokens = []
            tokens += rng.sample(TOPICS[topic], 4)
            tokens += rng.sample(TOPICS[other], 2)
            tokens += rng.sample(COMMON, 2)
            tokens += [f"doc{doc_id}", f"term{doc_id:06d}", f"{topic}{doc_id % 997:03d}"]
            if doc_id % 17 == 0:
                tokens += ["python", "list"]
            if doc_id % 23 == 0:
                tokens += ["red", "black", "tree"]
            if doc_id % 29 == 0:
                tokens += ["database", "index"]
            if doc_id % 31 == 0:
                tokens += ["fuzzy", "search"]
            if doc_id % 37 == 0:
                tokens += ["algorithm", "complexity"]
            seen = set()
            tokens = [x for x in tokens if not (x in seen or seen.add(x))]
            title = f"{topic.title()} notes {doc_id}: {' '.join(tokens[:4])}"
            f.write(json.dumps({"doc_id": doc_id, "title": title, "tokens": tokens}, separators=(",",":")) + "\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--n", type=int, default=100000)
    parser.add_argument("--out", default="data/sample/benchmark_docs_100000.jsonl")
    parser.add_argument("--seed", type=int, default=14)
    args = parser.parse_args()
    generate(args.out, args.n, args.seed)
    print(f"generated: {args.out}, docs={args.n}")
