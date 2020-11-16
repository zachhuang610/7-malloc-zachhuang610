#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * BEFORE GETTING STARTED:
 * 
 * Familiarize yourself with the functions and constants/variables
 * in the following included files.
 * This will make the project a LOT easier as you go!!
 * 
 * The diagram in Section 3.1 (Specification) of the handout will help you
 * understand the constants in mm.h
 * Section 3.2 (Support Routines) of the handout has information about
 * the functions in mminline.h and memlib.h
 */
#include "./memlib.h"
#include "./mm.h"
#include "./mminline.h"

#define EXTENSION (2 * MINBLOCKSIZE)
static block_t* flist_first;
block_t *prol;
block_t *epil;

// rounds up to the nearest multiple of WORD_SIZE
static inline size_t align(size_t size) {
  return (((size) + (WORD_SIZE - 1)) & ~(WORD_SIZE - 1));
}

/*
 *                             _       _ _
 *     _ __ ___  _ __ ___     (_)_ __ (_) |_
 *    | '_ ` _ \| '_ ` _ \    | | '_ \| | __|
 *    | | | | | | | | | | |   | | | | | | |_
 *    |_| |_| |_|_| |_| |_|___|_|_| |_|_|\__|
 *                       |_____|
 *
 * initializes the dynamic storage allocator (allocate initial heap space)
 * arguments: none
 * returns: 0, if successful
 *         -1, if an error occurs
 */
int mm_init(void) {
  prol = (block_t*) mem_sbrk(2*TAGS_SIZE);
  if (prol == (void *)-1) {
    return -1;
  }
  flist_first = NULL;
  block_set_size_and_allocated(prol, TAGS_SIZE, 1);
  epil = block_next(prol);
  block_set_size_and_allocated(epil, TAGS_SIZE, 1);
  return 0;
}

static inline block_t *extend_heap() {
  void* j = mem_sbrk(EXTENSION);
  if (j == (void *) -1) {
    return NULL;
  }
  block_set_size_and_allocated(epil, EXTENSION, 0);
  j = epil;
  insert_free_block(j);
  epil = block_next(epil);
  block_set_size_and_allocated(epil, TAGS_SIZE, 1);
  return epil;
}

static inline int isbig(block_t *fb, size_t size) {
  size_t bs = block_size(fb);
  if (bs >= size) {
    if (bs >= (size+MINBLOCKSIZE)) {
      return 1;
    }
    return 0;
  }
  return -1;
}

static inline block_t *search(size_t size) {
  block_t *fb = flist_first;
  size_t leftover;
  block_t *adjacent;

  do
  {
    int j = isbig(fb, size);

    if (j > -1) {
      if (j == 1) {
        //  ^^ free block has leftover space > MINBLOCKSPACE
        leftover = block_size(fb) - size;
        pull_free_block(fb);
        block_set_size_and_allocated(fb, size, 1);
        adjacent = block_next(fb);
        block_set_size_and_allocated(adjacent, leftover, 0);
        insert_free_block(adjacent);
        return fb;
      } else if (j == 0) {
        //  ^^ free block does not have leftover space > MINBLOCKSPACE
        pull_free_block(fb);
        block_set_allocated(fb, 1);
        return fb;
      }
    }
    fb = block_flink(fb);
  } while (fb != flist_first);
  
  return NULL;
}

/*     _ __ ___  _ __ ___      _ __ ___   __ _| | | ___   ___
 *    | '_ ` _ \| '_ ` _ \    | '_ ` _ \ / _` | | |/ _ \ / __|
 *    | | | | | | | | | | |   | | | | | | (_| | | | (_) | (__
 *    |_| |_| |_|_| |_| |_|___|_| |_| |_|\__,_|_|_|\___/ \___|
 *                       |_____|
 *
 * allocates a block of memory and returns a pointer to that block's payload
 * arguments: size: the desired payload size for the block
 * returns: a pointer to the newly-allocated block's payload (whose size
 *          is a multiple of ALIGNMENT), or NULL if an error occurred
 */
void *mm_malloc(size_t size) {
  // TODO
  block_t *p;
  size_t s = size+TAGS_SIZE;
  if (size == 0) {
    return NULL;
  } else {
      if (flist_first == NULL) {
        p = extend_heap();
        if (p == NULL) {
          return NULL;
        }
      }
      
      if (s < MINBLOCKSIZE) {
        s = MINBLOCKSIZE;
      } else {
        s = align(s);
      }
      block_t* fb = search(s);
      if (fb == NULL) {
        p = extend_heap();
        if (p == NULL) {
          return NULL;
        }
        fb = search(s);
      }
      block_t *payload = payload_to_block(fb);
      fprintf(stderr, "payload address: %lx\n", (long unsigned) payload);
      return payload;
  }
  return NULL;
}



/*                              __
 *     _ __ ___  _ __ ___      / _|_ __ ___  ___
 *    | '_ ` _ \| '_ ` _ \    | |_| '__/ _ \/ _ \
 *    | | | | | | | | | | |   |  _| | |  __/  __/
 *    |_| |_| |_|_| |_| |_|___|_| |_|  \___|\___|
 *                       |_____|
 *
 * frees a block of memory, enabling it to be reused later
 * arguments: ptr: pointer to the block's payload
 * returns: nothing
 */
void mm_free(void *ptr) {
  // TODO
  ptr = ptr;
}

/*
 *                                            _ _
 *     _ __ ___  _ __ ___      _ __ ___  __ _| | | ___   ___
 *    | '_ ` _ \| '_ ` _ \    | '__/ _ \/ _` | | |/ _ \ / __|
 *    | | | | | | | | | | |   | | |  __/ (_| | | | (_) | (__
 *    |_| |_| |_|_| |_| |_|___|_|  \___|\__,_|_|_|\___/ \___|
 *                       |_____|
 *
 * reallocates a memory block to update it with a new given size
 * arguments: ptr: a pointer to the memory block's payload
 *            size: the desired new payload size
 * returns: a pointer to the new memory block's payload
 */
void *mm_realloc(void *ptr, size_t size) {
  // TODO
  ptr = ptr;
  size = size;
  return NULL;
}