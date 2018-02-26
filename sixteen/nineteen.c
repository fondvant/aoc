#ifdef EXAMPLE
#define e_len 5
#else
//#define e_len 3017957
#define e_len 3017957
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

int i;
int elfid[e_len];

void print_stuff() {
    for(i=0;i < (132 < e_len ? 132 : e_len);i++)
        printf("%x", elfid[i]);
    printf("\n");
}

int main() {

    struct timespec begintime, nowtime;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begintime);

    int remaining = e_len;

    // fill ids
    i=0;
    for(i=0;i<e_len;i++) {
        elfid[i] = i + 1;
    }

    // receiving and giving elf
    int r = 0;
    int g;

    print_stuff();

    while(remaining > 1) { // infinitey loop

        if(remaining % 100 == 0) {
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &nowtime);
            printf("%d remaining after %d secs...\n", remaining, nowtime.tv_sec - begintime.tv_sec);
            //exit(1);
        }

        int skip = remaining / 2;
        g = (r + skip) % remaining; // find giver

        remaining--; // discard g and move the rest down
        for(i=g; i < remaining; i++)
            elfid[i] = elfid[i+1];

        if(r > g) // ensure r is at its proper place
            r--;

        r = (r + 1) % remaining; // next receiver

#ifdef EXAMPLE
        print_stuff();
#endif

    }

    printf("remaining elf index: %d\n", elfid[r]); // r is already 1-indexed
    raise(SIGINT); // break in case we fucked up again
}
