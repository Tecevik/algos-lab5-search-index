#include <stdio.h>
#include "rbtree.h"

int main(void) {
    RBTree* tree = createRBTree();

    if (!tree) {
        fprintf(stderr, "RB create failed\n");
        return 1;
    }

    freeRBTree(tree);

    printf("RB tests: skeleton OK\n");
    return 0;
}