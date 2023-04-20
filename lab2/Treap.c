#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>

const unsigned int KEY_LEN = 260;

typedef struct _TNode {
    struct _TNode *left;
    struct _TNode *right;
    char          *key;
    uint64_t      value;
    int           y;
} TNode;

TNode             *NodeCreate(const char *string, uint64_t value);
void              NodeDestroy(TNode *root);
TNode             *NodeMerge(TNode *treap1, TNode *treap2);
void              NodeSplit(TNode *t, const char *key, TNode **treap1, TNode **treap2);
TNode             *NodeGetMinNode(TNode *ptr);
TNode             *NodeSearch(TNode *node, const char *key);

typedef struct {
    TNode         *root;
} TTreap;

TTreap            TreapCreate();
TNode             *TreapFind(TTreap *treap, const char *key);
int               TreapInsert(TTreap *treap, const char *key, uint64_t value);
int               TreapRemove(TTreap *treap, char *key);
void              TreapDestroy(TTreap *treap);




void strtolower(char *str) {
    while ((*str) != '\0') {
        *str = tolower(*str);
        ++str;
    }
}

int main(int argc, char *argv[]) {
    TTreap treap = TreapCreate();
    int type;
    char input[KEY_LEN];
    uint64_t value;
    while (scanf("%s", input) > 0) {
        if (input[0] == '+') {
            scanf("%s ""%"SCNu64, input, &value);
            strtolower(input);
            if (TreapInsert(&treap, input, value) == -1) {
                printf("Exist\n");
            } else {
                printf("OK\n");
            }
        } else if (input[0] == '-') {
            scanf("%s", input);
            strtolower(input);
            if (TreapRemove(&treap, input) == -1) {
                printf("NoSuchWord\n");
            } else {
                printf("OK\n");
            }
        } else {
            strtolower(input);
            TNode *elem = TreapFind(&treap, input);
            if (elem == NULL) {
                printf("NoSuchWord\n");
            } else {
                printf("OK: %"SCNu64"\n", elem->value);
            }
        }
    }
    TreapDestroy(&treap);
    return 0;
}

TNode *NodeCreate(const char *string, uint64_t value) {
    TNode *node = malloc(sizeof(TNode));
    if (node == NULL) {
        fprintf(stderr, "ERROR: could not allocate memory at line %d\n", __LINE__);
        exit(1);
    }
    node->left = NULL;
    node->right = NULL;
    size_t strLen = strlen(string) + 1;
    node->y = rand();
    node->key = malloc(sizeof(char) * strLen);
    if (node->key == NULL) {
        fprintf(stderr, "ERROR: could not allocate memory at line %d\n", __LINE__);
        exit(1);
    }
    memcpy(node->key, string, strLen);
    node->value = value;
    return node;
}

TTreap TreapCreate() {
    TTreap t = {0};
    return t;
}

void NodeDestroy(TNode *root) {
    if (root == NULL) return;
    NodeDestroy(root->left);
    NodeDestroy(root->right);
    free(root->key);
    root->left = NULL;
    root->left = NULL;
    free(root);
}

void TreapDestroy(TTreap *treap) {
    if (treap == NULL) return;
    NodeDestroy(treap->root);
}

TNode *NodeMerge(TNode *treap1, TNode *treap2) {
    if (treap1 == NULL) {
        return treap2;
    }
    if (treap2 == NULL) {
        return treap1;
    }
    if (treap1->y > treap2->y) {
        treap1->right = NodeMerge(treap1->right, treap2);
        return treap1;
    } else {
        treap2->left = NodeMerge(treap1, treap2->left);
        return treap2;
    }
}

void NodeSplit(TNode *t, const char *key, TNode **treap1, TNode **treap2) {
    if (t == NULL) {
        *treap1 = NULL;
        *treap2 = NULL;
        return;
    }
    if (strcmp(t->key, key) < 0) {
        NodeSplit(t->right, key, &t->right, treap2);
        *treap1 = t;
    } else {
        NodeSplit(t->left, key, treap1, &t->left);
        *treap2 = t;
    }
}

TNode *NodeGetMinNode(TNode *ptr) {
    if (ptr == NULL) return NULL;
    if (ptr->left == NULL) return ptr;
    return NodeGetMinNode(ptr->left);
}

TNode *NodeSearch(TNode *node, const char *key) {
    if (node == NULL) return NULL;
    int rc = strcmp(node->key, key); 
    if (rc < 0) {
        return NodeSearch(node->right, key);
    } else if (rc > 0) {
        return NodeSearch(node->left, key);
    } else {
        return node;
    }
}

TNode *TreapFind(TTreap *treap, const char *key) {
    if (treap == NULL) return NULL;
    return NodeSearch(treap->root, key);
}

// -1 = already exist;
int TreapInsert(TTreap *treap, const char *key, uint64_t value) {
    if (treap == NULL) return -1;
    TNode *l = NULL;
    TNode *r = NULL;
    TNode *elem = NULL;
    NodeSplit(treap->root, key, &l, &r);
    elem = NodeGetMinNode(r);
    if (elem != NULL && strcmp(elem->key, key) == 0) {
        treap->root = NodeMerge(l, r);
        return -1;
    }
    TNode *m = NodeCreate(key, value);
    treap->root = NodeMerge(NodeMerge(l, m), r);
    return 1;
}

// -1 = no such node;
int TreapRemove(TTreap *treap, char *key) {
    if (treap == NULL) return -1;
    size_t strLen = strlen(key);
    key[strLen + 1] = '\0';
    TNode* l0 = NULL;
    TNode* r0 = NULL;
    TNode* l = NULL;
    TNode* r = NULL;
    NodeSplit(treap->root, key, &l0, &r0);
    ++key[strLen];
    NodeSplit(r0, key, &l, &r);
    if (l != NULL) {
        NodeDestroy(l);
        treap->root = NodeMerge(l0, r);
        --key[strLen];
        return 1;
    }
    r0 = NodeMerge(l, r);
    treap->root = NodeMerge(l0, r0);
    --key[strLen];
    return -1;
}

