/* bubble.c */

#include <stdio.h>
#include <stdlib.h>

#define MAX 20000

int a[MAX];

static void initran(void) ;
static void bubble(void) ;
static void show(int n);
static int rnum(void) ;

static void initran(void) 
{
    int i;

    for(i=0; i<MAX; i++)
        a[i] = rnum();
}

static void bubble(void) 
{
    int i,j,t;

    for(i=MAX-1; i>=0; i--)
        for(j=1; j<MAX; j++ )
            if ( a[j-1] > a[j] ) {
                t = a[j-1];
                a[j-1] = a[j];
                a[j] = t;
            }
}

static void show(int n)
{
    int i;

    for(i=0; i<n; i++ ) printf("%d ",a[i]);
    printf("\n");
}

static int rnum(void) 
{
    return( rand()%MAX );
}

int main(void) {
    initran();
    bubble();
    show(10);

    return 0;
}

