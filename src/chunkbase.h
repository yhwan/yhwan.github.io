/*--------------------------------------------------------------------*/
/* chunkbase.h                                                        */
/* Author: Donghwi Kim                                                */
/*--------------------------------------------------------------------*/

#ifndef _CHUNK_BASE_H_
#define _CHUNK_BASE_H_

#pragma once

#include <stdbool.h>
#include <unistd.h>

typedef struct Chunk *Chunk_T;

enum {
   CHUNK_FREE,
   CHUNK_IN_USE,
};

enum {
   CHUNK_UNIT = 16, /* 16 = sizeof(struct Chunk) */
};

/* ChunkFirstFreeChunk:
 * Returns the first chunk in the free chunk list.
 * Returns NULL if there is no free chunk. */
Chunk_T
ChunkFirstFreeChunk(void);

/* ChunkLastFreeChunk:
 * Returns the last chunk in the free chunk list.
 * Returns NULL if there is no free chunk. */
Chunk_T
ChunkLastFreeChunk(void);

/* ChunkNextFreeChunk:
 * Returns the next free chunk in free chunk list.
 * Returns NULL if 'c' is the last free chunk in the list. */
Chunk_T
ChunkNextFreeChunk(Chunk_T c);

/* ChunkRemoveFromList:
 * Removes 'c' from the free chunk list.
 * Returns 0 on success, -1 on failure. */
int
ChunkRemoveFromList(Chunk_T c);

/* ChunkInsertFirst:
 * Insert a chunk, 'c', into the head of the free chunk list.
 * Returns 0 on success, -1 on failure. */
int
ChunkInsertFirst(Chunk_T c);

/* ChunkInsertAfter:
 * Insert a chunk, 'c', to the free chunk list after 'e' which is
 * already in the free chunk list. After the operation, 'c' will be the
 * next element of 'e' in the free chunk list.  
 * Returns 0 on success, * -1 on failure. */
int
ChunkInsertAfter(Chunk_T e, Chunk_T c);

/* ChunkGetUnits:
 * Returns the size of a chunk, 'c', in terms of the number of chunk units. */
int
ChunkGetUnits(Chunk_T c);

/* ChunkNextAdjacent:
 * Returns the next adjacent chunk to 'c' in memory space.
 * Returns NULL if 'c' is the last chunk in memory space. */
Chunk_T
ChunkNextAdjacent(Chunk_T c);

/* ChunkPrevAdjacent:
 * Returns the previous adjacent chunk to 'c' in memory space.
 * Returns NULL if 'c' is the first chunk in memory space. */
Chunk_T
ChunkPrevAdjacent(Chunk_T c);

/* ChunkGetStatus:
 * Returns a chunk's status which shows whether the chunk is in use or free.
 * Return value is either CHUNK_IN_USE or CHUNK_FREE. */
int
ChunkGetStatus(Chunk_T c);

/* ChunkAllocateMem: 
 * Allocate a new chunk which is capable of holding 'units' chunk
 * units in memory by increasing the heap, and return the new
 * chunk. This function also performs chunk merging if possible after
 * new chunk allocation. */
Chunk_T
ChunkAllocateMem(int units);

/* ChunkSplit:
 * Shrink 'c' to 'units' and return a new chunk with the reduced space.
 * Note that 'c' must be large enough to be split into two.
 * Returns NULL on failure. */
Chunk_T 
ChunkSplit(Chunk_T c, int units);

/* ChunkMerge:
 * Merge two adjacent chunks and return the merged chunk.
 * Returns NULL on error. */
Chunk_T
ChunkMerge(Chunk_T c1, Chunk_T c2);

/* ChunkInitDS:
 * Initialize data structures and global variables for chunk management. */
void
ChunkInitDS(void);

/* Following two functions are for debugging.
 * These will be removed by C preprocessor if you compile the code with
 * -DNDEBUG option. */

#ifndef NDEBUG

/* ChunkValidityCheck:
 * Validity check for entire data structures for chunks. Note that this
 * is basic sanity check, and passing this test does not guarantee the
 * integrity of your code. 
 * Returns 'true' (non-zero value) on success, 'false' (zero) on failure. */
bool
ChunkValidityCheck(void);

/* ChunkPrint:
 * Print out entire data sturucture for chunks. This function will print
 * all chunks in free chunks list and in memory. */
void
ChunkPrint(void);

#endif /* NDEBUG */

#endif /* _CHUNK_BASE_H_ */
