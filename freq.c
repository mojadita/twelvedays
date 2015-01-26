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
#define N	256
#define D(X) __FILE__":%d:%s:"X,__LINE__,__func__

/* types */
struct entry {
	unsigned long long counter;
	unsigned char c;
} table[N];

char *strtab[]={
	"<nul>", "<soh>", "<stx>", "<etx>", "<eot>","<enq>","<ack>","<bel>",
	"<bs>", "<ht>", "<lf>", "<vt>","<ff>","<cr>","<so>","<si>",
	"<dle>","<dc1>","<dc2>","<dc3>","<dc4>","<nak>","<syn>","<etb>",
	"<can>","<em>","<sub>","<esc>","<fs>","<gs>","<rs>","<us>",
	" ","!","\\\"","#","$","%","&","\\'",
	"(", ")", "*", "+", ",", "-", ".", "/",
	"0", "1", "2", "3", "4", "5", "6", "7",
	"8", "9", ":", ";", "<", "=", ">", "?",
	"@", "A", "B", "C", "D", "E", "F", "G",
	"H", "I", "J", "K", "L", "M", "N", "O",
	"P", "Q", "R", "S", "T", "U", "V", "W",
	"X", "Y", "Z", "[", "\\\\", "]", "^", "_",
	"`", "a", "b", "c", "d", "e", "f", "g",
	"h", "i", "j", "k", "l", "m", "n", "o",
	"p", "q", "r", "s", "t", "u", "v", "w",
	"x", "y", "z", "{", "|", "}", "~", "<del>",
	"\\200", "\\201", "\\202", "\\203", "\\204", "\\205", "\\206", "\\207",
	"\\210", "\\211", "\\212", "\\213", "\\214", "\\215", "\\216", "\\217",
	"\\220", "\\221", "\\222", "\\223", "\\224", "\\225", "\\226", "\\227",
	"\\230", "\\231", "\\232", "\\233", "\\234", "\\235", "\\236", "\\237",
	"\\240", "\\241", "\\242", "\\243", "\\244", "\\245", "\\246", "\\247",
	"\\250", "\\251", "\\252", "\\253", "\\254", "\\255", "\\256", "\\257",
	"\\260", "\\261", "\\262", "\\263", "\\264", "\\265", "\\266", "\\267",
	"\\270", "\\271", "\\272", "\\273", "\\274", "\\275", "\\276", "\\277",
	"\\300", "\\301", "\\302", "\\303", "\\304", "\\305", "\\306", "\\307",
	"\\310", "\\311", "\\312", "\\313", "\\314", "\\315", "\\316", "\\317",
	"\\320", "\\321", "\\322", "\\323", "\\324", "\\325", "\\326", "\\327",
	"\\330", "\\331", "\\332", "\\333", "\\334", "\\335", "\\336", "\\337",
	"\\340", "\\341", "\\342", "\\343", "\\344", "\\345", "\\346", "\\347",
	"\\350", "\\351", "\\352", "\\353", "\\354", "\\355", "\\356", "\\357",
	"\\360", "\\361", "\\362", "\\363", "\\364", "\\365", "\\366", "\\367",
	"\\370", "\\371", "\\372", "\\373", "\\374", "\\375", "\\376", "\\377",
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
