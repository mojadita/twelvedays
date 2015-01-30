/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $
 * Author: Luis Colorado <luis.colorado@ericsson.com>
 * Date: Wed Jan 28 14:37:23 EET 2015
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

#define IN_TEST_DECO_C

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "deco.h"

/* constants */

/* types */

/* prototypes */

/* variables */
static char TEST_DECO_C_RCSId[]="\n$Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $\n";

/* functions */

int main()
{
    char buffer[1<<20]; /* 1Mb */
    int res;

    res = deco_string(buffer);
    write(1, buffer, res);

}

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
