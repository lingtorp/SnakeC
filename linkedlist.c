#include "linkedlist.h"
#include <stdlib.h> // calloc, free

LinkedList *linked_list_new() {
  LinkedList *list = calloc(sizeof(LinkedList), 1);
  return list;
}

void linked_list_dealloc(LinkedList *list) {
  Node *node;
  node = list->root_node;
  for (uint32_t i = 0; i < list->length - 1; i++) {
    Node *next_node = node->next_node;
    free(node->value);
    free(node);
    node = next_node;
  }
  free(list);
}

void linked_list_add_front(LinkedList *list, void *value) {
  Node *new_root = calloc(sizeof(Node), 1);
  new_root->value = value;
  new_root->next_node = list->root_node;
  list->root_node = new_root;
  list->length++;
}

void linked_list_add_back(LinkedList *list, void *value) {
  Node *node;
  node = list->root_node;
  for (uint32_t i = 0; i < list->length - 1;
       i++) { // Go to the back of the list
    node = node->next_node;
  }
  Node *new_node = calloc(sizeof(Node), 1);
  new_node->value = value;
  node->next_node = new_node;
  list->length++;
}

void *linked_list_get_value(LinkedList *list, uint32_t index) {
  if (index >= list->length) {
    return NULL;
  }
  Node *node;
  node = list->root_node;
  for (uint32_t i = 0; i <= index; i++) {
    node = node->next_node;
  }
  return node->value;
}

void *linked_list_get_first(LinkedList *list) { return list->root_node->value; }

void *linked_list_get_last(LinkedList *list) {
  Node *node;
  node = list->root_node;
  for (uint32_t i = 0; i < list->length; i++) {
    node = node->next_node;
  }
  return node->value;
}

void *linked_list_pop_first(LinkedList *list) {
  Node *old_root = list->root_node;
  list->root_node = old_root->next_node;
  void *old_root_value = old_root->value;
  free(old_root);
  list->length--;
  return old_root_value;
}

void *linked_list_pop_last(LinkedList *list) {
  if (list->root_node == NULL) {
    return NULL;
  }
  Node *last_node = list->root_node;
  for (uint32_t i = 0; i < list->length - 1; i++) {
    last_node = last_node->next_node;
  }
  void *last_node_value = last_node->value;
  free(last_node);
  list->length--;
  return last_node_value;
}

void *linked_list_next(LinkedList *list) {
  if (list->root_node == NULL) {
    return NULL;
  }
  Node *iterator_node = list->iterator_node;
  if (iterator_node == NULL) {
    iterator_node = list->root_node;       // Return the list root node's value
    list->iterator_node = list->root_node; // Reset to root node for the list
  } else {                                 // Walk forward with iterator node
    Node *next_node = iterator_node->next_node;
    list->iterator_node = next_node;
  }
  return iterator_node->value;
}

void linked_list_reset(LinkedList *list) {
  list->iterator_node = list->root_node;
}
