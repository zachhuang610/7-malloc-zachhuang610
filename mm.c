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

#define EXTENSION (32 * MINBLOCKSIZE)
static block_t *flist_first;
block_t *prol;
block_t *epil;
#define THRESHOLD (2 * MINBLOCKSIZE)

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
    prol = (block_t *)mem_sbrk(2 * TAGS_SIZE);
    if (prol == (void *)-1) {
        return -1;
    }
    flist_first = NULL;
    block_set_size_and_allocated(prol, TAGS_SIZE, 1);
    epil = block_next(prol);
    block_set_size_and_allocated(epil, TAGS_SIZE, 1);
    return 0;
}

/*
coalesce function, takes in a free block.
*/
static inline void coalesce(block_t *fb) {
    size_t s = block_size(fb);

    block_t *b = fb;
    if (block_prev_allocated(b) == 0) {
        b = block_prev(b);
        s += block_size(b);
        pull_free_block(b);
    }

    block_t *f = fb;
    if (block_next_allocated(f) == 0) {
        f = block_next(f);
        s += block_size(f);
        pull_free_block(f);
    }

    block_set_size_and_allocated(b, s, 0);
    insert_free_block(b);
}

/*
function for extending heap by EXTENSION number of bytes or by size_t size bytes, whichever is larger.
*/
static inline block_t *extend_heap(size_t size) {
    size_t s;
    if (EXTENSION >= size) {
        s = EXTENSION;
    } else {
        s = size;
    }
    void *err = mem_sbrk(s);
    if (err == (void *)-1) {
        return NULL;
    }
    block_set_size_and_allocated(epil, s, 0);
    epil = block_next(epil);
    block_set_size_and_allocated(epil, TAGS_SIZE, 1);
    coalesce(block_prev(epil));
    return epil;
}

/*
function for checking if the block size is big enough for the
requested block size.
returns -1 if block is too small, 0 if all of block must be allocated
and 1 if only a portion of the block must be allocated.
*/
static inline int isbig(size_t bs, size_t size) {
    if (bs >= size) {
        if (bs >= (size + THRESHOLD)) {
            return 1;
        }
        return 0;
    }
    return -1;
}

/*
search: a function for looking through flist to identify a free block.
returns pointer to free block of sufficient size, NULL otherwise.
*/
static inline block_t *search(size_t size) {
    block_t *fb = flist_first;
    size_t leftover;
    block_t *adjacent;
    do {
        int j = isbig(block_size(fb), size);
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
    block_t *err;
    size_t s = align(size) + TAGS_SIZE;
    if (size == 0) {
        return NULL;
    } else {
        if (flist_first == NULL) {
            err = extend_heap(s);
            if (err == NULL) {
                return NULL;
            }
        }

        if (s < MINBLOCKSIZE) {
            s = MINBLOCKSIZE;
        }

        block_t *fb = search(s);
        while (fb == NULL) {
            err = extend_heap(s);
            if (err == NULL) {
                return NULL;
            }
            fb = search(s);
        }
        return (fb->payload);
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
    block_t *b = payload_to_block(ptr);
    block_set_allocated(b, 0);
    coalesce(b);
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
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    if (ptr == NULL) {
        return mm_malloc(size);
    }

    block_t *ab = payload_to_block(ptr);
    size = align(size);
    size_t block_s = size + TAGS_SIZE;

    if (block_s <= MINBLOCKSIZE) {
        block_s = MINBLOCKSIZE;
    }

    if (block_s <= block_size(ab)) {
        if (isbig(block_size(ab), block_s) == 1) {
            size_t leftover = block_size(ab) - block_s;
            block_set_size_and_allocated(ab, block_s, 1);
            block_t *next = block_next(ab);
            block_set_size_and_allocated(next, leftover, 0);
            mm_free((next->payload));
            return ptr;
        }
        return ptr;
    }

    size_t max_s = block_size(ab);

    block_t *prev = ab;
    if (block_prev_allocated(ab) == 0) {
        prev = block_prev(ab);
    }
    block_t *next = ab;
    if (block_next_allocated(ab) == 0) {
        max_s += block_next_size(ab);
        next = block_next(ab);
    }
    // checks to see if nearby free space can fit realloc size
    int j = isbig(max_s, block_s);
    if (j == -1) {
        size_t payload_size = block_size(ab) - TAGS_SIZE;
        if (prev != ab) {
            size_t total_size = block_size(prev) + max_s;
            if (total_size >= block_s) {
                if (next != ab) {
                    pull_free_block(next);
                }
                if (total_size >= (block_s + THRESHOLD)) {
                    size_t leftover = total_size - block_s;
                    block_set_size(prev, leftover);
                    block_t *newblock = block_next(prev);
                    memmove((newblock->payload), ptr, payload_size);
                    block_set_size_and_allocated(newblock, block_s, 1);
                    return (newblock->payload);
                }
                pull_free_block(prev);
                memmove((prev->payload), ptr, payload_size);
                block_set_size_and_allocated(prev, total_size, 1);
                return (prev->payload);
            }
        }
        block_t *fb = mm_malloc(size);
        memcpy(fb, ptr, payload_size);
        mm_free(ptr);
        return fb;
    } else if (j == 0) {
        // requested realloc size needs all of the space
        if (next != ab) {
            pull_free_block(next);
        }
        block_set_size_and_allocated(ab, max_s, 1);
        return ptr;
    } else if (j == 1) {
        // fits with splitting on leftover
        pull_free_block(next);
        size_t leftover = max_s - block_s;
        block_set_size(ab, block_s);
        block_t *adjacent = block_next(ab);
        block_set_size_and_allocated(adjacent, leftover, 0);
        coalesce(adjacent);
        return ptr;
    }
    return NULL;
}