#include <stdio.h>
#include "btree.h"

int main(void) {
    BTree* tree = createBTree();

    if (!tree) {
        fprintf(stderr, "B-tree create failed\n");
        return 1;
    }

    freeBTree(tree);

    printf("B-tree tests: skeleton OK\n");
    return 0;
}