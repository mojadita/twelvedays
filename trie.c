/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Fri Dec 12 23:20:54 EET 2014
 *
 * Disclaimer:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define IN_TRIE_C

/* Standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <avl.h>
#include "trie.h"

/* functions */
static struct trie_node *new_node(struct trie_node *prt, const char c);
static struct ref_buff *add_ref(struct trie_node *n, const char *b, const void *d);

struct trie_node *new_trie(void)
{
	return new_node(NULL, 0);
} /* new_trie */

/* constructor */
static struct trie_node *new_node(struct trie_node *prt, const char c)
{
	struct trie_node *res;

	assert(res = malloc(sizeof(struct trie_node)));

	res->c = c;
	res->n = 0; /* it will be incremented when a reference is attached to it. */
	res->l = prt ? prt->l + 1 : 0;
	res->prt = prt; /* previous node in the trie. */
	res->sub = new_avl_tree(NULL, NULL, NULL, NULL);
	res->refs = NULL;
	if (prt) avl_tree_put(prt->sub, (const void *)c, res);

	return res;
} /* new_node */

static struct ref_buff *add_ref(struct trie_node *n, const char *b, const void *d)
{
	struct ref_buff *res;

	assert(res = malloc(sizeof(struct ref_buff)));

	res->b = b; /* buffer pointer, length is in trie node. */
	res->nxt = n->refs; /* stack insert */
	res->d = d;
	n->refs = res;
	n->n++; /* increment the number of references */

	return res;
} /* add_ref */

/**
 * this function adds a string to the trie, beginning at pos t.
 * @param s is the string to add.
 * @param t is the trie node to add this string to.
 */
struct trie_node *add_string(const char *s, struct trie_node *t, const void *data)
{
	const char *saved_s = s; 

	for (;*s;s++) {
		/* search the new character in the trie */
		struct trie_node *n = avl_tree_get(t->sub, (void *)*s);

		if (!n) { /* not found */
			n = new_node(t, *s); /* add it */
		} /* if */

		/* add the reference only if it doesn't overlap the previous */
		if (   !n->refs /* no previous reference */
			|| (n->refs->b + n->l <= saved_s)) /* no overlap */
		{
			add_ref(n, saved_s, data); 
		} /* if */
		t = n;
	} /* for */

	return t;
} /* add_string */

void del_trie(struct trie_node *t)
{
	AVL_ITERATOR i;
	while (t->refs) {
		struct ref_buff *p = t->refs;
		t->refs = p->nxt;
		free(p);
	} /* while */
	for (i = avl_tree_first(t->sub); i; i = avl_iterator_next(i)) {
		struct trie_node *p = avl_iterator_data(i);
		del_trie(p);
	} /* for */
	free_avl_tree(t->sub);
	free(t);
} /* del_trie */

/**
 * The next function gives, for a given trie_node *n, the
 * number of characters saved by using this node as a macro.
 * the number of characters saved is the length of this macro (n->l)
 * by the number of times it appears in the code (n->n) minus
 * one character per each time it appears (-n->n) minus the
 * table entry (-n->l - 1) (one char to identify the macro used)
 * so, in total we have (n->l)*(n->n) -(n->n) -(n->l) - 1 ==
 * (n->l - 1)*(n->n - 1) - 2.
 * @param n node to calculate f for.
 * @return the value calculated as above.
 */
static int f(struct trie_node *n)
{
	return n
		? (n->l - 1)*(n->n - 1) - 2
		: -1;
} /* f */

/**
 * This function walks a trie calculating f(n) for each node n.
 * It selects the node with the largest value of f and returns
 * a reference to it as return value.
 * @param t the node to begin walk on.  Normally this parameter
 * is the root node of the trie.
 * @return the node with the largest value of f(n).
 */
struct trie_node *walk_trie(struct trie_node *t)
{
	AVL_ITERATOR i;
	struct trie_node *res = t;
	int fres = f(res);
	
	for (i = avl_tree_first(t->sub); i; i = avl_iterator_next(i)) {
		struct trie_node *n = avl_iterator_data(i);
		int fn;
#if 0
		printf(D("[%.*s]: c=%c, n=%d, l=%d\n"),
			t->l,
			t->refs
				? t->refs->b
				: "",
			t->c, t->n, t->l);
#endif
		n = walk_trie(n);
		if ((fn = f(n)) > fres) {
			res = n;
			fres = fn;
		} /* if */
	} /* for */

	return res;
} /* walk_trie */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
