/*
  generic fifo queues based on a circular buffer

  J.J. Green 2012, 2015
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "fifo/fifo.h"
#include "log/lmlog.h"

#include <string.h>

//#ifdef TRACE
#include <stdio.h>
//#endif



struct fifo_t
{
  size_t size, n, max, peak, first;
  struct
  {
    size_t n;
    void *dat;
  } buffer;
};

/*
  the factor by which we expand/contract the ring buffer
  when it is full/almost empty, this must be at least 2
*/

#define RESIZE_FACTOR 2

/*
  the ratio of buffer size to queue length above which a
  buffer contraction is triggered, this must be larger
  than RESIZE_FACTOR
*/

#define CONTRACT_THRESH 4

/*
  For debugging
*/

#ifdef TRACE

static void fifo_print(fifo_t *f, const char *prefix)
{
  printf("%s %zi/%zi/%zi (%zi)\n",
	 prefix, f->first, f->n, f->buffer.n, f->size);
}

#else

#define fifo_print(f, prefix)

#endif

/*
  create a new fifo for objects of the specified
  size, and with a buffer big enough to hold n of
  them.  Returns the fifo or NULL on error.
*/

extern fifo_t* fifo_new(size_t size, size_t n, size_t max)
{
  if ((size == 0) || (n == 0))
    return NULL;

  fifo_t *f;

  if ((f = malloc(sizeof(fifo_t))) != NULL)
    {
      void *dat;

      if ((dat = malloc(n*size)) != NULL)
	{
	  f->buffer.n = n;
	  f->buffer.dat = dat;

	  f->size = size;
	  f->max = max;
	  f->n = 0;
	  f->peak = 0;
	  f->first = 0;

	  fifo_print(f, "*");

	  return f;
	}

      free(f);
    }

  return NULL;
}

/*
  return the number of objects queued
*/

extern size_t fifo_peak(const fifo_t *f)
{
  return f->peak;
}

/*
  add a new object to the end of the queue, expanding the
  allocated ring buffer if needed.  Returns FIFO_ERROR
  if an error occured (which will be due to a failed
  realloc)
*/

extern int fifo_enqueue(fifo_t *f, const void *item)
{
  /* check for user max */

  if ((f->max > 0) && (f->n >= f->max))
    return FIFO_USERMAX;

  fifo_print(f, "e <");

  /* expand the buffer if full */

  if (f->n == f->buffer.n)
    {
      size_t n0 = f->buffer.n * RESIZE_FACTOR;
      void *dat0 = realloc(f->buffer.dat, n0 * f->size);

      if (dat0 == NULL)
	return FIFO_ERROR;

      /*
	if the first element is not at the start of the
	buffer then copy the start of the buffer to the
	newly allocated end of the buffer
      */

      if (f->first > 0)
	memcpy((char*)dat0 + (f->buffer.n * f->size),
	       dat0,
	       f->first * f->size);

      f->buffer.n = n0;
      f->buffer.dat = dat0;
    }

  /* copy the object into its slot at the end of the queue */

  size_t last = (f->first + f->n) % f->buffer.n;

  memcpy((char*)(f->buffer.dat) + (last * f->size), item, f->size);

  f->n++;

  /* update peak length */

  if (f->n > f->peak)
    f->peak = f->n;

  /*
    note that in the expansion code above there is no
    need for a memcpy() if the head of the queue (first)
    is 0, so these are cheap expansions.

    we could check for (first == 0) and (nearly full)
    here and perform a pre-emptive cheap expansion
    if so.
  */

  fifo_print(f, "e >");

  return FIFO_OK;
}

/*
  dequeue the end of the queue. Returns FIFO_OK and
  assigns the dequeued object to item if successful,
  returns FIFO_EMPTY if there is nothing to dequeue,
  and FIFO_ERROR if an error occured
*/

extern int fifo_dequeue(fifo_t *f, void *item)
{
  fifo_print(f, "d <");

  if (f->n == 0)
    return FIFO_EMPTY;

  memcpy(item, (char*)(f->buffer.dat) + f->first * f->size, f->size);
  f->first = (f->first + 1) % f->buffer.n;
  f->n--;

  /* perform a contraction if it is easy and needed */

  if ((f->first == 0) &&
      (f->n * CONTRACT_THRESH < f->buffer.n) &&
      (f->buffer.n > CONTRACT_THRESH))
    {
      size_t n0 = f->buffer.n / RESIZE_FACTOR;
      void* dat0 = realloc(f->buffer.dat, n0 * f->size);

      /*
	the C standard does not rule out that a shrinking
	realloc() returns a NULL, but it does seem unlikely
      */

      if (dat0 == NULL)
	return FIFO_ERROR;

      f->buffer.n   = n0;
      f->buffer.dat = dat0;
    }

  fifo_print(f, "d >");

  return FIFO_OK;
}

extern void fifo_destroy(fifo_t *f)
{
  if (f->buffer.n > 0)
    free(f->buffer.dat);

  free(f);
}
