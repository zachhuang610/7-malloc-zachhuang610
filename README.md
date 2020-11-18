# 7-malloc

Heap Maintanance:
To prevent the heap from turning into a bunch of tiny free blocks, I 
implemented a coalesce, a function which merges adjacent free blocks
into a larger free block. This function is called whenever a new free
block is created. This occurs during mm_free and mm_realloc. This
does not occur in mm_malloc because at worst, we are shrinking a free block,
not creating a new one. 

mm_realloc implementation:
    First, realloc checks for the edge cases of ptr = NULL and size = 0. 
    Then it checks if the block should be shrunk. If it should be shrunk,
it does so only if the remaining space is enough to create a free block.
Otherwise, it just returns the ptr to the block. 
    Since it fails these two cases, we know that the request is to expand
the allocated block. Then we check to see if there are any adjacent free
blocks, and if expanding into them will be sufficient. If additional
adjacent space is enough, we call memcpy to move the payload from the 
original block to a new payload pointer  which will fit all of the requested
size. If there is splitting, we designate the higher (more prev) block
as the free block, and mvoe to the next block from the newly created free block.
otherwise, we allocate the entire adjacent free space to the block.
    If adjacent space is not enough, we call malloc and memcpy payload
contents into the new position.

BUGS: NONE

Optimizations:
    the way my realloc is structured is optimized to make sure that space
adjacent to the current block is not large enough to allocate. This ensures
that we utilize all existing space before mallocing and possibly extending
the heap.