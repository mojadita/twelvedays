#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MAX     65536
#define N       32

char buffer[MAX];
char *strings[N];

void pr(char *s)
{
    for(;*s;s++) {
        if (*s > 0 && *s < 32 && strings[*s])
            pr(strings[*s]);
        else putchar(*s);
    }
}

int main(int argc, char **argv)
{
    int c;
    char *p = buffer;
    int estado = 0;
    
    while (fgets(buffer, sizeof buffer, stdin)) {
        if (buffer[0] > 0 && buffer[0] < 32) {
            int l = strlen(buffer);
            if (buffer[l-1] == '\n') buffer[--l] = '\0';
            strings[*p] = strdup(buffer + 1);
        } else {
            pr(buffer);
        }
    }
}   
