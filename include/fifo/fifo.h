/*
  generic fifo queues based on a circular buffer

  J.J. Green 2015
*/

#ifndef FIFO_H
#define FIFO_H

#ifdef __cplusplus
extern "C" 
{
#endif 
  
#include <stdlib.h>

#define FIFO_OK      0
#define FIFO_EMPTY   1
#define FIFO_ERROR   2
#define FIFO_USERMAX 3

  typedef struct fifo_t fifo_t;
  
  extern fifo_t* fifo_new(size_t, size_t, size_t);
  extern size_t  fifo_peak(const fifo_t*);
  extern int     fifo_enqueue(fifo_t*, const void*);
  extern int     fifo_dequeue(fifo_t*, void*);
  extern void    fifo_destroy(fifo_t*);
  
#ifdef __cplusplus
}
#endif 

#endif
