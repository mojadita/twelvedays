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

/* constants */
#define MAX_PASSES  254

#define FLAG_DEBUG      (1 << 0)
#define FLAG_BINARY     (1 << 1)
#define FLAG_PROGRESS   (1 << 2)

#define MAX (1<<24)
#define N   512
#define N_PER_ROW   16
#define TYPE_BYTE   "const UWord8"
#define TYPE_INT    "const UWord16"

static void process_file(const char *n);

/* variables */

static byte buffer[MAX];            /* buffer to store everything */
//unsigned char *p;                 /* pointer to free space in buffer */
static int bs = 0;                  /* free space size */

static byte *strings[N];            /* strings table for the macros/strings */
static int strings_sz[N];           /* lengths of strings. */
static int strings_n = 0;           /* number of strings */
static int mark = 0;
static int flags = 0;               /* option flags */
static int chunk_size = 0;

static struct trie_node *main_trie = NULL;

static const char stdin_name[] = "<<stdin>>";

/* TODO: fill this function. */
static void do_usage(void)
{
} /* do_usage */

static void process_file(const char *n)
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

    if (flags & FLAG_DEBUG)
        fprintf(stderr, D("Processing [%s]: BEGIN.\n"), n);

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

    if (flags & FLAG_DEBUG)
        fprintf(stderr, D("Processing [%s]: END\n"), n);
} /* process_file */

#define PRINTOUT(X) \
    static int fprint_out_##X( \
        FILE *f, \
        const X *buffer, \
        int buffer_sz) \
    { \
        int res = 0; \
        int i; \
        for (i = 0; i < buffer_sz; i++) { \
            if (!(i % N_PER_ROW)) { \
                res += fprintf(f, "\n    /* %06x */ ", i); \
            } /* if */ \
            res += fprintf(f, "0x%02x,", buffer[i]); \
        } /* for */ \
        res += fprintf(f, "\n"); \
        return res; \
    }

PRINTOUT(byte) /* fprint_out_byte() */
PRINTOUT(int) /* fprint_out_int() */

/**
 * The next function gives, for a given trie_node *n, the
 * number of characters saved by using this node as a macro.
 * the number of characters saved is the length of this macro (n->l)
 * by the number of times it appears in the code (n->n) minus
 * MACRO_SIZE characters per each time it appears (-n->n) minus the
 * table entry (-n->l - MACRO_SIZE) (one char to identify the macro used)
 * so, in total we have 
 * (n->l - MACRO_SIZE)*(n->n - 1) - MACRO_SIZE.
 * @param n node to calculate f for.
 * @return the value calculated as above.
 */
static int savings_calculation(const struct trie_node *n)
{
    return n
        ? (n->l - MACRO_SIZE)*(n->n - 1) - MACRO_SIZE
        : -1;
} /* f */

static int print_strings()
{
    int i, res = 0;

    res += fprintf(stderr, D("STATE BEGIN:\n"));
    for (i = 0; i < strings_n; i++) {
        if (i == mark) fprintf(stderr, D("MACROS:\n"));
        res += fprintbuf(stderr,
            strings_sz[i],
            strings[i],
            "strings[%d], len=%d",
            i, strings_sz[i]);
    } /* for */
    res += fprintf(stderr, D("STATE END.\n"));

    return res;
} /* print_strings */

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
    int n_passes = MAX_PASSES;
    FILE *out = stdout;

    while ((opt = getopt(argc, argv, "dhPp:o:")) != EOF) {
        switch(opt) {
        case 'h': do_usage(); exit(0);
        case 'P': flags |= FLAG_PROGRESS; break;
        case 'p': n_passes = atol(optarg); break;
        case 'd': flags |= FLAG_DEBUG; break;
        case 'o': out = fopen(optarg, "wb"); break;
        } /* switch */
    } /* while */

    argc -= optind; argv += optind;

    if (argc) {
        for (i = 0; i < argc; i++)
            process_file(argv[i]);
    } else  process_file(stdin_name);

    mark = strings_n;

    /* print the strings in the begining */
    if (flags & FLAG_DEBUG)
        print_strings();

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
            const byte *s;
            int l;
            static char *progress[] = {
                "\\", "|", "/", "-",
            };
            for (s = strings[j], l = strings_sz[j]; l; s++, l--) {
                add_string(s, l, root_trie, j);
            } /* for */
            if (flags & FLAG_PROGRESS)
                fprintf(stderr,
                    "\b%s %d/%d",
                    progress[j % 4], j+1, strings_n);
        } /* for */

        /* SEARCH FOR THE MOST EFFICIENT MACRO SUBSTITUTION */
        max = walk_trie(root_trie, savings_calculation);

        /* IF NOT FOUND, FINISH */
        if (max == root_trie) {
            if (flags & FLAG_DEBUG) {
                fprintf(stderr,
                    D("MACRO NOT FOUND, FINISHING\n"));
            } /* if */
            break;
        } /* if */

        if (flags & FLAG_DEBUG) {
            struct ref_buff *ref;
            /* WRITE THE MACRO FOUND */
            fprintbuf(stderr,
                max->l, max->refs->b,
                D("MACRO FOUND: len=%d, nrep=%d, savings=%d"),
                max->l, max->n, savings_calculation(max));
        } /* if */

        /* copy the string macro as a new string. */
        strings[strings_n] = buffer + bs;
        memcpy(strings[strings_n], max->refs->b, max->l);
        bs += max->l;
        strings_sz[strings_n] = max->l;
        strings_n++;

        /* print the substitutions to be made. */
        if (flags & FLAG_DEBUG) {
            for (ref = max->refs; ref; ref = ref->nxt) {
                int         ix  = ref->ix;
                const byte *src = ref->b + max->l;
                byte       *dst = (byte *)ref->b;
                const byte *end = strings[ix] + strings_sz[ix];
                int n = strings_sz[ix] - (src - strings[ix]);

                fprintf(stderr,
                    D("SUBST: string[%d], beg_hole=0x%lx, end_hole=0x%lx, end=0x%lx\n"),
                    ix, dst - strings[ix], src - strings[ix], end - strings[ix]);
            } /* for */
        } /* if */

        /* substitute the strings as macro calls */
        for (ref = max->refs; ref; ref = ref->nxt) {
            int         ix  = ref->ix;
            const byte *src = ref->b + max->l;
            byte       *dst = (byte *)ref->b;
            const byte *end = strings[ix] + strings_sz[ix];
            int n = strings_sz[ix] - (src - strings[ix]);

            *dst++ = ESCAPE;
            *dst++ = i; /* i is the macro index */
            assert(n >= 0);
            while (n--) *dst++ = *src++;
            strings_sz[ix] -= max->l - MACRO_SIZE;
        } /* for */

        /* delete the trie as it is no more needed */
        del_trie(root_trie);
        /* print the strings */
        if (flags & FLAG_DEBUG)
            print_strings();
    } /* for */

    /* PRINT OUTPUT */
    fprintf(out,
        TYPE_INT " macros_n = %d;\n"
        TYPE_INT " macros_sz[] = {",
        strings_n - mark);
    fprint_out_int(out, strings_sz + mark, strings_n - mark);
    fprintf(out,
        "}; /* macros_sz */\n\n"
        TYPE_BYTE " macros[] = {");

    for (i = mark; i < strings_n; i++) {
        if (flags & FLAG_DEBUG) {
            fprintbuf(stderr, strings_sz[i], strings[i],
                D("macros[0x%02x, 0x%02x] ="),
                ESCAPE, i-mark);
        } /* if */
        fprintf(out, "\n    /* macro #%d */", i - mark);
        fprint_out_byte(out, strings[i], strings_sz[i]);
    } /* for */

    fprintf(out,
        "}; /* macros */\n"
        "\n"
        TYPE_INT " strings_n = %d;\n"
        TYPE_INT " strings_sz [] = {",
        mark);
    fprint_out_int(out, strings_sz, mark);
    fprintf(out,
        "}; /* strings_sz */\n"
        TYPE_BYTE " strings [] = {");

    for(i = 0; i < mark; i++) {
        if (flags & FLAG_DEBUG) {
            fprintbuf(stderr, strings_sz[i], strings[i],
                D("strings[%d] =>"), i);
        } /* if */

        fprintf(out, "\n    /* string #%d */", i);
        fprint_out_byte(out, strings[i], strings_sz[i]);
        fprintf(out, "}; /* strings */\n\n");
    } /* for */

    if (out != stdout) fclose(out);

    return EXIT_SUCCESS;

} /* main */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
