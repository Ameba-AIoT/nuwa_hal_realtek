/*
 * Copyright (c) 2026 Realtek Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Intrusive doubly linked list for the Realtek Ameba Bluetooth HAL.
 *
 * Each list node embeds a struct list_head. The owning struct is recovered
 * from a node pointer via list_entry() using offsetof() arithmetic.
 * The list head itself is an empty sentinel node: a non-empty list has
 * head.next != &head.
 */

#ifndef AMEBA_BT_HAL_DLIST_H
#define AMEBA_BT_HAL_DLIST_H

#include <stddef.h>

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *node)
{
	node->next = node;
	node->prev = node;
}

/* Insert @entry between @before and @after. */
static inline void __list_add(struct list_head *entry,
							  struct list_head *before,
							  struct list_head *after)
{
	after->prev  = entry;
	entry->next  = after;
	entry->prev  = before;
	before->next = entry;
}

static inline void list_add(struct list_head *entry, struct list_head *head)
{
	__list_add(entry, head, head->next);
}

static inline void list_add_tail(struct list_head *entry, struct list_head *head)
{
	__list_add(entry, head->prev, head);
}

static inline void __list_del(struct list_head *before, struct list_head *after)
{
	after->prev  = before;
	before->next = after;
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

static inline void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

static inline void list_move(struct list_head *entry, struct list_head *head)
{
	__list_del(entry->prev, entry->next);
	list_add(entry, head);
}

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

/* Recover the enclosing struct from a list node pointer. */
#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr) - offsetof(type, member)))

#define list_first_entry(head, type, member) \
	list_entry((head)->next, type, member)

#define list_for_each(pos, head) \
	for ((pos) = (head)->next; (pos) != (head); (pos) = (pos)->next)

#define list_for_each_safe(pos, tmp, head) \
	for ((pos) = (head)->next, (tmp) = (pos)->next; \
	     (pos) != (head); \
	     (pos) = (tmp), (tmp) = (pos)->next)

#define list_for_each_entry(pos, head, member, type) \
	for ((pos) = list_entry((head)->next, type, member); \
	     &(pos)->member != (head); \
	     (pos) = list_entry((pos)->member.next, type, member))

#define list_for_each_entry_safe(pos, tmp, head, member, type) \
	for ((pos) = list_entry((head)->next, type, member), \
	     (tmp) = list_entry((pos)->member.next, type, member); \
	     &(pos)->member != (head); \
	     (pos) = (tmp), \
	     (tmp) = list_entry((tmp)->member.next, type, member))

#endif /* AMEBA_BT_HAL_DLIST_H */
