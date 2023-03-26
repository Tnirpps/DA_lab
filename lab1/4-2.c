/* 26.03.2023 22:22 */
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef int     Vkey_t;
typedef struct  Vdata {
   unsigned int l;
   unsigned int s;
} Vdata_t;

typedef struct element_t {
    Vkey_t      key;
    Vdata_t     data;
} element_t;

typedef struct vector_t {
    element_t*  value;
    size_t      capacity;
    size_t      size;
} vector_t;

vector_t  VectorCreate(size_t capacity);
vector_t  VectorClone(const vector_t* v);
void      VectorExpand(vector_t* v);
void      VectorPushBack(vector_t* v, element_t e);
void      VectorDestroy(vector_t* v);
void      VectorCountSort(vector_t* v);
void      VectorRadixSort(vector_t* v);
void      VectorHeapSort(vector_t* v);
int       getchar_unlocked(void);
int       putchar_unlocked(int);
int       _scanCHR(char* x);
int       _scanSTR(char* x);
int       _scanINT(int64_t* x);
int       _scanUINT(uint64_t* x);
void      _printINT(int64_t x);
void      _printUINT(uint64_t x);
void      _printSTR(const char* s);
void      _printCHR(char c);

int main(int argc, char *argv[]) {
    vector_t v = VectorCreate((1<<16));
    char* string = malloc((int)1e8);
    if (string == NULL) {
        printf("ERROR: could not allocate memory for a big string\n");
        exit(1);
    }
    unsigned int ptr = 0;
    element_t x = {0};
    char str[2049 + 13] = {0};
    int strlen;
    while ((strlen=_scanSTR(str)) != EOF) {
        if (strlen == 0) continue;
        int day = 0;
        int month = 0;
        int year = 0;
        int iter = 0;
        while (str[iter] != '.') {
            day *= 10;
            day += (str[iter] - '0');
            ++iter;
        }
        ++iter;
        while (str[iter] != '.') {
            month *= 10;
            month += (str[iter] - '0');
            ++iter;
        }
        ++iter;
        while (str[iter] <= '9' && str[iter] >= '0') {
            year *= 10;
            year += (str[iter] - '0');
            ++iter;
        }
        memcpy(&string[ptr], str, strlen);
        x.data.l = ptr;
        x.data.s = strlen;
        x.key = day + month * 31 + year *(31 * 12);
        ptr += strlen;
        VectorPushBack(&v, x);
    }
    VectorRadixSort(&v);
    unsigned int s;
    unsigned int l;
    for (size_t i = 0; i < v.size; ++i) {
        s = v.value[i].data.s;
        l = v.value[i].data.l;
        for (size_t j = 0; j < s; ++j) {
            _printCHR(string[l + j]);
        }
        _printCHR('\n');
    }

    free(string);
    VectorDestroy(&v);
    return 0;
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
        if (_==EOF && len == 0) return EOF;
        if (_=='\n' || _==EOF) {
            break;
        }
        ++len;
        (*x++) = _;
    }
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

void VectorExpand(vector_t* v) {
    if (v == NULL) {
        fprintf(stderr, "ERROR: could not expand NULL vector\n");
        exit(1);
    }
    
    if (v->capacity <= 0) v->capacity = 16;
    
    element_t* tmp = realloc(v->value, sizeof(element_t) * ((v->capacity)<<1));
    
    if (tmp == NULL) {
        free(v->value);
        fprintf(stderr, "ERROR: could not allocate memory to expand vector\n");
        exit(1);
    }

    v->value = tmp;
    v->capacity <<= 1;
}

vector_t VectorCreate(size_t capacity) {
    vector_t v;
    v.value = malloc(sizeof(element_t) * capacity);
    if (!v.value) {
        fprintf(stderr, "ERROR: could not allocate memory for new vector\n");
        exit(1);
    }
    
    v.capacity = capacity;
    v.size = 0;
    return v;
}

vector_t VectorClone(const vector_t* v) {
    if (v == NULL) {
        fprintf(stderr, "ERROR: could not clone NULL vector\n");
        exit(1);
    }
    vector_t newVec = VectorCreate(v->capacity);
    memcpy(newVec.value, v->value, sizeof(element_t) * v->size);
    newVec.size = v->size;
    return newVec;
}

void VectorPushBack(vector_t* v, element_t e) {
    if (v == NULL) return;
    if (v->size + 2 >= v->capacity) {
        VectorExpand(v);
    }
    
    v->value[v->size] = e;
    v->size++;
}

void CountSort(vector_t* v, uint64_t digitMask, int shift) {
    if (v == NULL || v->value == NULL) {
        fprintf(stderr, "ERORR: could not sort NULL vector\n");
        exit(1);
    }

    int64_t min = (v->value[0].key & digitMask) >> shift;
    int64_t max = min;
    int64_t el;
    for (size_t i = 0; i < v->size; ++i) {
        el = (v->value[i].key & digitMask) >> shift; 
        if (el > max) {
            max = el;
        }
        if (el < min) {
            min = el;
        }
    }
    
    uint64_t size = (max - min) + 1;
    if (size == 1) {
        /* all elements are equal */
        return;
    }
    
    int* countVec = malloc(sizeof(Vkey_t) * (size_t)size);
    if (countVec == NULL) {
        fprintf(stderr, "ERROR: could not allocate memory for count vector\n");
        VectorDestroy(v);
        exit(1);
    }
    memset(countVec, 0, sizeof(Vkey_t) * (size_t)size);

    for (size_t i = 0; i < v->size; ++i) {
        countVec[((v->value[i].key & digitMask) >> shift) - min]++;
    }

    for (size_t i = 1; i < size; ++i) {
        countVec[i] += countVec[i - 1];
    }
    
    vector_t tmp = VectorClone(v);

    for (size_t i = v->size; i > 0; --i) {
        v->value[--countVec[((tmp.value[i - 1].key & digitMask) >> shift) - min]] = tmp.value[i - 1];
    }
    
    free(countVec);
    VectorDestroy(&tmp);
}

void VectorCountSort(vector_t* v) {
    CountSort(v, 0xFFFFFFFFFFFFFFFF, 0);
}

size_t log2_n(int n) {
    size_t res = 0;
    while (((uint64_t)1 << res) < n) {
        ++res;
    }
    return res;
}

void RadixSort(vector_t* v, int base) {
    size_t log2 = log2_n(base + 1);
    for (size_t shift = 0; shift < sizeof(Vkey_t); ++shift) {
        CountSort(v, ((uint64_t)base<<(shift * log2)), shift * log2);
    }
}

void VectorRadixSort(vector_t* v) {
    RadixSort(v, 0xFF);
}


void swap(element_t* a, element_t* b) {
    element_t c = *a;
    *a = *b;
    *b = c;
}

void push_down(vector_t* v, size_t index, size_t n) {
    if (index * 2 > n) return;
    if (index * 2 + 1 > n) {
        if (v->value[index*2].key < v->value[index].key) {
            swap(&v->value[index*2], &v->value[index]);
            push_down(v, index*2, n);
        }
    } else {
        if (v->value[index * 2 + 1].key <= v->value[index * 2].key 
                && v->value[index * 2 + 1].key < v->value[index].key) {
            swap(&v->value[index * 2 + 1], &v->value[index]);
            push_down(v, index * 2 + 1, n);
            return;
        }
        if (v->value[index * 2].key <= v->value[index * 2 + 1].key && v->value[index * 2].key < v->value[index].key) {
            swap(&v->value[index*2], &v->value[index]);
            push_down(v, index * 2, n);
        }
    }
}

void sieve_up(vector_t* v, size_t index) {
    if (index <= 1) return;
    if (v->value[index/2].key > v->value[index].key) {
        swap(&v->value[index/2], &v->value[index]);
        sieve_up(v, index/2);
    }

}

void sieve_down(vector_t* v, size_t n) {
   push_down(v, 1, n); 
}

void VectorHeapSort(vector_t* v) {
    if (v == NULL || v->value == NULL) {
        fprintf(stderr, "ERORR: could not sort NULL vector\n");
        exit(1);
    }
    
    size_t n = v->size;
    size_t c = n;
    size_t index = 1;
    vector_t heap = VectorCreate(n + 1);
    heap.size = n + 1;

    /* fill heap */
    while (c--) {
        heap.value[index] = v->value[c];
        sieve_up(&heap, index);
        ++index;
    }

    /* take heap Top and seive_down */
    c = n;
    while (c--) {
        v->value[n - c - 1] = heap.value[1];
        --index;
        heap.value[1] = heap.value[index];
        sieve_down(&heap, index);
    }

    VectorDestroy(&heap);
}

void VectorDestroy(vector_t* v) {
    if (v == NULL) {
        return;
    }
    free(v->value);
    v->capacity = 0;
    v->size = 0;
}

