# Benchmark datasets for Lab 14

Files:
- benchmark_docs_50000.jsonl
- benchmark_docs_100000.jsonl
- generate_benchmark_docs.py

Put files into:
data/sample/

Generate locally:
```powershell
python data/sample/generate_benchmark_docs.py --n 50000 --out data/sample/benchmark_docs_50000.jsonl
python data/sample/generate_benchmark_docs.py --n 100000 --out data/sample/benchmark_docs_100000.jsonl
```

Index examples:
```powershell
.\app.exe index --type=avl --data=data/sample/benchmark_docs_50000.jsonl --index=data/index_avl_50000.txt
.\app.exe index --type=rb --data=data/sample/benchmark_docs_50000.jsonl --index=data/index_rb_50000.txt
.\app.exe index --type=btree --data=data/sample/benchmark_docs_50000.jsonl --index=data/index_btree_50000.txt
```

Search examples:
```powershell
.\app.exe search --type=avl --index=data/index_avl_50000.txt --json "python list"
.\app.exe search --type=avl --index=data/index_avl_50000.txt --json --fuzzy --max-dist=1 "pyton list"
.\app.exe search --type=avl --index=data/index_avl_50000.txt --json --fuzzy --max-dist=2 "databse index"
```
