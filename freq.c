/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $
 * Author: Luis Colorado <luis.colorado@ericsson.com>
 * Date: Mon Jan 26 14:41:38 EET 2015
 *
 * Disclaimer:
 *  (C) 2015 ERICSSON AB.
 *
 * All Rights Reserved. Reproduction in whole or in part is prohibited
 * without the written consent of the copyright owner.
 *
 * ERICSSON MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF
 * THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. ERICSSON SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */

#define IN_FREQ_C

/* Standard include files */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* constants */
#define N   256
#define D(X) __FILE__":%d:%s:"X,__LINE__,__func__

/* types */
struct entry {
    unsigned long long counter;
    unsigned char c;
} table[N];

const char *const strtab[] = {
    /* 0 */ "<nul>", "<soh>", "<stx>", "<etx>",
    /* 4 */ "<eot>", "<enq>", "<ack>", "<bel>",
    /* 8 */ "<bs>", "<ht>", "<lf>", "<vt>",
    /* 12 */ "<ff>", "<cr>", "<so>", "<si>",
    /* 16 */ "<dle>", "<dc1>", "<dc2>", "<dc3>",
    /* 20 */ "<dc4>", "<nak>", "<syn>", "<etb>",
    /* 24 */ "<can>", "<em>", "<sub>", "<esc>",
    /* 28 */ "<fs>", "<gs>", "<rs>", "<us>",

    /* 32 */ " ","!","\\\"","#","$","%","&","\\'",
    /* 40 */ "(", ")", "*", "+", ",", "-", ".", "/",
    /* 48 */ "0", "1", "2", "3", "4", "5", "6", "7",
    /* 56 */ "8", "9", ":", ";", "<", "=", ">", "?",
    /* 64 */ "@", "A", "B", "C", "D", "E", "F", "G",
    /* 72 */ "H", "I", "J", "K", "L", "M", "N", "O",
    /* 80 */ "P", "Q", "R", "S", "T", "U", "V", "W",
    /* 88 */ "X", "Y", "Z", "[", "\\\\", "]", "^", "_",
    /* 96 */ "`", "a", "b", "c", "d", "e", "f", "g",
    /* 104 */ "h", "i", "j", "k", "l", "m", "n", "o",
    /* 112 */ "p", "q", "r", "s", "t", "u", "v", "w",
    /* 120 */ "x", "y", "z", "{", "|", "}", "~", "<del>",
    /* 128 */ "\\200", "\\201", "\\202", "\\203", "\\204", "\\205", "\\206", "\\207",
    /* 136 */ "\\210", "\\211", "\\212", "\\213", "\\214", "\\215", "\\216", "\\217",
    /* 144 */ "\\220", "\\221", "\\222", "\\223", "\\224", "\\225", "\\226", "\\227",
    /* 152 */ "\\230", "\\231", "\\232", "\\233", "\\234", "\\235", "\\236", "\\237",
    /* 160 */ "\\240", "\\241", "\\242", "\\243", "\\244", "\\245", "\\246", "\\247",
    /* 168 */ "\\250", "\\251", "\\252", "\\253", "\\254", "\\255", "\\256", "\\257",
    /* 176 */ "\\260", "\\261", "\\262", "\\263", "\\264", "\\265", "\\266", "\\267",
    /* 184 */ "\\270", "\\271", "\\272", "\\273", "\\274", "\\275", "\\276", "\\277",
    /* 192 */ "\\300", "\\301", "\\302", "\\303", "\\304", "\\305", "\\306", "\\307",
    /* 200 */ "\\310", "\\311", "\\312", "\\313", "\\314", "\\315", "\\316", "\\317",
    /* 208 */ "\\320", "\\321", "\\322", "\\323", "\\324", "\\325", "\\326", "\\327",
    /* 216 */ "\\330", "\\331", "\\332", "\\333", "\\334", "\\335", "\\336", "\\337",
    /* 224 */ "\\340", "\\341", "\\342", "\\343", "\\344", "\\345", "\\346", "\\347",
    /* 232 */ "\\350", "\\351", "\\352", "\\353", "\\354", "\\355", "\\356", "\\357",
    /* 240 */ "\\360", "\\361", "\\362", "\\363", "\\364", "\\365", "\\366", "\\367",
    /* 248 */ "\\370", "\\371", "\\372", "\\373", "\\374", "\\375", "\\376", "\\377",
    /* 256 */
};

/* functions */
void do_usage(void)
{
} /* do_usage */

void process(char *name)
{
    FILE *f = stdin;
    int c;

    if (name) {
        f = fopen(name, "rb");
        if (!f) {
            fprintf(stderr,
                D("%s: %s(errno=%d)\n"),
                name, strerror(errno),
                errno);
            exit(EXIT_FAILURE);
        } /* if */
    } else {
        f = stdin;
        name = "<<stdin>>";
    } /* if */

    printf(D("%s: BEGIN\n"), name);
    while ((c = fgetc(f)) != EOF)
        table[c].counter++;
    printf(D("%s: END\n"), name);

} /* process */

int cmptab(const void *_a, const void *_b)
{
    const struct entry *a = _a;
    const struct entry *b = _b;

    if (a->counter == b->counter)
        return b->c - a->c;
    return a->counter - b->counter;
}

/* main program */
int main (int argc, char **argv)
{

    extern int optind;
    extern char *optarg;
    int opt, i;

    while ((opt = getopt(argc, argv, "h")) != EOF) {
        switch(opt) {
        case 'h':
          do_usage(); exit(0);
        } /* switch */
    } /* while */

    argc -= optind; argv += optind;
    for(i = 0; i < N; i++) {
        table[i].counter = 0ULL;
        table[i].c = (char) i;
    } /* for */

    if (argc) {
        for (i = 0; i < argc; i++) {
            process(argv[i]);
        } /* for */
    } else {
        process(NULL);
    } /* if */

    qsort(table, N, sizeof table[0], cmptab);

    for (i = 0; i < N; i++) {
        int c = table[i].c;
        printf("%10llu: [%u,0%03o,0x%02x,'%s']\n",
            table[i].counter, c, c, c, strtab[c]);
    } /* for */

    return EXIT_SUCCESS;

} /* main */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
