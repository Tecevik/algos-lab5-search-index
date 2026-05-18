#include "fuzzy.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "../common/vector.h"

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

// алгоритм Левенштейна
int levenshteinDistance(const char* a, const char* b) {
    int len_a = strlen(a);
    int len_b = strlen(b);
    
    int matrix[len_a + 1][len_b + 1];
    
    for (int i = 0; i <= len_a; i++) matrix[i][0] = i;
    for (int j = 0; j <= len_b; j++) matrix[0][j] = j;
    
    for (int i = 1; i <= len_a; i++) {
        for (int j = 1; j <= len_b; j++) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            matrix[i][j] = MIN3(
                matrix[i - 1][j] + 1, // удаление
                matrix[i][j - 1] + 1, // вставка
                matrix[i - 1][j - 1] + cost // замена
            );
        }
    }
    return matrix[len_a][len_b];
}

// контекст для передачи в колбек обхода дерева
typedef struct {
    const char* target_term;
    int max_dist;
    Vector* candidates;
} TraverseCtx;

// колбек вызывается для каждого слова в индексе
static void fuzzyCallback(const char* key, Vector* postings, void* context) {
    TraverseCtx* ctx = (TraverseCtx*)context;
    
    int dist = levenshteinDistance(ctx->target_term, key);
    if (dist <= ctx->max_dist) {
        FuzzyCandidate cand;
        strncpy(cand.term, key, MAX_TERM_LEN - 1);
        cand.term[MAX_TERM_LEN - 1] = '\0';
        cand.distance = dist;
        cand.postings = postings; // сохраняем указатель
        
        vectorPushBack(ctx->candidates, &cand);
    }
}

Vector* fuzzyFindCandidates(Index* idx, const char* term, int max_distance) {
    Vector* candidates = vectorCreate(sizeof(FuzzyCandidate));
    
    TraverseCtx ctx = { term, max_distance, candidates };
    traverseIndex(idx, fuzzyCallback, &ctx);
    
    return candidates;
}

// вспомогательная структура для агрегации очков по документам
typedef struct {
    int doc_id;
    char title[256];
    int matched_terms;
    int total_distance;
    int score;
} DocScore;

// компаратор для сортировки по score
static int compareDocScores(const void* a, const void* b) {
    DocScore* da = (DocScore*)a;
    DocScore* db = (DocScore*)b;
    return db->score - da->score; 
}

SearchResults* fuzzySearch(Index* idx, const char* query, int max_distance) {
    clock_t start = clock();
    
    SearchResults* sr = malloc(sizeof(SearchResults));
    sr->results = vectorCreate(sizeof(SearchResult));
    sr->total = 0;
    sr->time_ms = 0.0;

    if (!idx || !query || query[0] == '\0') {
        sr->time_ms = ((double)(clock() - start) * 1000.0) / CLOCKS_PER_SEC;
        return sr;
    }

    // токенизация запроса
    char tokens[32][128];
    int n_tokens = 0;
    const char* p = query;
    int pos = 0;
    while (1) {
        if (*p != '\0' && isalnum(*p)) {
            if (pos < 127) tokens[n_tokens][pos++] = tolower(*p);
        } else {
            if (pos > 0) {
                tokens[n_tokens][pos] = '\0';
                n_tokens++;
                pos = 0;
                if (n_tokens >= 32) break;
            }
            if (*p == '\0') break;
        }
        p++;
    }

    Vector* doc_scores = vectorCreate(sizeof(DocScore));

    // поиск кандидатов для каждого токена
    for (int i = 0; i < n_tokens; i++) {
        Vector* candidates = fuzzyFindCandidates(idx, tokens[i], max_distance);
        
        // для каждого кандидата смотрим его документы
        for (size_t c = 0; c < candidates->size; c++) {
            FuzzyCandidate* cand = vectorGet(candidates, c);
            
            for (size_t p_idx = 0; p_idx < cand->postings->size; p_idx++) {
                // извлекаем PostingEntry
                int* entry_ptr = vectorGet(cand->postings, p_idx); 
                int doc_id = entry_ptr[0]; // doc_id это первое поле в PostingEntry
                char* title = (char*)(entry_ptr + 1); // title идёт следом
                
                // поиск документа в списке DocScore
                int found = 0;
                for (size_t d = 0; d < doc_scores->size; d++) {
                    DocScore* ds = vectorGet(doc_scores, d);
                    if (ds->doc_id == doc_id) {
                        ds->matched_terms++;
                        ds->total_distance += cand->distance;
                        found = 1;
                        break;
                    }
                }
                
                // если документ встретился первый раз, добавляем его
                if (!found) {
                    DocScore new_ds;
                    new_ds.doc_id = doc_id;
                    strncpy(new_ds.title, title, 255);
                    new_ds.title[255] = '\0';
                    new_ds.matched_terms = 1;
                    new_ds.total_distance = cand->distance;
                    vectorPushBack(doc_scores, &new_ds);
                }
            }
        }
        vectorFree(candidates);
    }

    // считаем итоговый score для документов и фильтруем те, где совпали не все слова
    Vector* final_docs = vectorCreate(sizeof(DocScore));
    for (size_t i = 0; i < doc_scores->size; i++) {
        DocScore* ds = vectorGet(doc_scores, i);
        // документ должен содержать совпадения по всем словам запроса
        if (ds->matched_terms == n_tokens) {
            ds->score = (ds->matched_terms * 10) - ds->total_distance;
            vectorPushBack(final_docs, ds);
        }
    }

    // сортируем документы по score
    if (final_docs->size > 0) {
        qsort(final_docs->data, final_docs->size, final_docs->elem_size, compareDocScores);
    }

    sr->total = final_docs->size;
    size_t limit = final_docs->size < 10 ? final_docs->size : 10;
    
    // Перекладываем в SearchResults
    for (size_t i = 0; i < limit; i++) {
        DocScore* ds = vectorGet(final_docs, i);
        SearchResult result;
        result.doc_id = ds->doc_id;
        result.score = ds->score;
        strncpy(result.title, ds->title, 255);
        result.title[255] = '\0';
        vectorPushBack(sr->results, &result);
    }

    vectorFree(doc_scores);
    vectorFree(final_docs);

    sr->time_ms = ((double)(clock() - start) * 1000.0) / CLOCKS_PER_SEC;
    return sr;
}