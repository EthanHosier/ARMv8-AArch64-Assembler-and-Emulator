#include "ArrayList.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

void print_ArrayList_elements(ArrayList *list) {
  assert(list->print_element != NULL);
  putc('[', stdout);
  for (int i = 0; i < list->size; i++) {
    (list->print_element)(get_ArrayList_element(list, i));
    if (i < list->size - 1) {
      printf(", ");
    }
  }
  printf("]\n");
}

static bool grow(ArrayList *list) {
  list->capacity = 2 * list->capacity;
  void **new = realloc(list->elements, list->capacity * sizeof(void *));
  if (new == NULL) {
    return false;
  }
  list->elements = new;
  return true;
}

bool add_ArrayList_element(ArrayList *list, void *element) {
  if (list->size == list->capacity) {
    if (!grow(list)) {
      return false;
    }
  }
  (list->elements)[list->size++] = element;
  return true;
}

void *remove_ArrayList_element(ArrayList *list) {
  assert(list->size != 0);
  list->size--;
  return (list->elements)[list->size];
}

void *get_ArrayList_element(ArrayList *list, int index) {
  assert(index < list->size);
  return (list->elements)[index];
}

ArrayList *
create_ArrayList(print_ArrayList_element print, free_ArrayList_element free) {
  ArrayList *list = malloc(sizeof(ArrayList));
  if (list == NULL) {
    return NULL;
  }
  void **elements = malloc(INITIAL_CAPACITY * sizeof(void *));
  if (elements == NULL) {
    return NULL;
  }
  list->elements = elements;
  list->capacity = INITIAL_CAPACITY;
  list->size = 0;
  list->free_element = free;
  list->print_element = print;
  return list;
}

void free_ArrayList(ArrayList *list) {
  if (list->free_element != NULL) {
    for (int i = 0; i < list->size; i++) {
      (list->free_element)(get_ArrayList_element(list, i));
    }
  }
  free(list->elements);
  free(list);
}
