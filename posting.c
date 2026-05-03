#include "posting.h"

#include <string.h>

Vector* createPostingList(void) {
    return createVector(sizeof(PostingEntry));
}

void appendPosting(Vector* list, int doc_id, const char* title) {
    if (!list) return;

    /*
     * Защита от повторного добавления того же документа.
     *
     * Если одно и то же слово встречается в документе несколько раз,
     * posting list не должен хранить один и тот же doc_id несколько раз.
     *
     * Во время индексации документы обычно обрабатываются последовательно,
     * поэтому достаточно проверить последнюю запись.
     */
    if (list->size > 0) {
        PostingEntry* last = getVectorItem(list, list->size - 1);

        if (last && last->doc_id == doc_id) {
            return;
        }
    }

    PostingEntry entry;
    entry.doc_id = doc_id;

    if (title) {
        strncpy(entry.title, title, MAX_TITLE_LEN - 1);
        entry.title[MAX_TITLE_LEN - 1] = '\0';
    } else {
        entry.title[0] = '\0';
    }

    appendVectorItem(list, &entry);
}

Vector* clonePostingList(const Vector* src) {
    if (!src) return NULL;

    Vector* clone = createPostingList();

    if (!clone) {
        return NULL;
    }

    for (size_t i = 0; i < src->size; i++) {
        const PostingEntry* entry = getVectorItemConst(src, i);

        if (entry) {
            appendVectorItem(clone, entry);
        }
    }

    return clone;
}