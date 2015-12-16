/* matrix multiplication */

#include <stdio.h>
#include <stdlib.h>

#define  N  50

int a[N][N], b[N][N], c[N][N];

static void show(void) {
    int i,j;

    for(i=0; i<N; i++)  {
        for(j=0; j<N; j++)
            printf("%d ",c[i][j]);
        printf("\n");
    }
}

static void inita(void) {
    int i,j;

    for(i=0; i<N; i++)
        for(j=0; j<N; j++)
            a[i][j] = 2;
}

static void initb(void) {
    int i,j;

    for(i=0; i<N; i++)
        for(j=0; j<N; j++)
            b[i][j] = 3;
}

static void matmul(void) {
    int i,j,k;

    for(i=0; i<N; i++) {
        for(j=0; j<N; j++) {
            c[i][j] = 0;
            for(k=0; k<N; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

int main(void) {
    inita();
    initb();
    matmul();
    show();

    return 0;
}

