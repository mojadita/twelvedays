/* $Id: header.h.m4,v 1.7 2005/11/07 19:39:53 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Mon Dec 15 21:43:00 EET 2014
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

/* Do not include anything BEFORE the line below, as it would not be
 * protected against double inclusion from other files
 */
#ifndef TRIE_H
#define TRIE_H

static char TRIE_H_RCSId[] = "\n$Id: header.h.m4,v 1.7 2005/11/07 19:39:53 luis Exp $\n";

/* constants */
#ifndef D
#define D(X) __FILE__":%d:%s:" X, __LINE__, __func__
#endif

#define MACRO_SIZE		2
#define IS_ESCAPE_SEQ	(1 << 8)

/* types */

/* ref_buff form a stack of references into the text where we have
 * found this substring. If this reference doesn't overlapp with the
 * previous one, we push it onto the stack making it the last one. */
struct ref_buff {
	const char *b;
	struct ref_buff *nxt;
	int ix;
}; /* ref_buff */

struct trie_node {
	int					c; /* the character encoded in this node */
	int					n; /* number of times this string repeats */
	int					l; /* length of this string. */
	struct trie_node	*prt; /* parent of this trie node. */
	AVL_TREE			sub; /* avl tree to subnodes of this trie */
	struct ref_buff		*refs; /* stack of references. */
}; /* trie_node */

/* prototypes */
struct trie_node *new_trie(void);
struct trie_node *add_string(char *s, int l, struct trie_node *t, int d);
struct trie_node *walk_trie(struct trie_node *t);
void del_trie(struct trie_node *t);

#endif /* TRIE_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.
 */

/* $Id: header.h.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
