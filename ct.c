/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Fri Dec 12 14:51:41 EET 2014
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

#define IN_CC_C

/* Standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <avl.h>

/* constants */
#define MAX		65536
#define D(X) __FILE__":%d:" X, __LINE__

/* types */
struct node {
	char *s;
	int l;
	int n;
};

/* prototypes */
struct node *new_node(char *s, int l);

/* variables */

char buffer[MAX];

/* functions */
struct node *new_node(char *s, int l)
{
	struct node *res;
	assert(res = malloc(sizeof(struct node)));
	res->s = s;
	res->l = l;
	res->n = 1;
	return res;
} /* new_node */

int node_fcomp(const void *n1, const void *n2)
{
	const struct node *p1 = n1;
	const struct node *p2 = n2;
	int i;
	for (i = 0; i < p1->l && i < p2->l; i++)
		if (p1->s[i] != p2->s[i]) break;
	/* if we have reached the end on both strings */
	if (i == p1->l && i == p2->l) {
		return 0;
	}
	/* if we have not reached the end on any string */
	if (i < p1->l && i < p2->l) {
		return p1->s[i] - p2->s[i];
	}
	/* we have reached the end on one and not in the other */
	return p1->l < p2->l
			? -1
			: +1;
} /* node_fcomp */

int node_fprint(const void *p, FILE *f)
{
	const struct node *n = p;
	return fprintf(f, "%.*s", n->l, n->s);
} /* node_fprint */

/* main program */
int main (int argc, char **argv)
{
	char *p;
	AVL_TREE t;
	AVL_ITERATOR it;
	int n = read(0, buffer, sizeof buffer - 1);
	if (n < 0) {
		fprintf(stderr,
			D("read:%s(errno=%d)\n"),
			strerror(errno), errno);
		exit(EXIT_FAILURE);
	} /* if */
	buffer[n] = 0;

	t = new_avl_tree(node_fcomp, NULL, NULL, node_fprint);
	for (p = buffer; *p; p++, n--) {
		int l;
		for (l = 1; l < n; l++) {
			struct node nod;
			nod.s = p;
			nod.l = *p ? l : 0;
			
			struct node *n = avl_tree_get(t, &nod);
			if (!n) {
				n = new_node(p, l);
				avl_tree_put(t, n, n);
			} /* if */
			/* if it doesn't share space with the previous
			 * add it. */
			if (n->s + n->l <= p)
				n->n++;
		}
	} /* for */	
	
	n = 0;
	for (it = avl_tree_first(t); it; it = avl_iterator_next(it)) {
		struct node *N = avl_iterator_data(it);
		int f = (N->l - 1)*(N->n - 1);
		
		if (f >= n) {
			printf("l=%04d n=%04d f=%4d: [%.*s]\n",
					N->l, N->n, f, N->l, N->s);
			n = f;
		} /* if */
	} /* for */
} /* main */
/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
