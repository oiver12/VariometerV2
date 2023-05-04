#ifndef __UTIL_H__
#define __UTIL_H__

#include "stdint.h"

#define PI          3.1415926535897932384626433832795

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

void INIT_LIST_HEAD(struct list_head *list);

/**
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 * @new: new entry to be added
 * @head: list head to add it after
 *
 */
void list_add_head(struct list_head *new, struct list_head *head);

/**
 * Insert a new entry at the end
 */
void list_add_tail(struct list_head *new, struct list_head *head);

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
void list_insert(struct list_head *new, struct list_head *prev, struct list_head *next);

int16_t     GET_BEARING(double lat1, double lon1, double lat2, double lon2);
double      GET_DISTANCE(double lat1, double lon1, double lat2, double lon2);


#endif
