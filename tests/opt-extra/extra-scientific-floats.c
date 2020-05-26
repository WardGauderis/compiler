#include <stdio.h>

// te grote floats werken niet in mips want deze zijn undefined behaviour
int main()
{
  printf("%f\n", 5e+55555555555F);
}