#include <stdio.h>
#include <assert.h>

#include "chunkbase.h"

/*--------------------------------------------------------------*/
/* SizeToUnits:
 * Returns capable number of units for 'size' bytes. */
/*--------------------------------------------------------------*/
int
SizeToUnits(int size)
{
   return (size + (CHUNK_UNIT-1))/CHUNK_UNIT;
}

/*--------------------------------------------------------------*/
/* FindProperChunk:
 * Find a free chunk that is capable of holding 'units' space.
 * Returns the proper chunk on success, NULL on failure. */
/*--------------------------------------------------------------*/
Chunk_T
FindProperChunk(int units)
{
   Chunk_T w;

   /* Traverse a free chunk list until the end, and return a free chunk 
	* that has enough space to hold units. */
   for (w = ChunkFirstFreeChunk();
        w != NULL;
        w = ChunkNextFreeChunk(w)) {
	   
	  if (ChunkGetUnits(w) >= units)
		return w;
	  else if (w == ChunkLastFreeChunk()) 
		return NULL;
   }
   return NULL;
}

/*--------------------------------------------------------------*/
/* UseChunk:
 * Remove the chunk, c, from the free list, and mark it as used
 * Returns 0 on success, -1 on failure. */
/*--------------------------------------------------------------*/
int
UseChunk(Chunk_T c)
{
   return ChunkRemoveFromList(c);
}

/*--------------------------------------------------------------*/
/* FreeChunk:
 * Mark a chunk as free by inserting it to free list.
 * Returns 0 on success, -1 on failure. */
/*--------------------------------------------------------------*/
int
FreeChunk(Chunk_T c)
{
   Chunk_T w, n = NULL;
   
   w = ChunkFirstFreeChunk();

   /* If the free chunk list is empty, insert it at first. */
   if (w == NULL)
	   return ChunkInsertFirst(c);  

   /* Traverse the free chunk list, and insert 'c' into a proper place.
    * Note that the free chunk list is sorted in the ascending order. */
   while (1) {
      n = ChunkNextFreeChunk(w);

	  /* Address is smaller than first chunk in free chunk list. */
      if (w > c)
         return ChunkInsertFirst(c);
	  /* There is only one chunk in free chunk list. */
	  else if (w == n) 
		  return ChunkInsertAfter(w, c);
      else if (w < c && (c < n || n < w))
         return ChunkInsertAfter(w, c);

	  w = n;
   }
   
   return -1;
}

/*--------------------------------------------------------------*/
/* GetChunkFromDataPtr:
 * Returns a chunk which contains data 'm'. */
/*--------------------------------------------------------------*/
Chunk_T
GetChunkFromDataPtr(void *m)
{
	return (Chunk_T)((char *)m - CHUNK_UNIT);
}

/*--------------------------------------------------------------*/
/* mymalloc:
 * Dynamically allocate a memory capable of holding size bytes. 
 * Substitute for GNU malloc().                                 */
/*--------------------------------------------------------------*/
void *
mymalloc(size_t size)
{
   static int isInitialized = 0;
   Chunk_T c = NULL;
   int units;
   
   if (size <= 0)
      return NULL;
   
   if (isInitialized == 0) {
      ChunkInitDS();
      isInitialized = 1;
   }

   assert(ChunkValidityCheck());

   units = SizeToUnits(size);

   /* Try to choose a proper free chunk from currently available free
    * chunks. If the trial fails, then try to allocate a new chunk by
    * increasing heap size. Returns NULL if both trials fail. */
   if ((c = FindProperChunk(units)) == NULL &&
       (c = ChunkAllocateMem(units)) == NULL)
      return NULL;

   /* Split the chosen chunk if it is too big. */
   if (ChunkGetUnits(c) > units + 1)
	   c = ChunkSplit(c, units);
   
   UseChunk(c);

   assert(ChunkValidityCheck());

   return (Chunk_T)((char *)c + CHUNK_UNIT);
}

/*--------------------------------------------------------------*/
/* myfree:
 * Releases dynamically allocated memory. 
 * Substitute for GNU free().                                   */
/*--------------------------------------------------------------*/
void
myfree(void *m)
{
   Chunk_T c = NULL, w = NULL;
   
   if (m == NULL)
      return;

   assert(ChunkValidityCheck());

   c = GetChunkFromDataPtr(m);
   
   if (ChunkGetStatus(c) == CHUNK_FREE)
      return;
   
   FreeChunk(c);
   
   /* Merge a chunk 'c' with an adjacent next chunk if the next chunk is
    * free. */
   if ((w = ChunkNextAdjacent(c)) != NULL &&
       ChunkGetStatus(w) == CHUNK_FREE)
      c = ChunkMerge(c, w);
   
   /* Merge a chunk 'c' with an adjacent previous chunk if the previous
    * chunk is free. */
   if ((w = ChunkPrevAdjacent(c)) != NULL &&
       ChunkGetStatus(w) == CHUNK_FREE)
      ChunkMerge(c, w);

   assert(ChunkValidityCheck());
}
