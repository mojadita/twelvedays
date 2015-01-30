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

static const unsigned int MAX_STR_LEN = (1 << 9);

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
	unsigned int char_count = 0;

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
		++char_count;
		if(c == ESCAPE) {
			buffer[bs++] = ESCOUT;
			++char_count;
		} /* if */

		if (char_count >= MAX_STR_LEN) {
		    strings_sz[strings_n] = bs - saved_bs;
		    ++strings_n;

		    saved_bs = bs;
		    strings[strings_n] = buffer + bs;
		    char_count -= MAX_STR_LEN;
		}
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

int vprint_out(FILE *f,
	const unsigned char *buffer,
	int buffer_sz,
	const char *fmt,
	va_list p)
{
	int res = 0;
	int i;
	res += vfprintf(f, fmt, p);
	for (i = 0; i < buffer_sz; i++) {
		if (!(i % N_PER_ROW)) {
			res += fprintf(f,
				"\n    /* %4d */ ",
				i);
		} /* if */
		res += fprintf(f, "0x%02x,", buffer[i]);
	} /* for */
	res += fprintf(f, "\n");
	return res;
} /* print_out */

int print_out(FILE *f,
	const unsigned char *buffer,
	int buffer_sz,
	const char *fmt,
	...)
{
	va_list p;
	int res;

	va_start(p, fmt);
	res = vprint_out(f, buffer, buffer_sz, fmt, p);
	va_end(p);

	return res;
} /* print_out */


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
	static const char eol[] = { ESCAPE, EOS };
	static const int eol_sz = sizeof eol;
	FILE *out = stdout;

	while ((opt = getopt(argc, argv, "bdhp:o:")) != EOF) {
		switch(opt) {
		case 'h': do_usage(); exit(0);
		case 'p': n_passes = atol(optarg); break;
		case 'd': flags |= FLAG_DEBUG; break;
		case 'b': flags |= FLAG_BINARY; break;
		case 'o': out = fopen(optarg, "wb"); break;
		} /* switch */
	} /* while */

	argc -= optind; argv += optind;

	if (argc) {
		for (i = 0; i < argc; i++)
			process_file(argv[i]);
	} else	process_file(stdin_name);

	mark = strings_n;

	for(i = 0; i < n_passes; i++) {
		struct trie_node *root_trie, *max;
		int j;
		char *o;
		struct ref_buff *ref;

		if (flags & FLAG_DEBUG) {
			fprintf(stderr,
				D("PASS #%d:\n"),
				i);
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
			fprintf(stderr,
				"\r%s %d/%d",
				progress[j % (sizeof progress/sizeof progress[0])],
				j, strings_n);
			fflush(stderr);
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
			char *src = ref->b + max->l;
			char *tgt = ref->b;
			int ix = ref->ix;
			int n = strings_sz[ix] - (src - strings[ix]);
			strings_sz[ix] -= max->l - MACRO_SIZE;
			*tgt++ = ESCAPE;
			*tgt++ = i;
			while (n--) *tgt++ = *src++;
		} /* for */

		/* delete the trie as it is no more needed */
		del_trie(root_trie);
	} /* for */

	/* PRINT THE MACROS */
	if (!(flags & FLAG_BINARY)) {
		fprintf(out,
			"const " TYPE_INT " macros_n = %d;\n"
			"const " TYPE_INT " macros_sz[] = {",
			strings_n - mark);
		for (i = mark; i < strings_n; i++) {
			if ((i - mark) % N_PER_ROW == 0)
				fprintf(out, "\n    /* %6d */ ",
					i - mark);
			fprintf(out, "%d, ", strings_sz[i]);
		} /* for */
		fprintf(out,
			"\n};\n\n"
			"const " TYPE_BYTE " macros[] = {");
	} /* if */

	for (i = mark; i < strings_n; i++) {
		if (flags & FLAG_DEBUG) {
			fprintbuf(stderr,
				strings_sz[i],
				strings[i],
				D("macros[0x%02x, 0x%02x] ="),
				ESCAPE, i-mark);
		} /* if */
		if (flags & FLAG_BINARY) {
			fwrite(
				strings[i],
				strings_sz[i],
				sizeof(char),
				out);
			fwrite(eol, eol_sz, sizeof(char), out);
		} else {
			print_out(out,
				strings[i],
				strings_sz[i],
				"\n    /* macro #%d */", i - mark);
		} /* if */
	} /* for */

	if (flags & FLAG_BINARY) {
		fwrite(eol, eol_sz, sizeof(char), out);
	} else {
		fprintf(out,
			"\n};\n\n");
	} /* if */


	for(i = 0; i < mark; i++) {
		if (flags & FLAG_DEBUG) {
			fprintbuf(stderr,
				strings_sz[i],
				strings[i],
				D("strings[%d] ="), i);
		} /* if */

		if (flags & FLAG_BINARY) {
			if (i) fwrite(eol, eol_sz, sizeof(char), out);
			fwrite(
				strings[i],
				strings_sz[i],
				sizeof(char),
				out);
		} else {
			fprintf(out,
				"const " TYPE_INT " strings_n = %d;\n"
				"const " TYPE_INT " strings_sz[] = {",
				mark);
			for (i = 0; i < mark; i++) {
				if (i % N_PER_ROW == 0)
					fprintf(out, "\n    /* %6d */ ", i);
				fprintf(out, "%d, ", strings_sz[i]);
			} /* for */
			fprintf(out,
				"\n};\n\n"
				"const " TYPE_BYTE " strings[] = {");
			for (i = 0; i < mark; i++) {
				print_out(out,
					strings[i],
					strings_sz[i],
					"\n    /* string #%d */", i);
			} /* for */
			fprintf(out,
				"\n};\n\n");
		} /* if */
	} /* for */

	if (out != stdout) fclose(out);

	return EXIT_SUCCESS;

} /* main */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
