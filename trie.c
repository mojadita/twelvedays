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
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <avl.h>

/* constants */
#define MAX 65536
#define N	512
#define D(X) __FILE__":%d:%s:" X, __LINE__, __func__

/* types */
struct trie_node {
	char				c;
	char				*b;
	int					l;
	int					n;
	struct trie_node	*prt;
	AVL_TREE			sub;
};

/* variables */

char buffer[MAX];
char *p;
int bs;

char *strings[N];
int strings_n = 0;

struct trie_node *main_trie = NULL;

/* functions */
void do_usage(void)
{
} /* do_usage */

struct trie_node *new_node(
	struct trie_node *prt,
	char c,
	char *b,
	int l,
	int n)
{
	struct trie_node *res;

	assert(res = malloc(sizeof(struct trie_node)));

	res->c = c;
	res->b = b;
	res->l = l; 
	res->n = n;
	res->prt = prt;
	res->sub = new_avl_tree(NULL, NULL, NULL, NULL);

	return res;
} /* new_node */

struct trie_node *add_string(char *s, struct trie_node *t)
{
	char *s2;
	for (s2 = s;*s;s++) {
		struct trie_node *n = avl_tree_get(t->sub, (void *)*s);
		if (!n) {
			n = new_node(t, *s, s2, t->l+1, 1);
			avl_tree_put(t->sub, (void *)*s, n);
		} /* if */
		t = n;
		if (t->b + t->l < s2) {
			t->b = s2;
			t->n++;
		} /* if */
	} /* for */
	return t;
} /* add_string */

void process(char *n)
{
	int in = 0; /* stdin */
	int rd;
	char *q;
	int l;
	if (n) {
		in = open(n, O_RDONLY);
		if (in < 0) {
			fprintf(stderr,
				D("file %s: %s(errno=%d)\n"),
				n, strerror(errno), errno);
			exit(EXIT_FAILURE);
		} /* if */
	} /* if */
	while ((rd = read(in, p, bs-1)) > 0) {
		strings[strings_n++] = p;
		p += rd;
		bs -= rd;
		*p++ = '\0'; bs--;
	} /* while */
	if (rd < 0) {
		fprintf(stderr,
			D("read %s: %s(errno=%d)\n"),
			n, strerror(errno), errno);
		exit(EXIT_FAILURE);
	} /* if */
	
	if (in > 0) close(in);

	for (q = strings[strings_n-1], l = strlen(q); l; l--, q++) {
		add_string(q, main_trie);
	} /* for */
	
} /* process */

int f(struct trie_node *n)
{
	return (n->l - 1)*(n->n - 1) - 2;
}

struct trie_node *recorre_trie(struct trie_node *t)
{
	AVL_ITERATOR i;
	struct trie_node *res = t;
	int fres = f(res);
	
	for (i = avl_tree_first(t->sub); i; i = avl_iterator_next(i)) {
		struct trie_node *n = avl_iterator_data(i);
		int fn;
		n = recorre_trie(n);
		if ((fn = f(n)) > fres) {
			res = n;
			fres = fn;
		}
	} /* for */
	return res;
} /* recorre_trie */

/* main program */
int main (int argc, char **argv)
{

	extern int optind;
	extern char *optarg;
	int opt;
	int i;
	struct trie_node *max;

	while ((opt = getopt(argc, argv, "h")) != EOF) {
		switch(opt) {
		case 'h':
		  do_usage(); exit(0);
		} /* switch */
	} /* while */

	argc -= optind; argv += optind;
	bs = sizeof buffer;
	p = buffer;

	main_trie = new_node(NULL, 0, NULL, 0, 0);

	if (argc) {
		int i;
		for (i = 0; i < argc; i++)
			process(argv[i]);
	} else	process(NULL);

	max = recorre_trie(main_trie);
	printf(D("max: [%.*s], l=%d, n=%d, f=%d\n"),
		max->l, max->b, max->l, max->n, f(max));

} /* main */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
