/* 8 queen problem */

#include <stdio.h>


#define Q 14		/* size of Q x Q board */
#define Z Q+1 		/* empty */
#define D (Q+Q-1)       /* size of diagonal */

int col[Q], d45[D], d135[D], queen[Q];
int soln;

void printboard(void) { soln++; }

void find(int level)
{
  int i;

  if ( level == Q ) {
    printboard();
  } else {
    for(i=0; i<Q; i++ ) {
      if( col[i] >= level &&
	  d45[level+i] >= level &&
	  d135[level + Q -1 - i] >= level ) {
	queen[level] = i;
	col[i] = d45[level+i] = d135[level + Q -1 - i] = level;
	find(level+1);
	col[i] = d45[level+i] = d135[level + Q -1 - i] = Z;
      }
    }
  }
}

int main(void) {
  int i;

  for(i=0; i<Q; i++) col[i] = Z;
  for(i=0; i<D; i++) { d45[i] = d135[i] = Z; }
  soln = 0;
  find(0);
  printf("no of soln %d\n",soln);

  return 0;
}
