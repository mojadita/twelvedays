/* $Id: header.h.m4,v 1.7 2005/11/07 19:39:53 luis Exp $
 * Author: Luis Colorado <luis.colorado@ericsson.com>
 * Date: Wed Jan 28 14:00:53 EET 2015
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

/* Do not include anything BEFORE the line below, as it would not be
 * protected against double inclusion from other files
 */
#ifndef DECO_H
#define DECO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* constants */
#define ESCAPE		139
#define EOS			255
#define ESCOUT		254

/* types */

typedef unsigned char UWord8;
typedef short UWord16;

/* prototypes */

int deco_macro(const UWord8 *in, UWord16 len, UWord8 *out);
int deco_string(UWord8 *out);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* DECO_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.
 */

/* $Id: header.h.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
