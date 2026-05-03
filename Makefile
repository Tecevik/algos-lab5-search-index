# Makefile для сборки лабораторной.
#
# Основные команды:
#
#   make        - собрать приложение и unit-тесты
#   make app    - собрать только CLI-приложение ./app
#   make u_tests - собрать и запустить unit-тесты деревьев
#   make test   - запустить минимальный end-to-end тест
#   make clean  - удалить артефакты сборки
#
# Важно:
# В Makefile команды под target'ами должны начинаться с TAB, а не с пробелов.

CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g

OBJ_SHARED = common/vector.o posting.o avl/avl.o rbtree/rbtree.o btree/btree.o \
             index/index.o index/search.o

.PHONY: all app u_tests test clean

all: app u_tests

app: $(OBJ_SHARED) main.o
	$(CC) $(CFLAGS) -o app $(OBJ_SHARED) main.o

test_avl: common/vector.o posting.o avl/avl.o avl/tests.o
	$(CC) $(CFLAGS) -o test_avl common/vector.o posting.o avl/avl.o avl/tests.o

test_rb: common/vector.o posting.o rbtree/rbtree.o rbtree/tests.o
	$(CC) $(CFLAGS) -o test_rb common/vector.o posting.o rbtree/rbtree.o rbtree/tests.o

test_btree: common/vector.o posting.o btree/btree.o btree/tests.o
	$(CC) $(CFLAGS) -o test_btree common/vector.o posting.o btree/btree.o btree/tests.o

u_tests: test_avl test_rb test_btree
	./test_avl
	./test_rb
	./test_btree

test: app
	@echo "=== E2E skeleton test ==="
	./app index --type=avl --data=data/sample/docs.jsonl --index=data/index_avl.txt
	./app search --type=avl --index=data/index_avl.txt --json "python list"
	@echo "=== E2E skeleton OK ==="

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f app test_avl test_rb test_btree
	rm -f *.o common/*.o avl/*.o rbtree/*.o btree/*.o index/*.o
	rm -f data/index_*.txt