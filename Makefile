# $Id$
# Author: Luis Colorado <lc@luiscoloradosistemas.com>
# Date: vie dic 12 19:19:35 EET 2014
# Disclaimer: (C) 2014 LUIS COLORADO

targets = ct td trie exp

all: $(targets)
clean:
	$(RM) $(targets) $(ct_objs) $(td_objs) $(trie_objs) $(exp_objs)

ct_objs = ct.o fprintbuf.o
ct_libs = -lavl
ct: $(ct_objs)
	$(CC) $(LDFLAGS) -o $@ $(ct_objs) $(ct_libs)

td_objs = td.o
td_libs = 
td: $(td_objs)
	$(CC) $(LDFLAGS) -o $@ $(td_objs) $(td_libs)

trie_objs= trie_main.o trie.o
trie_libs= -lavl
trie: $(trie_objs)
	$(CC) $(LDFLAGS) -o $@ $(trie_objs) $(trie_libs)

exp_objs = exp.o
exp_libs = 
exp: $(exp_objs)
	$(CC) $(LDFLAGS) -o $@ $(exp_objs) $(exp_libs)
