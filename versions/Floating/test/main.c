#include <stdio.h> 
#include <stdint.h> 


uint32_t f_to_u(float n)
{
   return *(uint32_t *)&n;
}

int32_t f_to_i(float n)
{
   return *(int32_t *)&n;
}

float u_to_f(uint32_t n)
{
   return *(float *)&n;
}

void main()
{
   float a = 177.13;
   printf("a: %d\n", f_to_i(a));

   float b = 231.9999;
   printf("b: %d\n", f_to_i(b));

   float c = a + b;
   printf("c signed: %d\n", f_to_i(c));

   float d = 99.3;
   printf("d signed: %d\n", f_to_i(d));
   float e = u_to_f(f_to_u(c) | f_to_u(d));
   printf("e signed: %d\n", f_to_i(e));

   float f = 20.8;
   printf("f signed: %d\n", f_to_i(f));
   f = e - f;
   printf("f signed: %d\n", f_to_i(f));
   printf("f: %.6f\n", f);

   // float c = *(float *)&((*(unsigned int *)&a) && (*(unsigned int *)&b));
}