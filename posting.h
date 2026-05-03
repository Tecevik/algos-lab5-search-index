#pragma once

#include "common/vector.h"

#define MAX_TITLE_LEN 256

typedef struct {
    int  doc_id;
    char title[MAX_TITLE_LEN];
} PostingEntry;

/*
 * Posting list — Vector с элементами PostingEntry.
 *
 * createPostingList() создаёт Vector(sizeof(PostingEntry)).
 * appendPosting() добавляет документ в posting list.
 * clonePostingList() делает глубокую копию списка.
 *
 * Освобождение: vectorFree(list).
 */
Vector* createPostingList(void);
void    appendPosting(Vector* list, int doc_id, const char* title);
Vector* clonePostingList(const Vector* list);