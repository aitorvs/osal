//portable delay.

int HZ = 100;

void delay(int loops)
{
  volatile long i;
  for (i = loops; i >= 0 ; i--) ;
}
