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
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <avl.h>
#include "trie.h"

/* constants */

#define MAX 65536
#define N	512

#ifdef DEBUG
#define P(args...)
#else
#define P(args...) fprintf(stderr, args)
#endif

void process_line(const char *n);
void process_file(const char *n);

/* variables */

char buffer[MAX]; /* buffer to store everything */
char *p; /* pointer to free space in buffer */
int bs; /* free space size */

char *strings[N]; /* strings table for the macros/strings */
int strings_n = 0; /* number of strings */

struct trie_node *main_trie = NULL;

static const char stdin_name[] = "<<stdin>>";

/* TODO: fill this function. */
void do_usage(void)
{
} /* do_usage */

/**
 * process a file reading its contents into a string.
 * @param n name of the file to open or NULL to read from stdin.
 */
void process_line(const char *n)
{
	FILE *fin = (n != stdin_name)
		? fopen(n, "r")
		: stdin;

	if (!fin) {
		fprintf(stderr,
			D("file %s: %s(errno=%d) fin=%p; stdin=%p\n"),
			n, strerror(errno), errno, fin, stdin);
		exit(EXIT_FAILURE);
	} /* if */

	P(D("Processing [%s]: by lines BEGIN.\n"), n ? n : stdin_name);

	for (; (strings_n < N) && fgets(buffer, sizeof buffer, fin); strings_n++) {
		buffer[strlen(buffer)-1] = '\0'; /* eliminate the last \n char. */
		strings[strings_n] = strdup(buffer);
		P(D("  line[%5d] = [%s]\n"), strings_n, strings[strings_n]);
	} /* while */

	if (n) fclose(fin);
	P(D("Processing [%s]: by lines END.\n"), n ? n : stdin_name);
} /* process_line */

void process_file(const char *n)
{
	int in = (n != stdin_name)
		? open(n, O_RDONLY)
		: 0; /* stdin */
	int rd;

	if (in < 0) {
		fprintf(stderr,
			D("file %s: %s(errno=%d)\n"),
			n, strerror(errno), errno);
		exit(EXIT_FAILURE);
	} /* if */

	P(D("Processing [%s]: in block BEGIN.\n"), n);

	for (; (strings_n < N)
		&& ((rd = read(in, buffer, sizeof buffer - 1)) > 0);
		strings_n++)
	{
		buffer[rd] = 0;
		strings[strings_n] = strdup(buffer);
		P(D("  string[%5d] = [%s]\n"), strings_n, strings[strings_n]);
	} /* while */

	if (rd < 0) {
		fprintf(stderr,
			D("read %s: %s(errno=%d)\n"),
			n, strerror(errno), errno);
		exit(EXIT_FAILURE);
	} /* if */

	if (n) close(in);
	P(D("Processing [%s]: in block END\n"), n);
} /* process_file */

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
	void (*process)(const char *) = process_line;
	int i, mark;
    char *tab = "!#$%&'()*+-/<=>?@[]^_`{|}~,.:;";
	int n_passes = strlen(tab);

	while ((opt = getopt(argc, argv, "hflp:t:")) != EOF) {
		switch(opt) {
		case 'h':
		  do_usage(); exit(0);
		case 'f': process = process_file; break;
		case 'l': process = process_line; break;
		case 'p': n_passes = atol(optarg); break;
        case 't': {
                      int l = strlen(tab = optarg);
                      if (l < n_passes) n_passes = l;
                  } break;
		} /* switch */
	} /* while */

	argc -= optind; argv += optind;

	if (argc) {
		for (i = 0; i < argc; i++)
			process(argv[i]);
	} else	process(stdin_name);

	mark = strings_n;
	for(i = 0; i < n_passes; i++) {
		struct trie_node *root_trie, *max;
		int j;
		char *o;
		struct ref_buff *ref;
        int savings;

		P(D("PASS #0x%02x[%c]:\n"), i, tab[i]);
		assert(root_trie = new_trie());
		for (j = 0; j < strings_n; j++) {
			char *s;
			for (s = strings[j]; *s; s++)
				add_string(s, root_trie, strings[j]);
		} /* for */
		max = walk_trie(root_trie);

        if ((savings = weight_function(max)) <= 0) break;

		P(D("  max: [%.*s], l=%d, n=%d, savings=%d\n"),
			max->l, max->refs->b, max->l, max->n, savings);

		/* copy the string macro */
		o = strings[strings_n++] = malloc(max->l + 1);
		memcpy(o, max->refs->b, max->l);
		o += max->l;
		*o++ = '\0';

		/* substitute the strings */
		for (ref = max->refs; ref; ref = ref->nxt) {
			char *s = ref->b + max->l;
			char *t = ref->b;
			*t++ = tab[i];
			while (*s) *t++ = *s++;
			*t++ = '\0';
		} /* for */

		/* delete the trie as it is no more needed */
		del_trie(root_trie);
	} /* for */

    printf("char*t=\"");
    for (i = mark; i < strings_n; i++)
        printf("%c", tab[i-mark]);
    printf("\",*m[]={");
	for (i = mark; i < strings_n; i++)
		printf("%s/*%c*/\"%s\"",
                i == mark ? "" : ",",
                tab[i-mark],
                strings[i]);
    printf("},*s=");

	for(i = 0; i < mark; i++) {
		printf("%s\"%s\"", i ? "," : "", strings[i]);
	} /* for */
    printf("*p,*strchr();r(char*s){for(;*s;s++){"
            "p=strchr(t,*s);if(p)r(m[p-t]);else "
            "putchar(*s);}}main(){r(s);}\n");

} /* main */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
