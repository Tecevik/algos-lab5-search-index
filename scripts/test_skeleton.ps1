Write-Host "Running skeleton checks..."

.\test_avl.exe
if ($LASTEXITCODE -ne 0) {
    Write-Host "AVL tests failed"
    exit $LASTEXITCODE
}

.\test_rb.exe
if ($LASTEXITCODE -ne 0) {
    Write-Host "RB tests failed"
    exit $LASTEXITCODE
}

.\test_btree.exe
if ($LASTEXITCODE -ne 0) {
    Write-Host "B-tree tests failed"
    exit $LASTEXITCODE
}

.\app.exe index --type=avl --data=data/sample/docs.jsonl --index=data/index_avl.txt
if ($LASTEXITCODE -ne 0) {
    Write-Host "AVL index command failed"
    exit $LASTEXITCODE
}

.\app.exe search --type=avl --index=data/index_avl.txt --json "python list"
if ($LASTEXITCODE -ne 0) {
    Write-Host "AVL search command failed"
    exit $LASTEXITCODE
}

.\app.exe index --type=rb --data=data/sample/docs.jsonl --index=data/index_rb.txt
if ($LASTEXITCODE -ne 0) {
    Write-Host "RB index command failed"
    exit $LASTEXITCODE
}

.\app.exe search --type=rb --index=data/index_rb.txt --json "python list"
if ($LASTEXITCODE -ne 0) {
    Write-Host "RB search command failed"
    exit $LASTEXITCODE
}

.\app.exe index --type=btree --data=data/sample/docs.jsonl --index=data/index_btree.txt
if ($LASTEXITCODE -ne 0) {
    Write-Host "B-tree index command failed"
    exit $LASTEXITCODE
}

.\app.exe search --type=btree --index=data/index_btree.txt --json "python list"
if ($LASTEXITCODE -ne 0) {
    Write-Host "B-tree search command failed"
    exit $LASTEXITCODE
}

Write-Host "Skeleton checks passed"