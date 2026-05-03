#include <stdio.h>
#include "avl.h"

int main(void) {
    AVLTree* tree = createAVLTree();

    if (!tree) {
        fprintf(stderr, "AVL create failed\n");
        return 1;
    }

    freeAVLTree(tree);

    printf("AVL tests: skeleton OK\n");
    return 0;
}