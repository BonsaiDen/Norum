#include "list.h"

#include "marco.h"

#include <stdlib.h>


struct List *list_create(const int size) {
    struct List *list = malloc(sizeof(struct List));
    list->size = size;
    list->length = 0;
    list->items = calloc(sizeof(void*), list->size);
    return list;
}


// O(1) Operations -------------------------------------------------------------
void *list_append(struct List *list, void *p) {
    if (unlikely(list->length > list->size - 1)) {
        list->size *= 2;
        
        void **tmp;
        if (!(tmp = realloc(list->items, sizeof(void*) * list->size))) {
            return NULL;
        }
        list->items = tmp;
    }
    
    list->items[list->length] = p;
    list->length += 1;
    return p;
}

void *list_get(struct List *list, const int o_index) {
    const int index = o_index < 0 ? list->length + o_index : o_index;
    if (unlikely(index < 0 || index > list->length - 1 || !list->items[index])) {
        return NULL;
    
    } else {
        return list->items[index];
    }
}


// O(1) / O(n) Operations ------------------------------------------------------
void *list_insert(struct List *list, const int o_index, void *p) {
    const int index = o_index < 0 ? list->length + o_index : o_index;
    if (unlikely(index < 0 || index > list->length)) {
        return NULL;
    
    } else {
        if (unlikely(list->length > list->size - 1)) {
            list->size *= 2;
            
            void **tmp;
            if (!(tmp = realloc(list->items, sizeof(void*) * list->size))) {
                return NULL;
            }
            list->items = tmp;
        }
        
        for(int i = list->length; i > index; i--) {
            list->items[i] = list->items[i - 1];
        }
        
        list->items[index] = p;
        list->length += 1;
        return p;
    }
}

void *list_pop(struct List *list, const int o_index) {
    const int index = o_index < 0 ? list->length + o_index : o_index;
    if (unlikely(index < 0 || index > list->length - 1 || !list->items[index])) {
        return NULL;
    
    } else {
        void *item = list->items[index];
        list->items[index] = NULL;
        
        for(int i = index, l = list->length; i < l; i++) {
            list->items[i] = list->items[i + 1];
        }
        
        list->items[list->length] = NULL;
        list->length -= 1;
        return item;
    }
}


// O(n) Operations -------------------------------------------------------------
int list_find(struct List *list, void *p) {
    for(int i = 0, l = list->length; i < l; i++) {
        if (list->items[i] == p) {
            return i;
        }
    }
    return -1;
}

void *list_add(struct List *list, void *p) {
    if (list_find(list, p) == -1) {
        return list_append(list, p);
    
    } else {
        return NULL;
    }
}

void *list_remove(struct List *list, void *p) {
    const int index = list_find(list, p);
    if (index != -1) {
        return list_pop(list, index);
    
    } else {
        return NULL;
    }
}

