#include "macro.h"

struct List {
    int length;
    int size;
    void **items;
};


struct List *list_create(const int size);
void *list_append(struct List *list, void *p);
void *list_get(struct List *list, const int index);
void *list_pop(struct List *list, const int index);
void *list_insert(struct List *list, const int index, void *p);

int list_find(struct List *list, void *p);

void *list_remove(struct List *list, void *p);
void *list_add(struct List *list, void *p);

