#pragma once
#include <stdint.h>
/*
 * Single linked list implementation.
 */

typedef struct Node Node;

struct Node {
  void *value;
  Node *next_node;
};

typedef struct {
  Node *root_node;
  uint32_t length;
  Node *iterator_node;
} LinkedList;

// Creates a new linked list on the heap and return the pointer to it.
LinkedList *linked_list_new();
// Goes through the list and frees the memory for every nodes value pointer and
// itself.
void linked_list_dealloc(LinkedList *list);

// Adds the value in the front of the list.
void linked_list_add_front(LinkedList *list, void *value);
// Adds the vlaue in the back of the list.
void linked_list_add_back(LinkedList *list, void *value);
// Returns the value from the n:th node in the list.
void *linked_list_get_value(LinkedList *list, uint32_t index);
// Returns the first value in the list w/o removing it.
void *linked_list_get_first(LinkedList *list);
// Returns the last value in the list w/o removing.
void *linked_list_get_last(LinkedList *list);
// Returns the first value in the list and removes it.
void *linked_list_pop_first(LinkedList *list);
// Return the last value in the list and removes it.
void *linked_list_pop_last(LinkedList *list);
// Returns the next value or null if the list is empty. Wraps from end to
// beginning.
void *linked_list_next(LinkedList *list);
// Resets the iterator in the list to point at the beginning.
void linked_list_reset(LinkedList *list);
