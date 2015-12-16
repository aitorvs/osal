/* quick sort */

#include <stdio.h>
#include <stdlib.h>

#define N 10000

int a[N];

static int rnum(void) {
  return( rand()%N );
}

static void initran(void) {
  int i;
  for(i=0; i<N; i++)
    a[i] = rnum();
}

static void qsort2(int l,int r)
{
  int v,t,i,j;

  if ( r>l ) {
    v = a[r];
    i = l-1;
    j = r;
    do {
      do i++; while (a[i] < v);
      do j--; while (a[j] > v);
      t = a[i];
      a[i] = a[j];
      a[j] = t;
    } while( j > i);
    a[j] = a[i];
    a[i] = a[r];
    a[r] = t;
    qsort2(l,i-1);
    qsort2(i+1,r);
  }
}

static void show(int n)
{
  int i;
  for(i=0; i<n; i++) printf("%d ",a[i]);
  printf("\n");
}

int main(void) {
  initran();
  qsort2(0,N);
  show(N);

  return 0;
}

