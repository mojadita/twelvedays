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
	char				c; /* the character encoded in this node */
	char				*b; /* pointer to the beginning of this string */
	int					l; /* length of this trie. */
	int					n; /* number of times this string repeats */
	struct trie_node	*prt; /* parent */
	AVL_TREE			sub; /* avl tree to subnodes of this trie */
};

/* variables */

char buffer[MAX]; /* buffer to store everything */
char *p; /* pointer to free space in buffer */
int bs; /* free space size */

char *strings[N]; /* strings table for the macros/strings */
int strings_n = 0; /* number of strings */

struct trie_node *main_trie = NULL;

/* functions */

/* TODO: fill this function. */
void do_usage(void)
{
} /* do_usage */

/* constructor */
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

/**
 * this function adds a string to the trie, beginning at pos t.
 * @param s is the string to add.
 * @param t is the trie node to add this string to.
 */
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

/**
 * process a file reading its contents into a string.
 * @param n name of the file to open or NULL to read from stdin.
 */
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
int f(struct trie_node *n)
{
	return (n->l - 1)*(n->n - 1) - 2;
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
		n = walk_trie(n);
		if ((fn = f(n)) > fres) {
			res = n;
			fres = fn;
		} /* if */
	} /* for */
	return res;
} /* walk_trie */

/**
 * main program.  Processes all files and returns one string
 * for each file in strings[]. In the process, all strings are
 * used to construct the trie we use to get the substring with
 * great f value.
 */
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

	max = walk_trie(main_trie);
	printf(D("max: [%.*s], l=%d, n=%d, f=%d\n"),
		max->l, max->b, max->l, max->n, f(max));
} /* main */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */