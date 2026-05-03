Write-Host "Building app.exe..."

gcc -Wall -Wextra -std=c11 -O2 -g `
  -o app.exe `
  common/vector.c `
  posting.c `
  avl/avl.c `
  rbtree/rbtree.c `
  btree/btree.c `
  index/index.c `
  index/search.c `
  main.c

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build app failed"
    exit $LASTEXITCODE
}

Write-Host "Building test_avl.exe..."

gcc -Wall -Wextra -std=c11 -O2 -g `
  -o test_avl.exe `
  common/vector.c `
  posting.c `
  avl/avl.c `
  avl/tests.c

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build test_avl failed"
    exit $LASTEXITCODE
}

Write-Host "Building test_rb.exe..."

gcc -Wall -Wextra -std=c11 -O2 -g `
  -o test_rb.exe `
  common/vector.c `
  posting.c `
  rbtree/rbtree.c `
  rbtree/tests.c

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build test_rb failed"
    exit $LASTEXITCODE
}

Write-Host "Building test_btree.exe..."

gcc -Wall -Wextra -std=c11 -O2 -g `
  -o test_btree.exe `
  common/vector.c `
  posting.c `
  btree/btree.c `
  btree/tests.c

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build test_btree failed"
    exit $LASTEXITCODE
}

Write-Host "Build finished successfully"