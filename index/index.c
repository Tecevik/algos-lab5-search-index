/*
* Модуль: Index.
 *
 * Ответственный: Егор.
 *
 * Назначение:
 * Index — это единая обёртка над тремя структурами:
 *
 * - AVL
 * - Red-Black Tree
 * - B-tree
 *
 * Остальной код не должен напрямую работать с avl/rbtree/btree.
 * Вместо этого используется общий интерфейс:
 *
 * - createIndex
 * - insertTerm
 * - lookupTerm
 * - traverseIndex
 * - saveIndex
 * - loadIndex
 * - freeIndex
 *
 * Благодаря этому search.c и main.c не зависят от конкретного дерева.
 *
 * Сейчас часть функций может быть временной заглушкой.
 * Особенно это касается saveIndex/loadIndex/runIndex.
 *
 * Что нужно реализовать:
 * 1. Создание нужного дерева по TreeType.
 * 2. Вставку термов через нужный backend.
 * 3. Поиск термов через нужный backend.
 * 4. Обход индекса.
 * 5. Чтение docs.jsonl.
 * 6. Построение индекса.
 * 7. Сохранение индекса в файл.
 * 8. Загрузку индекса из файла.
 *
 * Важно:
 * Этот слой должен скрывать различия между деревьями.
 */

#include "index.h"

#include "../avl/avl.h"
#include "../rbtree/rbtree.h"
#include "../btree/btree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TreeType parseType(const char* s) {
    if (!s) return TREE_AVL;

    if (strcmp(s, "avl") == 0) {
        return TREE_AVL;
    }

    if (strcmp(s, "rb") == 0 || strcmp(s, "rbtree") == 0 || strcmp(s, "redblack") == 0) {
        return TREE_RB;
    }

    if (strcmp(s, "btree") == 0 || strcmp(s, "b-tree") == 0) {
        return TREE_BTREE;
    }

    fprintf(stderr, "Unknown tree type: %s. Fallback to AVL.\n", s);
    return TREE_AVL;
}

const char* typeName(TreeType type) {
    switch (type) {
        case TREE_AVL:
            return "avl";
        case TREE_RB:
            return "rb";
        case TREE_BTREE:
            return "btree";
        default:
            return "unknown";
    }
}

Index* createIndex(TreeType type) {
    Index* idx = malloc(sizeof(Index));
    if (!idx) return NULL;

    idx->type = type;
    idx->tree = NULL;

    switch (type) {
        case TREE_AVL:
            idx->tree = createAVLTree();
            break;
        case TREE_RB:
            idx->tree = createRBTree();
            break;
        case TREE_BTREE:
            idx->tree = createBTree();
            break;
        default:
            free(idx);
            return NULL;
    }

    if (!idx->tree) {
        free(idx);
        return NULL;
    }

    return idx;
}

void freeIndex(Index* idx) {
    if (!idx) return;

    switch (idx->type) {
        case TREE_AVL:
            freeAVLTree((AVLTree*)idx->tree);
            break;
        case TREE_RB:
            freeRBTree((RBTree*)idx->tree);
            break;
        case TREE_BTREE:
            freeBTree((BTree*)idx->tree);
            break;
        default:
            break;
    }

    free(idx);
}

void insertTerm(Index* idx, const char* term, int doc_id, const char* title) {
    if (!idx || !term) return;

    switch (idx->type) {
        case TREE_AVL:
            avlInsert((AVLTree*)idx->tree, term, doc_id, title);
            break;
        case TREE_RB:
            rbInsert((RBTree*)idx->tree, term, doc_id, title);
            break;
        case TREE_BTREE:
            btreeInsert((BTree*)idx->tree, term, doc_id, title);
            break;
        default:
            break;
    }
}

Vector* lookupTerm(const Index* idx, const char* term) {
    if (!idx || !term) return NULL;

    switch (idx->type) {
        case TREE_AVL:
            return avlSearch((const AVLTree*)idx->tree, term);
        case TREE_RB:
            return rbSearch((const RBTree*)idx->tree, term);
        case TREE_BTREE:
            return btreeSearch((const BTree*)idx->tree, term);
        default:
            return NULL;
    }
}

void indexDocument(
    Index* idx,
    int doc_id,
    const char* title,
    const char** tokens,
    int n_tokens
) {
    if (!idx || !tokens || n_tokens <= 0) return;

    for (int i = 0; i < n_tokens; i++) {
        if (tokens[i] && tokens[i][0] != '\0') {
            insertTerm(idx, tokens[i], doc_id, title);
        }
    }
}

void traverseIndex(
    const Index* idx,
    void (*visit)(const char* key, Vector* postings, void* ctx),
    void* ctx
) {
    if (!idx || !visit) return;

    switch (idx->type) {
        case TREE_AVL:
            avlTraverse((const AVLTree*)idx->tree, visit, ctx);
            break;
        case TREE_RB:
            rbTraverse((const RBTree*)idx->tree, visit, ctx);
            break;
        case TREE_BTREE:
            btreeTraverse((const BTree*)idx->tree, visit, ctx);
            break;
        default:
            break;
    }
}

// Callback для обхода дерева при сохранении индекса
static void savePostingCallback(const char* key, Vector* postings, void* ctx) {
    FILE* f = (FILE*)ctx;
    fprintf(f, "%s\t", key);
    
    for (size_t i = 0; i < postings->size; i++) {
        PostingEntry* entry = vectorGet(postings, i);
        if (!entry) continue;
        
        if (i > 0) fputc('|', f);
        fprintf(f, "%d:%s", entry->doc_id, entry->title);
    }
    fputc('\n', f);
}

void saveIndex(const Index* idx, const char* path) {
    if (!idx || !path) return;

    FILE* f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "Failed to open index for writing: %s\n", path);
        return;
    }

    fprintf(f, "# index type: %s\n", typeName(idx->type));
    
    // обход дерева, запись в файл
    traverseIndex(idx, savePostingCallback, f);

    fclose(f);
}

Index* loadIndex(const char* path, TreeType type) {
    Index* idx = createIndex(type);
    if (!idx) return NULL;

    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Failed to open index for reading: %s\n", path);
        return idx;
    }

    // большой буфер в куче на 16Мб, потому что список может быть большим
    size_t buf_size = 16 * 1024 * 1024; 
    char* line = malloc(buf_size);
    if (!line) {
        fprintf(stderr, "Memory allocation failed during loadIndex\n");
        fclose(f);
        return idx;
    }

    while (fgets(line, buf_size, f)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        // удаление переноса строки
        line[strcspn(line, "\r\n")] = '\0';

        char* tab = strchr(line, '\t');
        if (!tab) continue;

        *tab = '\0'; // разделяем term и postings
        char* term = line;
        char* postings = tab + 1;

        // парсинг списка доков
        char* token = strtok(postings, "|");
        while (token) {
            char* colon = strchr(token, ':');
            if (colon) {
                *colon = '\0';
                int doc_id = atoi(token);
                const char* title = colon + 1;
                insertTerm(idx, term, doc_id, title);
            }
            token = strtok(NULL, "|");
        }
    }

    free(line);
    fclose(f);
    return idx;
}

void runIndex(TreeType type, const char* data_path, const char* idx_path) {
    if (!data_path || !idx_path) return;

    printf("Indexing started\n");
    printf("type=%s\n", typeName(type));
    printf("data=%s\n", data_path);
    printf("index=%s\n", idx_path);

    Index* idx = createIndex(type);
    if (!idx) {
        fprintf(stderr, "Failed to create index\n");
        return;
    }

    FILE* f = fopen(data_path, "r");
    if (!f) {
        fprintf(stderr, "Failed to open data file: %s\n", data_path);
        freeIndex(idx);
        return;
    }

    // буфер для чтения одной строки JSONL
    size_t buf_size = 2 * 1024 * 1024;
    char* line = malloc(buf_size);
    if (!line) {
        fprintf(stderr, "Memory allocation failed during runIndex\n");
        fclose(f);
        freeIndex(idx);
        return;
    }

    int docs_processed = 0;

    // быстрый парсинг JSONL, чтобы не использовать тяжёлые библиотеки
    while (fgets(line, buf_size, f)) {
        // парсинг doc_id
        char* p = strstr(line, "\"doc_id\"");
        if (!p) continue;
        p = strpbrk(p + 8, "0123456789");
        if (!p) continue;
        int doc_id = atoi(p);

        // парсинг title
        char title[256] = {0};
        p = strstr(line, "\"title\"");
        if (p) {
            p = strchr(p + 7, '"');
            if (p) {
                p++; // пропуск ковычки
                char* end = strstr(p, "\", \"tokens\""); // поиск конца title
                if (!end) end = strchr(p, '"'); // если структура другая, фолбек
                if (end) {
                    size_t len = end - p;
                    if (len >= sizeof(title)) len = sizeof(title) - 1;
                    strncpy(title, p, len);
                    title[len] = '\0';
                }
            }
        }

        // парсинг tokens
        const char* tokens[2048]; // максимум токенов на документ
        int n_tokens = 0;
        
        p = strstr(line, "\"tokens\"");
        if (p) {
            p = strchr(p, '[');
            if (p) {
                p++;
                while (*p && *p != ']') {
                    if (*p == '"') {
                        p++; // начало токена
                        char* end = strchr(p, '"');
                        if (end) {
                            *end = '\0'; // заменяем ковычку на конец строки
                            if (n_tokens < 2048) {
                                tokens[n_tokens++] = p;
                            }
                            p = end + 1;
                        }
                    } else {
                        p++;
                    }
                }
            }
        }

        indexDocument(idx, doc_id, title, tokens, n_tokens);
        docs_processed++;
        
        if (docs_processed % 10000 == 0) {
            printf("Indexed %d documents...\n", docs_processed);
        }
    }

    printf("Total documents indexed: %d\n", docs_processed);
    
    printf("Saving index to %s...\n", idx_path);
    saveIndex(idx, idx_path);
    
    free(line);
    fclose(f);
    freeIndex(idx);

    printf("Indexing finished\n");
}