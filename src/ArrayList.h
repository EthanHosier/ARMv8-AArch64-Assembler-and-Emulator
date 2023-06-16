#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdbool.h>
#define INITIAL_CAPACITY 10

typedef void (*print_ArrayList_element)(void *);

typedef void (*free_ArrayList_element)(void *);

typedef struct {
  void **elements;
  int size;
  int capacity;
  print_ArrayList_element print_element;
  free_ArrayList_element free_element;
} ArrayList;

extern void print_ArrayList_elements(ArrayList *);

extern void free_ArrayList(void *);

extern void add_ArrayList_element(ArrayList *list, void *element);

extern void *remove_ArrayList_element(ArrayList *);

extern ArrayList *
create_ArrayList(print_ArrayList_element, free_ArrayList_element);

extern void *get_ArrayList_element(ArrayList *list, int index);

#endif
