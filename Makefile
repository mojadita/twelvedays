# $Id$
# Author: Luis Colorado <lc@luiscoloradosistemas.com>
# Date: vie dic 12 19:19:35 EET 2014
# Disclaimer: (C) 2014 LUIS COLORADO

targets = ct td

all: $(targets)
clean:
	$(RM) $(targets) $(ct_objs) $(td_objs)

ct_objs = ct.o
ct_libs = -lavl
ct: $(ct_objs)
	$(CC) $(LDFLAGS) -o $@ $(ct_objs) $(ct_libs)

td_objs = td.o
td: $(td_objs)
	$(CC) $(LDFLAGS) -o $@ $(td_objs)
