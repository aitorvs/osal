#include <stdio.h>

/* prime Sieve of Erasthones */

#include <stdio.h>

#define N 9000000

int p[N];

static void sieve(void) {
    int i, k, prime;

    for(i=0; i<N; i++)
        if( p[i] ) {
            prime = i + i + 3;
            for(k=i+ prime; k<N; k+= prime)
                p[k] = 0;
        }
}

static void show(void) {
    int i, cnt, last;

    cnt = 0;
    last = 0;
    for(i=0; i<N; i++)
        if ( p[i] ) {
            last = i;
            cnt++;
        }
    printf("prime found %d last %d\n",cnt,last);
}

int main(void) {
    int i;

    for(i=0; i<N; i++ ) p[i] = 1;
    sieve();
    show();

    return 0;
}

