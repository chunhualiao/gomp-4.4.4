// https://github.com/rose-compiler/edg4x-rose/blob/master/tests/CompileTests/OpenMP_tests/hello-1.c
#include <stdio.h>
#ifdef _OPENMP
#include <omp.h>
#endif
int main(void)
{
#pragma omp parallel
  {
    printf("Hello,world!\n");
  }
  return 0;
}
