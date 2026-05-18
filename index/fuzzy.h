#pragma once

#include "search.h"

#define MAX_TERM_LEN 256

typedef struct {
    char    term[MAX_TERM_LEN];
    int     distance;
    Vector* postings;
} FuzzyCandidate;

int levenshteinDistance(const char* a, const char* b);

Vector* fuzzyFindCandidates(
    Index* idx,
    const char* term,
    int max_distance
);

SearchResults* fuzzySearch(
    Index* idx,
    const char* query,
    int max_distance
);