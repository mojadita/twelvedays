#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

char tab[256] = {0};

int main()
{
    int i, n = 0;

    while ((i = getchar()) != EOF) tab[i] = 1;

    for (i = ' '; i < 0x7f; i++) {
        if (!tab[i]) printf("%c", i);
    }
    printf("\n");
}
            
