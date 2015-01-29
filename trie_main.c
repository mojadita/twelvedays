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
#include <stdarg.h>
#include <avl.h>
#include "trie.h"
#include "deco.h"

/* constants */
#define MAX_PASSES	253

#define FLAG_DEBUG	(1 << 0)
#define FLAG_BINARY	(1 << 1)

#define MAX (1<<24)
#define N	512
#define N_PER_ROW	16
#define TYPE_BYTE	"UWord8"
#define TYPE_INT	"UWord16"

void process_file(const char *n);

/* variables */

unsigned char buffer[MAX];	/* buffer to store everything */
unsigned char *p;			/* pointer to free space in buffer */
int bs = 0;					/* free space size */

char *strings[N];			/* strings table for the macros/strings */
int strings_sz[N];			/* lengths of strings. */
int strings_n = 0;			/* number of strings */
int flags = 0;				/* option flags */

struct trie_node *main_trie = NULL;

static const char stdin_name[] = "<<stdin>>";

/* TODO: fill this function. */
void do_usage(void)
{
} /* do_usage */

void process_file(const char *n)
{
	FILE *in = stdin;
	int saved_bs = bs;
	int c;

	if (n != stdin_name) {
		in = fopen(n, "rb");
		if (!in) {
			fprintf(stderr,
				D("%s: %s (errno = %d)\n"),
				n, strerror(errno), errno);
			exit(EXIT_FAILURE);
		} /* if */
	} /* if */

#if DEBUG
	fprintf(stderr,
		D("Processing [%s]: BEGIN.\n"),
		n);
#endif

	strings[strings_n] = buffer + bs;
	while((c = fgetc(in)) != EOF) {
		buffer[bs++] = c;
		if(c == ESCAPE) {
			buffer[bs++] = ESCOUT;
		} /* if */
	} /* while */

	strings_sz[strings_n] = bs - saved_bs;
	strings_n++;

	if (n != stdin_name) fclose(in);

#if DEBUG
	printf(stderr,
		D("Processing [%s]: END\n"),
		n);
#endif
} /* process_file */

#define PRINTOUT(X) \
	int fprint_out_##X( \
		FILE *f, \
		const unsigned X *buffer, \
		int buffer_sz) \
	{ \
		int res = 0; \
		int i; \
		for (i = 0; i < buffer_sz; i++) { \
			if (!(i % N_PER_ROW)) { \
				res += fprintf(f, "\n    /* %6d */ ", i); \
			} /* if */ \
			res += fprintf(f, "0x%02x,", buffer[i]); \
		} /* for */ \
		res += fprintf(f, "\n"); \
		return res; \
	}

PRINTOUT(char) /* fprint_out_char() */
PRINTOUT(int) /* fprint_out_int() */

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
	int i, mark;
	int n_passes = MAX_PASSES;
	FILE *out = stdout;

	while ((opt = getopt(argc, argv, "dhp:o:")) != EOF) {
		switch(opt) {
		case 'h': do_usage(); exit(0);
		case 'p': n_passes = atol(optarg); break;
		case 'd': flags |= FLAG_DEBUG; break;
		case 'o': out = fopen(optarg, "wb"); break;
		} /* switch */
	} /* while */

	argc -= optind; argv += optind;

	if (argc) {
		for (i = 0; i < argc; i++)
			process_file(argv[i]);
	} else	process_file(stdin_name);

	mark = strings_n;

	/* print the strings */
	{	int i;
		for (i = 0; i < strings_n; i++) {
			if (i == mark)
				printf("MARK\n");
			fprintbuf(stderr,
				strings_sz[i],
				strings[i],
				"strings[%d], len=%d",
				i, strings_sz[i]);
		} /* for */
	} /* block */

	for(i = 0; i < n_passes; i++) {
		struct trie_node *root_trie, *max;
		int j;
		char *o;
		struct ref_buff *ref;

		if (flags & FLAG_DEBUG) {
			fprintf(stderr, D("PASS #%d:\n"), i);
		} /* if */

		/* INITIALIZE THE TRIE */
		assert(root_trie = new_trie());
		for (j = 0; j < strings_n; j++) {
			char *s;
			int l;
			static char *progress[] = {
				"\\", "|", "/", "-",
			};
			for (s = strings[j], l = strings_sz[j]; l; s++, l--) {
				add_string(s, l, root_trie, j);
			} /* for */
			fprintf(stderr, "\b%s", progress[j % 4]);
		} /* for */

		/* SEARCH FOR THE MOST EFFICIENT MACRO SUBSTITUTION */
		max = walk_trie(root_trie);

		/* IF NOT FOUND, FINISH */
		if (max == root_trie) break;

		if (flags & FLAG_DEBUG) {
			/* WRITE THE MACRO FOUND */
			fprintbuf(stderr,
				max->l, max->refs->b,
				D("len=%d, nrep=%d"),
				max->l, max->n);
		} /* if */

		/* copy the string macro as a new string. */
		strings[strings_n] = buffer + bs;
		memcpy(strings[strings_n], max->refs->b, max->l);
		bs += max->l;
		strings_sz[strings_n] = max->l;
		strings_n++;

		/* substitute the strings as macro calls */
		for (ref = max->refs; ref; ref = ref->nxt) {
			const char *src = ref->b + max->l;
			char *tgt = (char *)ref->b;
			int ix = ref->ix;
			int n = strings_sz[ix] - (src - strings[ix]);
			strings_sz[ix] -= max->l - MACRO_SIZE;
			*tgt++ = ESCAPE;
			*tgt++ = i;
			if (n < 0) {
				fprintbuf(stderr,
					strings_sz[ix],
					strings[ix],
					"strings[%d]: pos=0x%x, len=%d->0x%x, n=%d",
					ix,
					tgt - strings[ix],
					max->l,
					(tgt - strings[ix]) + max->l,
					n);
				fflush(stderr);
				continue;
				abort();
			} /* if */
			while (n--) *tgt++ = *src++;
		} /* for */

		/* delete the trie as it is no more needed */
		del_trie(root_trie);
		/* print the strings */
		{	int i;
			for (i = 0; i < strings_n; i++) {
				fprintbuf(stderr,
					strings_sz[i],
					strings[i],
					"strings[%d], len=%d",
					i, strings_sz[i]);
			} /* for */
		} /* block */
	} /* for */

	/* PRINT THE MACROS */
	if (!(flags & FLAG_BINARY)) {
		fprintf(out,
			"const " TYPE_INT " macros_n = %d;\n"
			"const " TYPE_INT " macros_sz[] = {",
			strings_n - mark);
		fprint_out_int(out,
			strings_sz + mark,
			strings_n - mark);
		fprintf(out,
			"\n}; /* macros_sz */\n\n"
			"const " TYPE_BYTE " macros[] = {");
	} /* if */

	for (i = mark; i < strings_n; i++) {
		if (flags & FLAG_DEBUG) {
			fprintbuf(stderr, strings_sz[i], strings[i],
				D("macros[0x%02x, 0x%02x] ="),
				ESCAPE, i-mark);
		} /* if */
		fprintf(out, "\n    /* macro #%d */", i - mark);
		fprint_out_char(out, strings[i], strings_sz[i]);
	} /* for */

	fprintf(out,
		"\n}; /* macros */\n"
		"\n"
		"const " TYPE_INT " strings_n = %d;\n"
		"const " TYPE_INT " strings_sz [] = {",
		mark);
	fprint_out_int(out, strings_sz, mark);
	fprintf(out,
		"\n}; /* strings_sz */\n"
		"const " TYPE_BYTE " strings [] = {");

	for(i = 0; i < mark; i++) {
		if (flags & FLAG_DEBUG) {
			fprintbuf(stderr, strings_sz[i], strings[i],
				D("strings[%d] ="), i);
		} /* if */

		fprintf(out, "\n    /* string #%d */", i);
		fprint_out_char(out, strings[i], strings_sz[i]);
		fprintf(out, "\n}; /* strings */\n\n");
	} /* for */

	if (out != stdout) fclose(out);

	return EXIT_SUCCESS;

} /* main */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
