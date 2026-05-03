/*
 * Модуль: Search.
 *
 * Ответственный: Егор.
 *
 * Назначение:
 * Здесь реализуется логика поиска по уже построенному индексу.
 *
 * Search не должен знать, какое дерево используется внутри.
 * Для доступа к данным используется только lookupTerm().
 *
 * Базовая семантика поиска:
 *
 *     "python list" = документы, где есть И "python", И "list"
 *
 * То есть используется AND-семантика через пересечение posting list'ов.
 *
 * Что нужно реализовать:
 * 1. Токенизацию пользовательского запроса.
 * 2. Получение posting list для каждого токена.
 * 3. Пересечение posting list'ов.
 * 4. Формирование top-10 результатов.
 * 5. Подсчёт времени поиска.
 * 6. Текстовый вывод.
 * 7. JSON-вывод для frontend.
 *
 * Важно:
 * В режиме --json нельзя печатать ничего, кроме валидного JSON.
 */
#include "search.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_QUERY_TOKENS 32
#define MAX_TOKEN_LEN 128
#define TOP_K 10

static double elapsed_ms(clock_t start, clock_t end) {
    return ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC;
}

static int tokenizeQuery(const char* query, char tokens[MAX_QUERY_TOKENS][MAX_TOKEN_LEN]) {
    if (!query) return 0;

    int count = 0;
    int pos = 0;

    for (const char* p = query; ; p++) {
        unsigned char ch = (unsigned char)*p;

        if (*p != '\0' && isalnum(ch)) {
            if (pos < MAX_TOKEN_LEN - 1) {
                tokens[count][pos++] = (char)tolower(ch);
            }
        } else {
            if (pos > 0) {
                tokens[count][pos] = '\0';
                count++;
                pos = 0;

                if (count >= MAX_QUERY_TOKENS) {
                    break;
                }
            }

            if (*p == '\0') {
                break;
            }
        }
    }

    return count;
}

static int postingDocIdAt(const Vector* list, size_t i) {
    const PostingEntry* entry = getVectorItemConst(list, i);
    return entry ? entry->doc_id : -1;
}

Vector* intersectPostings(Vector** lists, int n) {
    if (!lists || n <= 0) return createPostingList();

    for (int i = 0; i < n; i++) {
        if (!lists[i] || lists[i]->size == 0) {
            return createPostingList();
        }
    }

    /*
     * Простая реализация через первый список.
     * Для MVP достаточно. Потом можно оптимизировать:
     * - сортировать списки по длине;
     * - two-pointer intersection;
     * - учитывать score.
     */
    Vector* result = createPostingList();
    Vector* base = lists[0];

    for (size_t i = 0; i < base->size; i++) {
        PostingEntry* base_entry = getVectorItem(base, i);
        if (!base_entry) continue;

        int found_in_all = 1;

        for (int j = 1; j < n; j++) {
            int found = 0;

            for (size_t k = 0; k < lists[j]->size; k++) {
                if (postingDocIdAt(lists[j], k) == base_entry->doc_id) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                found_in_all = 0;
                break;
            }
        }

        if (found_in_all) {
            appendPosting(result, base_entry->doc_id, base_entry->title);
        }
    }

    return result;
}

SearchResults* search(Index* idx, const char* query) {
    clock_t start = clock();

    SearchResults* sr = malloc(sizeof(SearchResults));
    if (!sr) return NULL;

    sr->results = createVector(sizeof(SearchResult));
    sr->total = 0;
    sr->time_ms = 0.0;

    if (!idx || !query || query[0] == '\0') {
        sr->time_ms = elapsed_ms(start, clock());
        return sr;
    }

    char tokens[MAX_QUERY_TOKENS][MAX_TOKEN_LEN];
    int n_tokens = tokenizeQuery(query, tokens);

    if (n_tokens <= 0) {
        sr->time_ms = elapsed_ms(start, clock());
        return sr;
    }

    Vector* lists[MAX_QUERY_TOKENS];

    for (int i = 0; i < n_tokens; i++) {
        lists[i] = lookupTerm(idx, tokens[i]);

        if (!lists[i]) {
            sr->time_ms = elapsed_ms(start, clock());
            return sr;
        }
    }

    Vector* intersection = intersectPostings(lists, n_tokens);
    if (!intersection) {
        sr->time_ms = elapsed_ms(start, clock());
        return sr;
    }

    sr->total = (int)intersection->size;

    size_t limit = intersection->size < TOP_K ? intersection->size : TOP_K;

    for (size_t i = 0; i < limit; i++) {
        PostingEntry* entry = getVectorItem(intersection, i);
        if (!entry) continue;

        SearchResult result;
        result.doc_id = entry->doc_id;
        result.score = n_tokens;

        strncpy(result.title, entry->title, MAX_TITLE_LEN - 1);
        result.title[MAX_TITLE_LEN - 1] = '\0';

        appendVectorItem(sr->results, &result);
    }

    vectorFree(intersection);

    sr->time_ms = elapsed_ms(start, clock());
    return sr;
}

void printResultsText(const SearchResults* sr) {
    if (!sr) return;

    printf("Time: %.3f ms | Found: %d documents\n\n", sr->time_ms, sr->total);

    for (size_t i = 0; i < sr->results->size; i++) {
        const SearchResult* r = getVectorItemConst(sr->results, i);
        if (!r) continue;

        printf("%zu. [id=%d] %s | score=%d\n", i + 1, r->doc_id, r->title, r->score);
    }
}

static void printJsonEscaped(const char* s) {
    if (!s) return;

    for (const char* p = s; *p; p++) {
        switch (*p) {
            case '"':
                printf("\\\"");
                break;
            case '\\':
                printf("\\\\");
                break;
            case '\n':
                printf("\\n");
                break;
            case '\r':
                printf("\\r");
                break;
            case '\t':
                printf("\\t");
                break;
            default:
                putchar(*p);
                break;
        }
    }
}

void printResultsJSON(const SearchResults* sr) {
    /*
     * Важно для frontend:
     * Эта функция должна печатать только валидный JSON.
     *
     * Нельзя добавлять сюда debug-сообщения вида:
     * printf("DEBUG ...");
     *
     * Streamlit-приложение парсит stdout через json.loads().
     */
    if (!sr) {
        printf("{\"total\":0,\"time_ms\":0,\"results\":[]}\n");
        return;
    }

    printf("{");
    printf("\"total\":%d,", sr->total);
    printf("\"time_ms\":%.3f,", sr->time_ms);
    printf("\"results\":[");

    for (size_t i = 0; i < sr->results->size; i++) {
        const SearchResult* r = getVectorItemConst(sr->results, i);
        if (!r) continue;

        if (i > 0) {
            printf(",");
        }

        printf("{");
        printf("\"doc_id\":%d,", r->doc_id);
        printf("\"title\":\"");
        printJsonEscaped(r->title);
        printf("\",");
        printf("\"score\":%d", r->score);
        printf("}");
    }

    printf("]}\n");
}

void freeSearchResults(SearchResults* sr) {
    if (!sr) return;

    vectorFree(sr->results);
    free(sr);
}