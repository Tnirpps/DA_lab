#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IO functions */
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

#define MAX_LETTER_LEN 11

typedef struct TElement {
    char         key[MAX_LETTER_LEN];
    unsigned int wordNum;
    unsigned int lineNum;
} TElement;

typedef struct TVector {
    TElement*    value;
    size_t       capacity;
    size_t       size;
} TVector;

TVector          VectorCreate(size_t capacity);
TVector          VectorClone(const TVector *v);
void             VectorExpand(TVector *v);
void             VectorPushBack(TVector *v, TElement *e);
void             VectorPrint(const TVector *v);
void             VectorDestroy(TVector *v);


void ZFunction(const TVector *string, unsigned int *zVector);
int VectorReadline(TVector *string, unsigned int lineNum);

int main(int argc, char *argv[]) {
    TVector str = VectorCreate(10);
    unsigned int line = 0;
    VectorReadline(&str, line);
    int pattertLen = str.size;
    TElement sentinel = {"$", 0, 0};
    VectorPushBack(&str, &sentinel);
    while(VectorReadline(&str, ++line) != EOF);


    unsigned int *zVector = (unsigned int*) malloc(sizeof(TElement) * str.size);
    if (zVector == NULL) {
        fprintf(stderr, "ERROR: could not allocate memory for z-vector at line %d\n", __LINE__);
        exit(1);
    }
    ZFunction(&str, zVector);
    for (size_t i = pattertLen; i < str.size; ++i) {
        if(zVector[i] == pattertLen) {
            _printUINT(str.value[i].lineNum);
            _printSTR(", ");
            _printUINT(str.value[i].wordNum);
            _printCHR('\n');
        }
    }

    free(zVector);
    VectorDestroy(&str);
    return 0;
}

int VectorReadline(TVector *string, unsigned int lineNum) {
    if (string == NULL || string->value == NULL) {
        fprintf(stderr, "ERROR: could not read in uninitialized vector at line %d\n", __LINE__);
        exit(1);
    }
    static TElement cur = {0};
    char t;
    int len = 0;
    char lastChar = '\0';
    unsigned int wordNum = 1;
    int totlen = 0;
    while (1) {
        t = getchar_unlocked();
        if (t == '+') {
            continue;
        }
        if ((t == ' ' || t == '\t') && (len == 0 || lastChar == ' ' || lastChar == '\t' )) {
            lastChar = ' ';
            continue;
        }
        if (t == EOF && len == 0) {
            return EOF;
        }
        if (t == '\n' || t == EOF) {
            if (len == 0) {
                break;
            } else {
                if (totlen == 0) {
                    cur.key[0] = '0';
                    totlen = 1;
                }
            }
            cur.wordNum = wordNum;
            cur.lineNum = lineNum;
            cur.key[totlen] = '\0';
            VectorPushBack(string, &cur);
            break;
        }

        if ((t == ' ' || t == '\t') && len != 0) {

            if (totlen == 0) {
                cur.key[0] = '0';
                len = totlen = 1;;

            }
            cur.wordNum = wordNum;
            cur.lineNum = lineNum;
            cur.key[totlen] = '\0';
            VectorPushBack(string, &cur);
            lastChar = t;
            ++wordNum;
            totlen = 0;
            len = 0;
            continue;
        }
        if (t =='0' && totlen == 0) {
            ++len;
            lastChar = t;
            continue;
        } else {
            cur.key[totlen] = tolower(t);
            ++totlen;
            ++len;
        }
        lastChar = t; 
    }
    return len;
}



void ZFunction(const TVector *string, unsigned int *zVector) {
    if (zVector == NULL || string == NULL) {
        return;
    }
    size_t strLen = string->size;
    int l = 0, r = 0;
    memset(zVector, 0, strLen * sizeof(unsigned int));
    zVector[0] = 0;
    for (size_t i = 1; i < strLen; ++i) {
        if (i <= r) {
//          z[i] = min(z[i - l], r - i) :
            zVector[i] = (zVector[i - l] < r - i) ? zVector[i - l] : r - i;
        }
        while (i + zVector[i] < strLen && strcmp(string->value[i + zVector[i]].key, string->value[zVector[i]].key) == 0) {
            ++zVector[i];
        }
        if (i + zVector[i] > r) {
            l = i;
            r = i + zVector[i];
        }
    }
}


static char _;

#define MAX_BUF_SIZE 30
static char BUFF[MAX_BUF_SIZE];   /* enough for int64_t*/

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
    char* p = &BUFF[MAX_BUF_SIZE - 1];
    static char digs[] = "0123456789";
    do {
       *p-- = digs[x % 10];
       x /= 10;
    } while (x);
    while (++p != &BUFF[MAX_BUF_SIZE]) {
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

void VectorExpand(TVector* v) {
    if (v == NULL) {
        fprintf(stderr, "ERROR: could not expand NULL vector at line %d\n", __LINE__);
        exit(1);
    }
    
    if (v->capacity <= 0) v->capacity = 16;
    
    TElement* tmp = realloc(v->value, sizeof(TElement) * ((v->capacity)<<1));
    
    if (tmp == NULL) {
        free(v->value);
        fprintf(stderr, "ERROR: could not allocate memory to expand vector at line %d\n", __LINE__);
        exit(1);
    }

    v->value = tmp;
    v->capacity <<= 1;
}

TVector VectorCreate(size_t capacity) {
    TVector v;
    v.value = malloc(sizeof(TElement) * capacity);
    if (!v.value) {
        fprintf(stderr, "ERROR: could not allocate memory for new vector at line %d\n", __LINE__);
        exit(1);
    }
    
    v.capacity = capacity;
    v.size = 0;
    return v;
}

void VectorPushBack(TVector *v, TElement *e) {
    if (v == NULL) return;
    if (v->size + 2 >= v->capacity) {
        VectorExpand(v);
    }
    
    v->value[v->size] = *e;
    v->size++;
}

void VectorDestroy(TVector* v) {
    if (v == NULL) {
        return;
    }
    free(v->value);
    v->capacity = 0;
    v->size = 0;
}

