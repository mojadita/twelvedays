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
#include <errno.h>
#include <assert.h>
#include <avl.h>
#include "trie.h"
#include "deco.h"

/* functions */
static struct trie_node *new_node(struct trie_node *prt, const int c);
static struct ref_buff *add_ref(struct trie_node *n, const byte *b, int ix);

struct trie_node *new_trie(void)
{
    return new_node(NULL, 0);
} /* new_trie */

/* constructor */
static struct trie_node *new_node(struct trie_node *prt, const int c)
{
    struct trie_node *res;

    assert(res = malloc(sizeof(struct trie_node)));

    res->c = c; /* the char/macro involved. */
    res->n = 0; /* it will be incremented when a reference is attached to it. */
    res->l = prt
        /* if there's a parent, the length is that of the parent plus this seq length */
        ? prt->l + (c & IS_ESCAPE_SEQ
            ? MACRO_SIZE
            : 1)
        /* if there's no parent, the length is zero (this is the root node) */
        : 0;
    res->prt = prt; /* previous node in the trie. */
    res->sub = new_avl_tree(NULL, NULL, NULL, NULL);
    res->refs = NULL;
    if (prt)
        avl_tree_put(prt->sub, (const void *)c, res);

    return res;
} /* new_node */

static struct ref_buff *add_ref(
        struct trie_node *n, /* the node where we add the reference */
        const byte *b, /* a pointer where we found this reference (length is in n->l) */
        int ix) /* string index where this reference belongs. */
{
    struct ref_buff *res;

    assert(res = malloc(sizeof(struct ref_buff)));

    res->b = b; /* buffer pointer, length is in trie node. */
    res->nxt = n->refs; /* stack insert */
    res->ix = ix;
    n->refs = res;
    n->n++; /* increment the number of references */

    return res;
} /* add_ref */

/**
 * this function adds a string to the trie, beginning at pos t.
 * @param s is the string to add.
 * @param l length of the string to add.
 * @param t is the trie node to add this string to.
 * @param data is the user data to add to the reference (normally
 *      the index of the string where this reference belongs)
 */
struct trie_node *add_string(const byte *s, int l, struct trie_node *t, int data)
{
    const byte *saved_s = s; 

    for (; l > 0; s++, l--) {
        struct trie_node *n;
        int c = 0;

        /* detect scape seq */
        if (*s == ESCAPE) {
            c |= IS_ESCAPE_SEQ;
            s++; l--;
        } /* if */
        c |= *s;

        assert(l > 0);

        /* search the new character in the trie */
        n = avl_tree_get(t->sub, (void *)c);

        if (!n) { /* not found */
            n = new_node(t, c); /* add it */
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
    /* t->refs == NULL */
    for (i = avl_tree_first(t->sub); i; i = avl_iterator_next(i)) {
        struct trie_node *p = avl_iterator_data(i);
        del_trie(p);
    } /* for */
    free_avl_tree(t->sub);
    free(t);
} /* del_trie */

/**
 * This function walks a trie calculating f(n) for each node n.
 * It selects the node with the largest value of f and returns
 * a reference to it as return value.
 * @param t the node to begin walk on.  Normally this parameter
 * is the root node of the trie.
 * @return the node with the largest value of f(n).
 */
struct trie_node *walk_trie(
        const struct trie_node *t,
        int (*weight_func)(const struct trie_node *))
{
    AVL_ITERATOR i;
    const struct trie_node *res = t;
    int fres = weight_func(res); /* initial value, our own one */
    
    for (i = avl_tree_first(t->sub); i; i = avl_iterator_next(i)) {
        struct trie_node *n = avl_iterator_data(i);
        int fn;
#if 0
        printf(D("[%.*s]: c=0x%02x, n=%d, l=%d\n"),
            t->l,
            t->refs
                ? t->refs->b
                : "",
            t->c, t->n, t->l);
#endif
        n = walk_trie(n, weight_func);
        fn = weight_func(n);
        if ((fn > 0) && (fn > fres)) {
            res = n;
            fres = fn;
        } /* if */
    } /* for */

    return (struct trie_node *)res;
} /* walk_trie */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
