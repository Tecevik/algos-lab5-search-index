#include "posting.h"
#include <stdlib.h>
#include <string.h>

Vector* createPostingList(void) {
    return vectorCreate(sizeof(PostingEntry));
}

void appendPosting(Vector* list, int doc_id, const char* title) {
    if (!list || !title) return;

    if (list->size > 0) {
        PostingEntry* last = (PostingEntry*)vectorGet(list, list->size - 1);
        if (last && last->doc_id == doc_id) {
            return;
        }
    }
    
    PostingEntry entry;
    entry.doc_id = doc_id;
    strncpy(entry.title, title, 255);
    entry.title[255] = '\0';

    vectorPushBack(list, &entry);
}

Vector* clonePostingList(const Vector* src) {
    if (!src) return NULL;
    Vector* clone = vectorCreate(src->elem_size);
    if (!clone) return NULL;

    for (size_t i = 0; i < src->size; i++) {
        const PostingEntry* entry = (const PostingEntry*)vectorGet(src, i);
        if (entry) {
            vectorPushBack(clone, entry);
        }
    }
    
    return clone;
}

PostingEntry* getPostingEntry(const Vector* list, size_t index) {
    if (!list) return NULL;
    return (PostingEntry*)vectorGet(list, index);
}

size_t getPostingListSize(const Vector* list) {
    return list ? list->size : 0;
}