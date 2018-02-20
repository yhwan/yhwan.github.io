/*--------------------------------------------------------------------*/
/* chunkbase.c                                                        */
/* Author: Donghwi Kim                                                */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "chunkbase.h"

enum {
   MEMALLOC_MIN = 1024,
};

struct Chunk {
   Chunk_T next;     /* Pointer to the next chunk in the free chunk list */
   size_t units;     /* Capacity of a chunk (chunk units) */
};

/* gFreeHead: point to first/last chunk in free list */
static Chunk_T gFreeHead = NULL, gFreeTail = NULL;

/* gHeapStart, gHeapEnd: start and end of the heap area.
 * gHeapEnd will move if you increase the heap */
static void *gHeapStart = NULL, *gHeapEnd = NULL;

/*--------------------------------------------------------------------*/
void
ChunkInitDS(void)
{
   /* Initialize gHeapStart if it is not initialized yet */
   gHeapStart = gHeapEnd = sbrk(0);
   if (gHeapStart == (void *)-1) {
      fprintf(stderr, "sbrk(0) failed\n");
      exit(-1);
   }
}

/*--------------------------------------------------------------------*/
int
ChunkGetStatus(Chunk_T c)
{
   /* If a chunk is not in the free chunk list, then the next pointer
    * must be NULL. We can figure out whether a chunk is in use or
    * free by looking at the next chunk pointer. */
   if (c->next == NULL)
      return CHUNK_IN_USE;
   else
      return CHUNK_FREE;
}

/*--------------------------------------------------------------------*/
Chunk_T
ChunkFirstFreeChunk(void)
{
	return gFreeHead;
}

/*--------------------------------------------------------------------*/
Chunk_T
ChunkLastFreeChunk(void)
{
	return gFreeTail;
}

/*--------------------------------------------------------------------*/
Chunk_T
ChunkNextFreeChunk(Chunk_T c)
{
	return c->next;
}

/*--------------------------------------------------------------------*/
int
ChunkRemoveFromList(Chunk_T c)
{
	Chunk_T w, n;
   
   if (ChunkGetStatus(c) != CHUNK_FREE)
      return -1;
   
   /* Find a previous chunk and a next chunk of a chunk to be removed
    * from free chunk list. */
   for (w = gFreeHead;
        w != NULL;
        w = n) {  
	   n = ChunkNextFreeChunk(w);

	   /* Start from second free chunk in order to have a pointer to 
		* previous free chunk. */
	   if (n == c) {
		   /* This was last free chunk in the free chunk list. */
		   if (w == n) {
			   gFreeHead = NULL;
			   gFreeTail = NULL;
		   }
		   else {
			   w->next = c->next;
			   /* Remove first free chunk. */
			   if (c == gFreeHead)
				   gFreeHead = c->next;
			   /* Remove last free chunk. */
			   else if (c == gFreeTail) {
				   gFreeTail = w;
			   }
		   }
		   c->next = NULL;
		   return 0;
	   }
   }
   
   return -1;
}

/*--------------------------------------------------------------------*/
int
ChunkInsertFirst(Chunk_T c)
{
   if (ChunkGetStatus(c) == CHUNK_FREE)
      return -1;

   /* If free chunk list is empty, chunk c points to itself. */
   if (gFreeHead == NULL) {
	   gFreeHead = c;
	   gFreeTail = c;
	   c->next = c;
   }
   else {
	   c->next = gFreeHead;
	   gFreeHead = c;

	   /* Set last free chunk to point to new free chunk. */
	   Chunk_T l = ChunkLastFreeChunk();   
	   l->next = c;
   }
   
   return 0;
}

/*--------------------------------------------------------------------*/
int
ChunkInsertAfter(Chunk_T e, Chunk_T c)
{
   if (ChunkGetStatus(c) == CHUNK_FREE ||
       ChunkGetStatus(e) != CHUNK_FREE)
      return -1;
   
   c->next = e->next;
   e->next = c;

   /* If e was last chunk in chunk free list, update gFreeTail to point 
	* to c. */
   if (e == gFreeTail)
	   gFreeTail = c;
   
   return 0;
}

/*--------------------------------------------------------------------*/
int
ChunkGetUnits(Chunk_T c)
{
   return c->units;
}

/*--------------------------------------------------------------------*/
Chunk_T
ChunkNextAdjacent(Chunk_T c)
{
   Chunk_T n;
   
   /* Note that a chunk consists of one chunk unit for a header and
    * many chunk units for data. */
   n = c + c->units + 1;
   
   /* If 'c' is the last chunk in memory space, then return NULL. */
   if ((void *)n >= gHeapEnd)
      return NULL;
   
   return n;
}

/*--------------------------------------------------------------------*/
Chunk_T
ChunkPrevAdjacent(Chunk_T c)
{
   Chunk_T w, n;
   
   /* Since there is no way to directly figure out a previous adjacent
    * chunk in memory, we check all chunks in memory one by one. */
   for (w = (Chunk_T)gHeapStart;
        w != NULL;
        w = n) {
      n = ChunkNextAdjacent(w);
      if (n == c)
         return w;
      else if (n > c)
         return NULL;
   }
   
   return NULL;
}

/*--------------------------------------------------------------------*/
Chunk_T 
ChunkSplit(Chunk_T c, int units)
{
   Chunk_T c2;

   if (c < (Chunk_T)gHeapStart || c > (Chunk_T)gHeapEnd)
      return NULL;

   if (ChunkGetStatus(c) != CHUNK_FREE)
      return NULL;

   /* This chunk is not large enough to be splitted */
   if (c->units <= units + 1)
      return NULL;
   
   /* Initialize the second chunk */
   c2 = c + c->units - units;
   c2->units = units;
   c2->next = c->next;
   
   /* Adjust the first chunk */
   c->units -= (units + 1);
   c->next = c2;

   return c2;
}

/*--------------------------------------------------------------------*/
Chunk_T
ChunkMerge(Chunk_T c1, Chunk_T c2)
{
   Chunk_T w1, w2;
   
   if (c1 == c2) {
      return NULL;
   }
   
   if (ChunkGetStatus(c1) != CHUNK_FREE ||
       ChunkGetStatus(c2) != CHUNK_FREE) {
      return NULL;
   }
   
   /* w2 always comes after w1 */
   if (c1 < c2) {
      w1 = c1;
      w2 = c2;
   } else {
      w1 = c2;
      w2 = c1;
   }
   
   /* w1 and w2 are not adjacent. */
   if (ChunkNextAdjacent(w1) != w2) {
      return NULL;
   }
   
   /* Adjust the merged chunk */
   w1->units += w2->units + 1;
   w1->next = w2->next;

   /* Update tail if w2 was last chunk in free chunk list. */
   if (w2 == gFreeTail)
	   gFreeTail = w1;
   
   return w1;
}

/*--------------------------------------------------------------------*/
Chunk_T
ChunkAllocateMem(int units)
{
   Chunk_T c, w;

   if (units < MEMALLOC_MIN)
      units = MEMALLOC_MIN;
   
   /* Note that we need to allocate one more unit for header. */
   c = (Chunk_T)sbrk((units + 1) * CHUNK_UNIT);
   if (c == (Chunk_T)-1)
      return NULL;
   
   gHeapEnd = sbrk(0);
   
   c->units = units;
   
   /* Insert the newly allocated chunk 'c' to the free list.
    * Note that the list is sorted in ascending order. */
   if (gFreeTail == NULL)
      ChunkInsertFirst(c);
   else 
      ChunkInsertAfter(gFreeTail, c);

   /* Merge the new chunk with an existing free chunk, if possible. */
   if ((w = ChunkPrevAdjacent(c)) != NULL &&
       ChunkGetStatus(w) == CHUNK_FREE)
      c = ChunkMerge(c, w);
   
   assert(ChunkValidityCheck());

   return c;
}

/*--------------------------------------------------------------------*/
bool
ChunkValidityCheck(void)
{
   Chunk_T w, p = NULL;

   if (gHeapStart == NULL) {
      fprintf(stderr, "Uninitialized heap start\n");
      return false;
   }

   if (gHeapEnd == NULL) {
      fprintf(stderr, "Uninitialized heap end\n");
      return false;
   }

   for (w = gFreeHead; w != NULL; w = w->next) {
      if (p == w) {
         fprintf(stderr, "Invalid loop in free chunk list\n");
         return false;
      }
	  p = w;

	  if (w->next == gFreeHead)
		  break;
   }

   for (w = (Chunk_T)gHeapStart, p = NULL;
        w && w < (Chunk_T)gHeapEnd;
        w = ChunkNextAdjacent(w)) {
      if (p && ChunkGetStatus(p) == CHUNK_FREE &&
          ChunkGetStatus(w) == CHUNK_FREE) {
         fprintf(stderr, "Uncoalesced chunks\n");
         return false;
      }
      p = w;
   }
   
   return true;
}

/*--------------------------------------------------------------------*/
void
ChunkPrint(void)
{
   Chunk_T w, p = NULL;
   
   printf("heap: %p ~ %p\n", gHeapStart, gHeapEnd);
   printf("gFreeHead: %p\n", (void *)gFreeHead);
   
   printf("free chain\n");
   for (w = gFreeHead; w != NULL; w = w->next) {
      if (p == w) {
		  printf("%p->next == %p, weird!\n", (void *)p, (void *)w); exit(0);
      }
      printf("[%p: %ld units]\n", (void *)w, w->units);
      p = w;

	  if (w->next == gFreeHead)
		  break;
   }
   
   printf("mem\n");
   for (w = (Chunk_T)gHeapStart;
        w < (Chunk_T)gHeapEnd;
        w = (Chunk_T)((char *)w + (w->units + 1) * CHUNK_UNIT))
	   printf("[%p (%c): %ld units]\n", (void *)w, w->next ? 'F' : 'U', w->units);
}
