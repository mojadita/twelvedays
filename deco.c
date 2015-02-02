/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $
 * Author: Luis Colorado <luis.colorado@ericsson.com>
 * Date: Wed Jan 28 10:50:41 EET 2015
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

#define IN_DECO_C

/* Standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "deco.h"

#include "rom_com_fx.binDIFF.i"

/* variables */

const UWord8 **the_strings = NULL;
const UWord8 **the_macros = NULL;

/* functions */

int deco_macro(const UWord8 *in, UWord16 len, UWord8 *out)
{
    int res = 0;

    for (; len > 0; len--, in++) {
        if (*in == ESCAPE) {
            len--; in++;
            assert(len > 0);
            if (*in == ESCOUT) {
                *out++ = ESCAPE; res++;
            } else {
                register int n = deco_macro(
                        the_macros[*in],
                        macros_sz[*in],
                        out);
                out += n; res += n;
            } /* if */
        } else {
            *out++ = *in;
            res++;
        } /* if */
    } /* for */

    return res;
} /* deco_macro */

int deco_string(UWord8 *out)
{
    int i, res = 0;
    const UWord8 *p = strings;

    /* INITIALIZATION */
    if (!the_strings) {
        the_strings = calloc(strings_n, sizeof(UWord8 *));
        for (i = 0; i < strings_n; i++) {
            the_strings[i] = p;
            p += strings_sz[i];
        } /* for */
    } /* if */
    if (!the_macros) {
        the_macros = calloc(macros_n, sizeof(UWord8 *));
        for (i = 0; i < macros_n; i++) {
            the_macros[i] = p;
            p += macros_sz[i];
        } /* for */
    } /* if */

    /* PROCESSING */
    for (i = 0; i < strings_n; i++) {
        register int n;
        n = deco_macro(the_strings[i], strings_sz[i], out);
        res += n; out += n;
    } /* for */

    fprintf(stderr,
            "Sizeof  strings_n: %8ld\n"
            "Sizeof strings_sz: %8ld\n"
            "Sizeof   macros_n: %8ld\n"
            "Sizeof  macros_sz: %8ld\n"
            "Sizeof    strings: %8ld\n"
            "            Total: %8ld\n"
            "           Output: %8ld\n",
            sizeof strings_n,  sizeof strings_sz,  sizeof macros_n,  sizeof macros_sz,  sizeof strings,
            sizeof strings_n + sizeof strings_sz + sizeof macros_n + sizeof macros_sz + sizeof strings,
            res);

    return res;
} /* deco */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
