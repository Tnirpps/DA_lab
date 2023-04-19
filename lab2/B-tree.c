#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define KEY_LEN (257)
#define DEGREE 5

typedef struct {
    uint64_t       value;
    char           key[KEY_LEN];
} TElem;

typedef struct {
    TElem*         storage;
    size_t         size;
    size_t         capacity;
    unsigned int   freeHead;
    unsigned int   freeTail;
} TElist;

TElist             ElListCreate(size_t capacity);
void               ElListExpand(TElist* v);
unsigned int       ElListInsert(TElist* list, const TElem* data);
void               ElListRemove(TElist* list, unsigned int index);
void               ElListDestroy(TElist* list);
void               ElListPrint(const TElist* list);
void               ElListRecover(TElist *list, int fd); 
void               ElListDump(const TElist *list, int fd);

typedef struct {
    unsigned int   keys_num;
    unsigned int   parent;
    unsigned int   keys[2 * DEGREE];
    unsigned int   children[2 * DEGREE + 1];
    unsigned int   distToBottom;
} TNode;

typedef struct {
    TNode*         storage;
    size_t         size;
    size_t         capacity;
    unsigned int   freeHead;
    unsigned int   freeTail;
} TNlist;

TNlist           NodListCreate(size_t capacity);
void             NodListExpand(TNlist* v);
unsigned int     NodListInsert(TNlist* list, const TNode* data);
void             NodListRemove(TNlist* list, unsigned int index);
void             NodListDestroy(TNlist* list);
void             NodListPrint(const TNlist* list);
void             NodListRecover(TNlist *list, int fd); 
void             NodListDump(const TNlist *list, int fd);


int              getchar_unlocked(void);
void             ungetchar_unlocked(void);
int              putchar_unlocked(int);
int              _scanCHR(char* x);
int              _scanSTR(char* x);
int              _scanINT(int64_t* x);
int              _scanUINT(uint64_t* x);

void             _printINT(int64_t x);
void             _printUINT(uint64_t x);
void             _printSTR(const char* s);
void             _printCHR(char c);

typedef struct {
   TElist eMemory;
   TNlist nMemory;
   unsigned int root;
} TTree;

TTree             TreeCreate(void);
void              TreeRecover(TTree *tree, int fd);
void              TreeDump(const TTree *tree, int fd);
void              TreeSearch(TTree* tree, const char* str, unsigned int* ptr, unsigned int* pos);
void              TreeDelete(TTree* tree, unsigned int node);
void              TreeSplitChild(TTree* tree, unsigned int parent, unsigned int pos);
void              Insert(TTree* tree, unsigned int node, const TElem* elem);
int               TreeInsert(TTree* tree, const TElem* elem);
int               SearchInNode(TTree *tree, unsigned int node, const char *str);
void              RemoveFromNode(TTree *tree, unsigned int node, int pos);
void              MergeNodes(TTree *tree, unsigned int parentInd, int pos);
void              Rebalance(TTree *tree, unsigned int nodeInd, int* pos);
void              RemoveNode(TTree *tree, unsigned int node, const char *str);
void              TreeDestroy(TTree* t);
int               TreeRemove(TTree *tree, const char *str);

void StringToLower(char *str) {
    for (size_t i = 0; str[i] != '\0'; ++i) {
        str[i] = tolower(str[i]);
    }
}

int main(int argc, char *argv[]) {
    TTree tree = TreeCreate();
    TElem input = {0};
    char cmd;
    while (_scanCHR(&cmd) != EOF) {
        if (cmd == '+') {
            // add
            _scanSTR(input.key);
            _scanUINT(&input.value);

            StringToLower(input.key);

            int rs = TreeInsert(&tree, &input);
            if (rs) {
                _printSTR("OK\n");
            } else {
                _printSTR("Exist\n");
            }
        } else if (cmd == '-') {
            // remove
            _scanSTR(input.key);

            StringToLower(input.key);
            
            int rs = TreeRemove(&tree, input.key);
            if (rs) {
                _printSTR("OK\n");
            } else {
                _printSTR("NoSuchWord\n");
            }
        } else if (cmd == '!') {
            // Load or Save
            _scanSTR(input.key);
            if (strcmp(input.key, "Save") == 0) {
                // read path
                _scanSTR(input.key);
                int saveTargetFile = open(input.key, O_WRONLY | O_CREAT, 0777);

                if (saveTargetFile == -1) {
                } else {
                    TreeDump(&tree, saveTargetFile);
                    close(saveTargetFile);
                }
                _printSTR("OK\n");
            } else {
                // read path
                _scanSTR(input.key);

                int recoverTargetFile = open(input.key, O_RDWR);
                if (recoverTargetFile == -1) {
                    TreeDestroy(&tree);
                    tree = TreeCreate();
                } else {
                    TreeRecover(&tree, recoverTargetFile);
                    close(recoverTargetFile);
                }
                _printSTR("OK\n");
            }
        } else {
            // find
            if (cmd == '\n') {
                continue;
            }
            // return char back to stdin
            ungetc(cmd, stdin);
            // read full string
            _scanSTR(input.key);

            StringToLower(input.key);
            unsigned int ptr, pos;
            
            TreeSearch(&tree, input.key, &ptr, &pos);
            if (ptr == 0) {
                _printSTR("NoSuchWord\n");
            } else {
                unsigned int p = tree.nMemory.storage[ptr].keys[pos];
                _printSTR("OK: ");
                _printUINT(tree.eMemory.storage[p].value);
                _printCHR('\n');
            }
        }
    }
    TreeDestroy(&tree);
    return 0;
}

void TreeRecover(TTree *tree, int fd) {
    char mark[] = "L@B@_2";
    char text[] = "L@B@_9";
    read(fd, text, 6 * sizeof(char));
    if (strcmp(mark, text) != 0) {
        return;
    }
    NodListRecover(&tree->nMemory, fd);
    ElListRecover(&tree->eMemory, fd);
    read(fd, &tree->root, sizeof(unsigned int));
}

void TreeDump(const TTree *tree, int fd) {
    char HEAD[] = "L@B@_2";
    if (write(fd, HEAD, 6 * sizeof(char)) != 6 * sizeof(char)) {
        return;
    }
    NodListDump(&tree->nMemory, fd);
    ElListDump(&tree->eMemory, fd);
    write(fd, &tree->root, sizeof(unsigned int));
}

void TreeSearch(TTree* tree, const char* str, unsigned int* ptr, unsigned int* pos) {
    if (tree == NULL) {
        (*ptr) = 0;
        return;
    }
    unsigned int cur = tree->root;
    unsigned int deph = tree->nMemory.storage[cur].distToBottom + 1;
    unsigned int iter;
    unsigned int ind;
    while (deph--) {
        iter = 0;
        while (iter < tree->nMemory.storage[cur].keys_num) {
            ind = tree->nMemory.storage[cur].keys[iter]; 
            if (strcmp(tree->eMemory.storage[ind].key, str) >= 0) {
                break;
            }
            iter++;
        }

        if (iter < tree->nMemory.storage[cur].keys_num) {
            if (strcmp(tree->eMemory.storage[ind].key, str) == 0) {
                (*ptr) = cur;
                (*pos) = iter;
                return;
            }
        }
        cur = tree->nMemory.storage[cur].children[iter]; 
    }
    (*ptr) = 0;
    return;
}

void TreeDelete(TTree* tree, unsigned int node) {
    if (node == 0 || tree == NULL) return;
    if (tree->nMemory.storage[node].distToBottom) {
        for (size_t i = 0; i < 2 * DEGREE + 1; ++i) {
            TreeDelete(tree, tree->nMemory.storage[node].children[i]);
            tree->nMemory.storage[node].children[i] = 0;
            if (i < 2 * DEGREE) {
                ElListRemove(&tree->eMemory, tree->nMemory.storage[node].keys[i]);
            }
        }
    }
    NodListRemove(&tree->nMemory, node);
}

void TreeSplitChild(TTree* tree, unsigned int parentInd, unsigned int pos) {
    if (tree == NULL) {
        return;
    }
    TNode *parent = &(tree->nMemory.storage[parentInd]);
    unsigned int childInd = parent->children[pos];
    TNode *child = &(tree->nMemory.storage[childInd]);
    TNode new_child  = {0};
    new_child.distToBottom = child->distToBottom;
    new_child.keys_num = DEGREE - 1;
    for (size_t i = 0; i < DEGREE - 1; ++i) {
        new_child.keys[i] = child->keys[i + DEGREE];
    }
    if (child->distToBottom != 0) {
        for (size_t i = 0; i < DEGREE; ++i) {
            new_child.children[i] = child->children[i + DEGREE];
            child->children[i + DEGREE] = 0;
        }
    }
    child->keys_num = DEGREE - 1;

    for (size_t i = parent->keys_num; i >= pos + 1; --i) {
        parent->children[i + 1] = parent->children[i];
        parent->children[i] = 0;
    }

    if (parent->children[pos + 1] != 0) {
        TreeDelete(tree, parent->children[pos + 1]);
    }
    unsigned int NN = NodListInsert(&tree->nMemory, &new_child);

    // important to update pointers because of migrations in nMemory!!!
    parent = &(tree->nMemory.storage[parentInd]);
    child = &(tree->nMemory.storage[childInd]);
    parent->children[pos + 1] = NN; 

    for (size_t i = parent->keys_num; i >= pos + 1; --i) {
        parent->keys[i] = parent->keys[i - 1];
    }
    parent->keys[pos] = child->keys[DEGREE - 1];
    ++(parent->keys_num);
}

void Insert(TTree* tree, unsigned int node, const TElem* elem) {
    if (tree == NULL || node == 0 || elem == NULL) return;
    unsigned int i = tree->nMemory.storage[node].keys_num;
    if (tree->nMemory.storage[node].distToBottom == 0) {
        unsigned int ind = 0;
        if (i >= 1) {
            ind = tree->nMemory.storage[node].keys[i - 1];
        }
        while (i >= 1 && strcmp(elem->key,tree->eMemory.storage[ind].key) < 0) {
            tree->nMemory.storage[node].keys[i] = tree->nMemory.storage[node].keys[i - 1];
            --i;
            if (i >= 1) {
                ind = tree->nMemory.storage[node].keys[i - 1];
            }
        }
        tree->nMemory.storage[node].keys[i] = ElListInsert(&tree->eMemory, elem);
        ++(tree->nMemory.storage[node].keys_num);
    } else {
        unsigned int ind = 0;
        if (i >= 1) {
            ind = tree->nMemory.storage[node].keys[i - 1];
        }
        while (i >= 1 && strcmp(elem->key,tree->eMemory.storage[ind].key) < 0) {
            --i;
            if (i >= 1) {
                ind = tree->nMemory.storage[node].keys[i - 1];
            }
        }
        ++i;
        
        unsigned int child = tree->nMemory.storage[node].children[i - 1];
        if (tree->nMemory.storage[child].keys_num == 2 * DEGREE - 1) {
            TreeSplitChild(tree, node, i - 1);
            ind = tree->nMemory.storage[node].keys[i - 1];
            if (strcmp(tree->eMemory.storage[ind].key, elem->key) < 0) {
                ++i;
            }
        }
        Insert(tree, tree->nMemory.storage[node].children[i - 1], elem);
    }
}

int TreeInsert(TTree* tree, const TElem* elem) {
    unsigned int node;
    unsigned int pos;
    TreeSearch(tree, elem->key, &node, &pos);
    if (node != 0) {
        // already Exist
        return 0;
    }
    unsigned int root = tree->root;
    if (tree->nMemory.storage[root].keys_num == 2 * DEGREE - 1) {
        TNode newRoot = {0};
        newRoot.distToBottom = tree->nMemory.storage[root].distToBottom + 1;
        newRoot.children[0] = root;
        tree->root = NodListInsert(&tree->nMemory, &newRoot); 
        TreeSplitChild(tree, tree->root, 0);
    }
    Insert(tree, tree->root, elem);
    return tree->root;
}

TTree TreeCreate() {
    TTree t = {0};
    t.eMemory = ElListCreate(10);
    t.nMemory = NodListCreate(10);
    TNode root = {0};
    t.root = NodListInsert(&t.nMemory, &root);
    return t;
}

int SearchInNode(TTree *tree, unsigned int node, const char *str) {
    if (tree == NULL) return 0;
    TNode *cur = &(tree->nMemory.storage[node]);
    int left = -1;
    int right = cur->keys_num;
    int middle;
    unsigned int ind;
    while (right - left > 1) {
        middle = (left + right) / 2;
        ind = cur->keys[middle];
        if (strcmp(tree->eMemory.storage[ind].key, str) < 0) {
            left = middle;
        } else {
            right = middle;
        }
    }
    if (right < cur->keys_num) {
        ind = cur->keys[right];
        if (strcmp(tree->eMemory.storage[ind].key, str) == 0) {
            return right;
        }
    }
    return -(right + 1);
}

void RemoveFromNode(TTree *tree, unsigned int node, int pos) {
    TNode *cur = &(tree->nMemory.storage[node]);
    for (size_t i = pos; i + 1 < cur->keys_num; ++i) {
        cur->keys[i] = cur->keys[i + 1];
    }
    if (cur->distToBottom) {
        for (size_t i = pos; i < cur->keys_num; ++i) {
            cur->children[i] = cur->children[i + 1];
            cur->children[i + 1] = 0;
        }
    }
    if (cur->keys_num != 0) {
        --(cur->keys_num);
    }
}

void MergeNodes(TTree *tree, unsigned int parentInd, int pos) {
    TNode *parent = &(tree->nMemory.storage[parentInd]);
    unsigned int leftChild = parent->children[pos];
    TNode *leftC = &(tree->nMemory.storage[leftChild]); 
    unsigned int rightChild = parent->children[pos + 1];
    TNode *rightC = &(tree->nMemory.storage[rightChild]); 

    leftC->keys[leftC->keys_num++] = parent->keys[pos]; 

    for (size_t i = 0; i < rightC->keys_num; ++i) {
        leftC->keys[leftC->keys_num++] = rightC->keys[i]; 
    }

    if (rightC->distToBottom) {
        for (size_t i = 0; i <= rightC->keys_num; ++i) {
            leftC->children[DEGREE + i] = rightC->children[i];
            rightC->children[i] = 0;
        }
    }
    rightC->keys_num = 0;
    NodListRemove(&tree->nMemory, rightChild);
    RemoveFromNode(tree, parentInd, pos);
    parent->children[pos] = leftChild; 
}

void Rebalance(TTree *tree, unsigned int nodeInd, int* pos) {
    TNode *node = &(tree->nMemory.storage[nodeInd]);
    unsigned int childInd = node->children[(*pos)];
    TNode *child = &(tree->nMemory.storage[childInd]);
    if ((*pos) >= 1 && tree->nMemory.storage[node->children[(*pos) - 1]].keys_num >= DEGREE) {
        unsigned int ind = node->children[(*pos) - 1];
        TNode *leftN = &(tree->nMemory.storage[ind]);
        
        for (size_t i = child->keys_num; i >= 1; --i) {
            child->keys[i] = child->keys[i - 1];
        }

        if (child->distToBottom) {
            for (size_t i = child->keys_num + 1; i >= 1; --i) {
                child->children[i] = child->children[i - 1];
                child->children[i - 1] = 0;
            }
        }

        child->keys[0] = node->keys[(*pos) - 1];
        child->children[0] = leftN->children[leftN->keys_num];
        leftN->children[leftN->keys_num] = 0;

        ++(child->keys_num);

        node->keys[(*pos) - 1] = leftN->keys[leftN->keys_num - 1];
        --(leftN->keys_num);
    } else if ((*pos) + 1 <= node->keys_num && tree->nMemory.storage[node->children[(*pos) + 1]].keys_num >= DEGREE) {
        unsigned int ind = node->children[(*pos) + 1];
        TNode *rightN = &(tree->nMemory.storage[ind]);
        child->keys[child->keys_num] = node->keys[(*pos)];
        child->children[child->keys_num + 1] = rightN->children[0];
        rightN->children[0] = 0;
        ++(child->keys_num);

        node->keys[(*pos)] = rightN->keys[0];
        RemoveFromNode(tree, ind, 0);
    } else if ((*pos) >= 1 && tree->nMemory.storage[node->children[(*pos) - 1]].keys_num == DEGREE - 1) {
        MergeNodes(tree, nodeInd, (*pos) - 1);
        --(*pos);
    } else if ((*pos) + 1 <= node->keys_num && tree->nMemory.storage[node->children[(*pos) + 1]].keys_num == DEGREE - 1) {
        MergeNodes(tree, nodeInd, (*pos));
    }
}

void RemoveNode(TTree *tree, unsigned int node, const char *str) {
    int pos = SearchInNode(tree, node, str);
    TNode *cur = &(tree->nMemory.storage[node]);
    if (pos >= 0 && cur->distToBottom == 0) {
        RemoveFromNode(tree, node, pos);
        return;
    }
    
    if (pos >= 0 && cur->distToBottom) {
        TNode *leftC = &(tree->nMemory.storage[cur->children[pos]]);
        if (leftC->keys_num >= DEGREE) {
            while (leftC->distToBottom) {
                leftC = &(tree->nMemory.storage[leftC->children[leftC->keys_num]]);
            }
            unsigned int ind = leftC->keys[leftC->keys_num - 1];
            cur->keys[pos] = leftC->keys[leftC->keys_num - 1];
            RemoveNode(tree, cur->children[pos], tree->eMemory.storage[ind].key);
            return;
        }
    
    
        TNode *rightC = &(tree->nMemory.storage[cur->children[pos + 1]]);
        if (rightC->keys_num >= DEGREE) {
            while (rightC->distToBottom) {
                rightC = &(tree->nMemory.storage[rightC->children[0]]);
            }
            unsigned int ind = rightC->keys[0];
            cur->keys[pos] = rightC->keys[0];
            RemoveNode(tree, cur->children[pos + 1], tree->eMemory.storage[ind].key);
            return;
        }
        MergeNodes(tree, node, pos);
        RemoveNode(tree, cur->children[pos], str);
        return;
    }
    pos = -pos - 1;
    if (tree->nMemory.storage[cur->children[pos]].keys_num == DEGREE - 1) {
        Rebalance(tree, node, &pos);
    }
    RemoveNode(tree, cur->children[pos], str);
    return;
}

void TreeDestroy(TTree* t) {
    TreeDelete(t, t->root);
    NodListDestroy(&t->nMemory);
    ElListDestroy(&t->eMemory);
    t->root = 0;
}

int TreeRemove(TTree *tree, const char *str) {
    unsigned int ptr, p;
    TreeSearch(tree, str, &ptr, &p);
    if (ptr == 0) {
        // Not found
        return 0;
    } else {
        RemoveNode(tree, tree->root, str);
        TNode *root = &(tree->nMemory.storage[tree->root]);
        if (root->keys_num == 0 && root->distToBottom) {
            unsigned int newRoot = root->children[0];
            NodListRemove(&tree->nMemory, tree->root);
            tree->root = newRoot;
        }
    }
    return tree->root;
}

void ElListRecover(TElist *list, int fd) {
    size_t cap;
    read(fd, &cap, sizeof(size_t));
    if (cap < list->capacity) {
        list->storage = realloc(list->storage, sizeof(TElem) * cap);
        list->capacity = cap;
    }
    while (cap > list->capacity) {
        ElListExpand(list);
    }
    assert(cap == list->capacity);
    unsigned int strl;
    for (size_t i = 0; i < list->capacity; ++i) {
        read(fd, &strl, sizeof(unsigned int));
        read(fd, &list->storage[i].key, strl * sizeof(char));
        read(fd, &list->storage[i].value, sizeof(uint64_t));
    }
    read(fd, &list->freeHead, sizeof(unsigned int));
    read(fd, &list->freeTail, sizeof(unsigned int));
    read(fd, &list->size,     sizeof(size_t));
}

void ElListDump(const TElist *list, int fd) {
    write(fd, &list->capacity, sizeof(size_t));
    unsigned int strl;;
    for (size_t i = 0; i < list->capacity; ++i) {
        strl = strlen(list->storage[i].key);
        ++strl;
        write(fd, &strl, sizeof(unsigned int));
        write(fd, &list->storage[i].key, strl * sizeof(char));
        write(fd, &list->storage[i].value, sizeof(uint64_t));
    }
    write(fd, &list->freeHead, sizeof(unsigned int));
    write(fd, &list->freeTail, sizeof(unsigned int));
    write(fd, &list->size,     sizeof(size_t));
}

void NodListRecover(TNlist *list, int fd) {
    size_t cap;
    read(fd, &cap, sizeof(size_t));
    if (cap < list->capacity) {
        list->storage = realloc(list->storage, sizeof(TNode) * cap);
        list->capacity = cap;
    }
    while (cap > list->capacity) {
        NodListExpand(list);
    }
    assert(cap == list->capacity);
    read(fd, list->storage,   sizeof(TNode) * cap);
    read(fd, &list->freeHead, sizeof(unsigned int));
    read(fd, &list->freeTail, sizeof(unsigned int));
    read(fd, &list->size,     sizeof(size_t));
}

void NodListDump(const TNlist *list, int fd) {
    write(fd, &list->capacity, sizeof(size_t));
    write(fd, list->storage,   sizeof(TNode) * list->capacity);
    write(fd, &list->freeHead, sizeof(unsigned int));
    write(fd, &list->freeTail, sizeof(unsigned int));
    write(fd, &list->size,     sizeof(size_t));
}

TElist ElListCreate(size_t capacity) {
    if (capacity < 1) capacity = 1;
    /* for tetminator */
    ++capacity;
    TElist list;
    list.storage = malloc(sizeof(TElem) * capacity);
    if (list.storage == NULL) {
        fprintf(stderr, "ERROR: could not creat List (lack of RAM)\n");
        exit(1);
    }
    list.capacity = capacity;
    list.freeHead = 1;
    list.freeTail = capacity - 1;
    list.size     = 0;
    TElem init = {0};
    for (size_t i = 0; i < capacity; ++i) {
        list.storage[i] = init;
        if (i + 1 == capacity) {
            list.storage[i].value = 0;
        } else {
            list.storage[i].value = i + 1;
        }
    }
    return list;
}

void ElListExpand(TElist* list) {
    size_t oldCapacity = list->capacity;
    TElem* tmp = realloc(list->storage, sizeof(TElem) * (oldCapacity << 1));
    if (tmp == NULL) {
        fprintf(stderr, "ERROR: could not expand List (lack of RAM)\n");
        exit(1);
    }
    list->capacity = (oldCapacity << 1);
    TElem init = {0};
    for (size_t i = oldCapacity; i < list->capacity; ++i) {
        tmp[i] = init;
        if (i + 1 == list->capacity) {
            tmp[i].value = 0;
        }
        else {
            tmp[i].value = i + 1;
        }
    }
    if (list->freeHead == 0) {
        list->freeHead = oldCapacity;
    } else {
        tmp[list->freeTail].value = oldCapacity;
    }
    list->freeTail = list->capacity - 1;
    list->storage = tmp;
}

unsigned int ElListInsert(TElist* list, const TElem* data) {
    if (list == NULL) {
        fprintf(stderr, "ERROR: could not push element to List (NULL adress)\n");
        exit(1);
    }
    if (list->freeHead == 0) {
        /* no free space */
        ElListExpand(list);
    }
    unsigned int iter = list->freeHead;
    list->freeHead = list->storage[iter].value;
    memcpy(&list->storage[iter], data, sizeof(TElem));
    ++list->size;
    return iter;
}
void ElListRemove(TElist* list, unsigned int index) {
    if (list == NULL || index == 0 || list->size == 0) return;
    --list->size;
    list->storage[index].value = list->freeHead;
    list->freeHead = index;
}

void ElListDestroy(TElist* list) {
    if (list == NULL) return;
    free(list->storage);
    list->capacity = 0;
    list->size = 0;
    list->freeHead = 0;
    list->freeTail = 0;
}

TNlist NodListCreate(size_t capacity) {
    if (capacity < 1) capacity = 1;
    /* for tetminator */
    ++capacity;
    TNlist list;
    list.storage = malloc(sizeof(TNode) * capacity);
    if (list.storage == NULL) {
        fprintf(stderr, "ERROR: could not creat List (lack of RAM)\n");
        exit(1);
    }
    list.capacity = capacity;
    list.freeHead = 1;
    list.freeTail = capacity - 1;
    list.size     = 0;
    TNode init = {0};
    for (size_t i = 0; i < capacity; ++i) {
        list.storage[i] = init;
        if (i + 1 == capacity) {
            list.storage[i].parent = 0;
        } else {
            list.storage[i].parent = i + 1;
        }
    }
    return list;
}

void NodListExpand(TNlist* list) {
    size_t oldCapacity = list->capacity;
    TNode* tmp = realloc(list->storage, sizeof(TNode) * (oldCapacity << 1));
    if (tmp == NULL) {
        fprintf(stderr, "ERROR: could not expand List (lack of RAM)\n");
        exit(1);
    }
    list->capacity = (oldCapacity << 1);
    TNode init = {0};
    for (size_t i = oldCapacity; i < list->capacity; ++i) {
        tmp[i] = init;
        if (i + 1 == list->capacity) {
            tmp[i].parent = 0;
        }
        else {
            tmp[i].parent = i + 1;
        }
    }
    if (list->freeHead == 0) {
        list->freeHead = oldCapacity;
    } else {
        tmp[list->freeTail].parent = oldCapacity;
    }
    list->freeTail = list->capacity - 1;
    list->storage = tmp;
}

unsigned int NodListInsert(TNlist* list, const TNode* data) {
    if (list == NULL) {
        fprintf(stderr, "ERROR: could not push element to List (NULL adress)\n");
        exit(1);
    }
    if (list->freeHead == 0) {
        /* no free space */
        NodListExpand(list);
    }
    unsigned int iter = list->freeHead;
    list->freeHead = list->storage[iter].parent;
    memcpy(&list->storage[iter], data, sizeof(TNode));
    ++list->size;
    return iter;
}

void NodListRemove(TNlist* list, unsigned int index) {
    if (list == NULL || index == 0 || list->size == 0) return;
    --list->size;
    list->storage[index].parent = list->freeHead;
    list->freeHead = index;
}

void NodListDestroy(TNlist* list) {
    if (list == NULL) return;
    free(list->storage);
    list->capacity = 0;
    list->size = 0;
    list->freeHead = 0;
    list->freeTail = 0;
}

static char _;

#define MAXBUF 30
static char BUFF[MAXBUF];   /* enough for int64_t*/

int _scanCHR(char* x) {
    _ = getchar_unlocked();
    if (_ == EOF) {
        return EOF;
    }
    (*x) = _;
    return 1;
}

int _scanSTR(char* x) {
    int len = 0;
    while (1) {
        _ = getchar_unlocked();
        if (_ == ' ' && len == 0) {
            continue;
        }
        if (_ == EOF && len == 0) {
            return EOF;
        }
        if (_ == '\n' || _ == EOF || _ == ' ') {
            break;
        }
        ++len;
        (*x++) = _;
    }
    (*x) = '\0';
    return len;
}

int _scanINT(int64_t* x) { 
    while (_ = getchar_unlocked(),_<=' ') {
        if (_==EOF) return EOF;
    }
    if (_==EOF) return EOF;
    int sign = (_=='-');
    if (sign) {
        _ = getchar_unlocked(); 
    } 
    *x = (_ - '0');
    while (_ = getchar_unlocked(),!((_<'0')|(_>'9'))) {
        *x = ((*x)<<3) + ((*x)<<1) + (_^48);
    }
    if (sign) *x = -(*x);
    return _;
}

int _scanUINT(uint64_t* x) { 
    while (_ = getchar_unlocked(),_<=' ') {
        if (_==EOF) return EOF;
    }
    if (_==EOF) return EOF;
    int sign = (_=='-');
    if (sign) {
        _ = getchar_unlocked(); 
    } 
    *x = (_ - '0');
    while (_ = getchar_unlocked(),!((_<'0')|(_>'9'))) {
        *x = ((*x)<<3) + ((*x)<<1) + (_^48);
    }
    if (_==EOF) return EOF;
    return _;
}

void _printINT(int64_t x) {
    if (x < 0) {
        x = -x;
        putchar_unlocked('-');
    }
    _printUINT((uint64_t)x);
}

void _printUINT(uint64_t x) {
    char* p = &BUFF[MAXBUF - 1];
    static char digs[] = "0123456789";
    do {
       *p-- = digs[x % 10];
       x /= 10;
    } while (x);
    while (++p != &BUFF[MAXBUF]) {
        putchar_unlocked(*p);
    }
}

void _printCHR(char c) {
    putchar_unlocked(c);
}

void _printSTR(const char* s) {
    for (int i = 0; s[i] > 0; ++i) {
        putchar_unlocked(s[i]);
    }
}

