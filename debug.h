#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define eprintf(format,...);{\
    if(env){\
      fprintf(stderr,format,__VA_ARGS__);\
  }
}

