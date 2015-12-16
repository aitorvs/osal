#include <stdio.h>

/* tower of hanoi */

int num[4];

static void mov( int n, int from, int to)
{
    int other;
    if( n == 1) {
        num[from] = num[from] - 1;
        num[to] = num[to] + 1;
        printf("%d -> %d\n",from,to);
    } else {
        other = 6 - from - to;
        mov(n-1, from, other);
        mov(1, from, to );
        mov(n-1, other, to);
    }
}

int main(void) {
    int disk;
    disk = 3;
    num[0] = 0;
    num[1] = disk;
    num[2] = 0;
    num[3] = 0;
    mov(disk,1,3);

    return 0;
}
