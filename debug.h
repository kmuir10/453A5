#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define eprintf(format,...);{\
    if(env){\
      fprintf(stderr,format,__VA_ARGS__);\
  }\
}

#endif
