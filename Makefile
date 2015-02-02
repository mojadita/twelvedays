# $Id$
# Author: Luis Colorado <lc@luiscoloradosistemas.com>
# Date: vie dic 12 19:19:35 EET 2014
# Disclaimer: (C) 2014 LUIS COLORADO

targets = ct td trie exp freq test_deco

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

trie_objs= trie_main.o trie.o fprintbuf.o
trie_libs= -lavl
trie: $(trie_objs)
	$(CC) $(LDFLAGS) -o $@ $(trie_objs) $(trie_libs)

exp_objs = exp.o
exp_libs = 
exp: $(exp_objs)
	$(CC) $(LDFLAGS) -o $@ $(exp_objs) $(exp_libs)

freq_objs = freq.o
frea_libs =
freq: $(freq_objs)
	$(CC) $(LDFLAGS) -o $@ $(freq_objs) $(freq_libs)

test_deco_objs = test_deco.o deco.o
test_deco_libs = 
test_deco: $(test_deco_objs)
	$(CC) $(LDFLAGS) -o $@ $(test_deco_objs) $(test_deco_libs)

rom.i: td.txt.orig trie
	trie -d -s -o $@ $< 2>trie.log

trie_main.o trie.o deco.o test_deco.o: deco.h
deco.o: rom.i
test_main.o fprintbuf.o: fprintbuf.h
trie.o trie_main.o: trie.h
