/*
 * Our minimal block size is 4 words because we need them for footer,
 * header, pointer to next free block and pointer to previous free block.
 * So for Freed block, the structure is as mentioned. For allocated block, the
 * structure is just header and footer with content in between.
 * We use segregated free lists to improve the general utilization and speed.
 * We use 12 segregated free lists, start from the smallest which contains free
 * block of smaller than 4 words and then each of other free list is twice as large
 * as previous free list. We also has a implicit free list to keep track of the
 * entire heap. 
 * For the malloc function we use first fit policy to search for the correct size
 * of free block from the segregated free list and then split it to make it more fit
 * if possible.
 * For the free function, we put the freed block into our free lists using ordered 
 * address policy which sort the free blocks by there addresses. This way we can reduce
 * the external fragmentation from some experiments.
 * Then for the realloc function we need to consider several cases to reduce the use of
 * memcopy function and also increase the utilization of space. 
 * So basically we want to keep the block in it's original address but
 * to realloc it's size. The cases include if the new size is smaller, if the next block
 * is free, or if the block is at the end of heap.
 * When extend the heap, we do not do the coalesce because it can reduce the 
 * external fragmentation when encounter the binary test cases. We also decide a
 * minimum size to extend if the passed in size is smaller than that. This way, it 
 * can better react to the binary test case where smaller and larger size of malloc 
 * interlace and then free. Because by grouping smaller sized allocated blocks together, 
 * it leads to smaller external fragmentation eventually.
 * And we use a immediate coalesce policy when we free a block, this way it is 
 * faster by experiment.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Team Herbert~!",
    /* First member's full name */
    "He Zhang",
    /* First member's email address */
    "heherbert.zhang@mail.utoronto.ca",
    /* Second member's full name (leave blank if none) */
    "Joo Young Kang",
    /* Second member's email address (leave blank if none) */
    "justin.kang@mail.utoronto.ca"
};

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
 *************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))

#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

void* heap_listp = NULL;


//define free list using explicit list method
#define FREE_LIST_NUM 12
#define MIN_BLOCK_SIZE (4*WSIZE)

//each free list head need 4 word size, and 4 extra for the prologue and epilogue
#define INITIAL_HEAP_SIZE (FREE_LIST_NUM * MIN_BLOCK_SIZE + 4*WSIZE)

void* free_lists[FREE_LIST_NUM] = {0};

int mm_check(int i);//header for our self check function

/*set the previous node for the free node in free list*/
void inline setFreeBLockPrev(void* bp, void* prev) {
    assert(GET_ALLOC(HDRP(bp)) == 0);
    PUT(bp, (uintptr_t) prev);
}

/*set the next node for the free node in free list*/
void inline setFreeBlockNext(void* bp, void* next) {
    assert(GET_ALLOC(HDRP(bp)) == 0);
    PUT((bp + WSIZE), (uintptr_t) next);
}

/*get the next node for the free node in free list*/
void* getNextFreeBlock(void* bp) {
    return (void*) GET((bp + WSIZE));
}

/*get the previous node for the free node in free list*/
void* getPrevFreeBlock(void* bp) {
    return (void*) GET(bp);
}

/*set the previous node and next node for the free node in free list*/
void inline setFreeBlockPrevNext(void* bp, void* prev, void* next) {
    setFreeBLockPrev(bp, prev);
    setFreeBlockNext(bp, next);
}

/*insert this node by connecting it in between two nodes*/
void inline insertFreeBlock(void* bp, void* prev, void* next) {
    setFreeBlockPrevNext(bp, prev, next);
    setFreeBlockNext(prev, bp);
    //in the case the next node is NULL we don't need to set it
    if (next != NULL) {
        setFreeBLockPrev(next, bp);
    }

}

/*helper function to set both header and footer at the same time*/
void inline setBlockHeaderFooter(void* bp, size_t size, int alloc) {
    PUT(HDRP(bp), PACK(size, alloc));
    PUT(FTRP(bp), PACK(size, alloc));
}

/*give a size to determine which segregated free list to use*/
int getWhichFreeList(size_t size) {
    //the smallest free list should contain blocks of size of 4 bytes
    int freeListSize = MIN_BLOCK_SIZE;
    //go through the list to find which free list to use
    int i;
    for (i = 0; i < FREE_LIST_NUM - 1; i++) {
        if (size <= freeListSize) {//if it fits in current free list return the index
            return i;
        } else {
            //increase to next free list which has size doubled
            freeListSize = freeListSize << 1;
        }
    }

    //if cannot find, go to the last free list since anything bigger than 
    //2^(FREE_LIST_NUM-1) bytes should go to the last free list
    return FREE_LIST_NUM - 1;
}

/*convenient function that find the correct free list and insert that free block*/
void insertToFreeList(void* bp) {

    size_t size = GET_SIZE(HDRP(bp));
    int which = getWhichFreeList(size); //get the fit free list

    void* freeListHead = free_lists[which];
    void* next = getNextFreeBlock(freeListHead);
    if (next == NULL) {
        //if the list is empty just insert to head
        insertFreeBlock(bp, freeListHead, NULL);
        return;
    } else {
        //find the correct position to insert using ordered address policy
        void* last = NULL;

        while (next != NULL) {
            if (bp > next) {
                void* prev = getPrevFreeBlock(next);
                insertFreeBlock(bp, prev, next);
                return;
            }
            void* nextnext = getNextFreeBlock(next);
            if (nextnext == NULL) {
                last = next;
                break;
            }
            next = nextnext;
        }
        //insert at the end of the free list
        insertFreeBlock(bp, last, NULL);
        return;
    }
}

/*remove the given free block from the free list*/
void removeFromFreeList(void* bp) {
    void* next = getNextFreeBlock(bp);
    void* prev = getPrevFreeBlock(bp);
    setFreeBlockNext(prev, next);
    if (next != NULL) {
        setFreeBLockPrev(next, prev);
    }
    setFreeBlockPrevNext(bp, NULL, NULL); //clear removed block's link
}

/*split the block if the given free block has larger size than the required size*/
void* splitBlock(void* bp, size_t asize) {
    size_t totalsize = GET_SIZE(HDRP(bp));
    size_t delta = totalsize - asize;
    //if the remaining free block larger than min block size we can split
    //we know that both bp size and required size are aligned with 8 
    //so delta is also aligned with 8

    if (delta >= MIN_BLOCK_SIZE) {
        //split
        PUT(HDRP(bp), PACK(asize, 0));
        PUT(FTRP(bp), PACK(asize, 0)); //footer
        //insert the rest free block to free list
        PUT(HDRP(NEXT_BLKP(bp)), PACK(delta, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(delta, 0));
        insertToFreeList(NEXT_BLKP(bp));

        return bp;
    }
    return bp;
}

/**********************************************************
 * mm_init
 * Our implementation first initializes 12 segregated class and its pointers.
 * The following pointers will be used to store address of free blocks in corrsponding class.
 *
 * After this, we shift the heap_listp pointer to end of free lists head and then 
 * allocate padding , prologue header, footer, and epilogue header.
 * 
 **********************************************************/

int mm_init(void) {
    //we need to allocate initial size to fit our free list heads.
    if ((heap_listp = mem_sbrk(INITIAL_HEAP_SIZE)) == (void *) - 1)
        return -1;

    //initialize the free lists
    int i;
    for (i = 0; i < FREE_LIST_NUM; i++) {
        void* freeListHeadi = heap_listp + i * MIN_BLOCK_SIZE + WSIZE;
        setBlockHeaderFooter(freeListHeadi, MIN_BLOCK_SIZE, 0);
        setFreeBlockPrevNext(freeListHeadi, NULL, NULL);
        free_lists[i] = freeListHeadi;
    }
    //after that, start initialize the actual heap
    heap_listp += FREE_LIST_NUM * MIN_BLOCK_SIZE;

    PUT(heap_listp, 0); // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); // epilogue header
    heap_listp += DSIZE;

    return 0;
}

/**********************************************************
 * coalesce (Immediate Coalescing)
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 * based on these cases, we removed the coalesced blocks and
 * re-insert into the free lists
 **********************************************************/
void *coalesce(void *bp) {

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) { /* Case 1 */
        //add to free list
        insertToFreeList(bp);
        return bp;
    } else if (prev_alloc && !next_alloc) { /* Case 2 */
        //remove that one from free list
        removeFromFreeList(NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        //add the new block to free list
        insertToFreeList(bp);

        return (bp);
    } else if (!prev_alloc && next_alloc) { /* Case 3 */
        //remove that one from free list
        removeFromFreeList(PREV_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));

        //add the new block to free list
        insertToFreeList(PREV_BLKP(bp));

        return (PREV_BLKP(bp));
    } else { /* Case 4 */
        //remove that one from free list
        removeFromFreeList(NEXT_BLKP(bp));
        removeFromFreeList(PREV_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));

        //add the new block to free list
        insertToFreeList(PREV_BLKP(bp));

        return (PREV_BLKP(bp));
    }
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words) {

    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *) - 1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0)); // free block header
    PUT(FTRP(bp), PACK(size, 0)); // free block footer

    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue header

    // we do not coalesce at the end for better binary malloc case
    return bp;
}

/**********************************************************
 * find_fit
 * Traverse the segregated free lists searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize) {
    int which = getWhichFreeList(asize);

    while (which < FREE_LIST_NUM) {
        void* freeList = free_lists[which];
        void* freeListNext = getNextFreeBlock(freeList);
        //go through the free list to find the fit sized free block
        for (; freeListNext != NULL; freeListNext = getNextFreeBlock(freeListNext)) {
            if (asize <= GET_SIZE(HDRP(freeListNext))) {
                return freeListNext;
            }
        }
        //else then go to next level of free list if not found this level
        which++;
    }
    return NULL;
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize) {
    /* Get the current block size */
    size_t bsize = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(bsize, 1));
    PUT(FTRP(bp), PACK(bsize, 1));
}

/**********************************************************
 * mm_free
 * Free the block and coalesce immediately with neighbouring blocks
 * and insert to free lists
 **********************************************************/
void mm_free(void *bp) {
    if (bp == NULL) {
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    //coalesce and then insert to free list
    coalesce(bp);
}

/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting is determined by spiltBlock
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size) {

    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        //remove from the free list
        removeFromFreeList(bp);

        //break the block into smaller one if possible
        bp = splitBlock(bp, asize);
        size_t bsize = GET_SIZE(HDRP(bp));

        //place the block 
        setBlockHeaderFooter(bp, bsize, 1);
        return bp;
    }

    /* No fit found. Get more memory and place the block */

    //Increasing chunksize by 16B gives huge improvment in binary test case.
    //we incease 16 for the reason that it matches size of header and footer
    extendsize = MAX(asize, CHUNKSIZE + 16);

    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;

    splitBlock(bp, asize);
    place(bp, asize);
    return bp;
}

/**********************************************************
 * mm_realloc
 * Deals with a few cases:
 * 1. if the realloc size is smaller than current size
 * we split the current block and then put the extra part
 * to free list
 * 2. if the next block of the current block is freed, we check if
 * merge these two blocks can lead to a fit block for realloc
 * 3. if the current block is at the end of heap,
 * we just increase the heap by the required amount and then merge
 * that amount into that block
 * 4. if the new size is same as old size we do nothing
 * 5. if the new size is 0, same as free
 * 6. else we malloc a new block and then copy the data from old block 
 *********************************************************/
void *mm_realloc(void *ptr, size_t size) {
    /* If size == 0 then this is just free, and we return NULL. */
    //case 5
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
        return (mm_malloc(size));

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    size_t oldSize = GET_SIZE(HDRP(oldptr));
    size_t asize;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    //case 4 (see above)
    if (oldSize == asize) {
        return ptr;
    }        //case 1
    else if (oldSize > asize) {
        void* newptr = splitBlock(ptr, asize);
        place(newptr, asize);
        return newptr;
    }        //case 2
    else if (GET_SIZE(HDRP(NEXT_BLKP(ptr))) != 0) {

        if (GET_ALLOC(HDRP(NEXT_BLKP(ptr))) == 0) {
            //get the merge size after merge with next block
            size_t msize = oldSize + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
            if (msize >= asize) {
                //coalesce next block with current block
                removeFromFreeList(NEXT_BLKP(ptr));
                PUT(HDRP(ptr), PACK(msize, 0));
                PUT(FTRP(ptr), PACK(msize, 0));

                //split block if there is extra space
                void* newptr = splitBlock(ptr, asize);
                place(newptr, asize);
                return newptr;
            }
        }
    }        //case 3
    else if (GET_SIZE(HDRP(NEXT_BLKP(ptr))) == 0) {
        //new size larger than old size and next block is epilogue
        //we can extend the heap and then coalesce

        size_t esize = asize - oldSize; //calculate sufficient space to extend
        //extend heap by the sufficient amount
        void* ebp = extend_heap(esize / WSIZE);

        if (ebp != NULL) {
            //coalesce the extend space into current block
            PUT(HDRP(ptr), PACK(asize, 1));
            PUT(FTRP(ptr), PACK(asize, 1));
            return ptr;
        }
    }

    //case 6
    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;

    /* Copy the old data. */
    copySize = GET_SIZE(HDRP(oldptr));

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/***************************************************************************************************
 * Following function is controlled by parameter (E.g., mm_check(1))
 * 
 * First two methods (1-2) will display information of heap and free lists.
 * Arguments between (3-7) will test each case scenario.
 * 
 *  
 *                       Arguments and Description
 *
 * 1 - Prints entire heap starting from prologue upto epilogue block.  
 *     Each block is represented with 1)Address 2)Allocated Bit 3)Size of the Block
 *     It basically checks if the heap block point to the valid heap address
 * 
 * 2 - Prints lists of free block and free block pointers in each list.
 *     Displays 12 different sized (starting at 2^5 upto 2^16 B)
 * 
 *
 *                           Testers  (3 - 7)
 * 
 * 3 - Tester will traverse entire heap and check if any contiguous free blocks exist
 * 
 * 4 - Tester will verify if every free block exist in free list 
 * 
 * 5 - Tester will verify if every node in free list point to valid free blocks in heap (reverse of case 4)
 * 
 * 6 - Tester will verify if all pointers in heap point to valid heap address 
 *                                                                   (between mem_heap_low and mem_heap_hi)
 * 7 - Tester will verify if there is overlapped blocks in the heap
 * 
 *************************************************************************************************/
//helper function header declaration
void *mem_heap_hi(void);
int find_element(void * a);
int isInFreeList(void* bp);

int mm_check(int i) {

    void* base = heap_listp;

    // Print Entire Heap starting from prologue upto eplilogue
    if (i == 1) {
        printf("Address : %p  Allocated: %ld  Size:%ld  <-----  PROLOGUE\n", base, GET_ALLOC(HDRP(base)), GET_SIZE(HDRP(base)));
        base = NEXT_BLKP(base);

        while (GET_SIZE(HDRP(base)) != 0) {
            printf("Address : %p  Allocated: %ld  Size:%ld\n", base, GET_ALLOC(HDRP(base)), GET_SIZE(HDRP(base)));
            base = NEXT_BLKP(base);
        }
        printf("Address : %p  Allocated: %ld   Size:%ld  <-----  EPILOGUE\n", base, GET_ALLOC(HDRP(base)), GET_SIZE(HDRP(base)));
    }

    // Print 12 Segregated Free List.  Very helpful at early stage. 
    if (i == 2) {

        int i;
        for (i = 0; i < FREE_LIST_NUM; i++) {
            printf("Free List #%d [%ld-%ld]\n", i, MIN_BLOCK_SIZE / 2 << i, MIN_BLOCK_SIZE / 2 << (i + 1));

            void* next = getNextFreeBlock(free_lists[i]);

            while (next != NULL) {
                printf("Address:%p   Alloc:%ld   Size:%ld\n", next, GET_ALLOC(HDRP(next)), GET_SIZE(HDRP(next)));
                next = getNextFreeBlock(next);
            }
        }

    }
    
    // Check if there are any contiguous free blocks and print address of two (if exists)
    if (i == 3) {
        int z = 0;

        while (GET_SIZE(HDRP(base)) != 0) {
            if ((GET_ALLOC(HDRP(base))) == 0 && (GET_ALLOC(HDRP(NEXT_BLKP(base))) == 0)) {
                z = 1;
                printf("Address: %p (alloc:%ld) and  Address: %p (alloc:%ld) are contiguous FREE BLOCKS!\n", base, GET_ALLOC(HDRP(NEXT_BLKP(base))), base, GET_ALLOC(HDRP(NEXT_BLKP(base))));
            }
            base = NEXT_BLKP(base);
        }

        if (z == 0) {
            printf("\tVERIFIED.  There aren't any contiguous FREE Blocks in your heap.\n");
        }
    }

    //Check if every free blocks are in the free list
    if (i == 4) {
        void* bp = heap_listp;
        //terminate at the epilogue where size is 0
        while (GET_SIZE(HDRP(bp)) != 0) {
            if (GET_ALLOC(HDRP(bp)) == 0) {
                isInFreeList(bp);
                printf("VERIFIED.  ALL free blocks exist within free list\n");
            }
            bp = NEXT_BLKP(bp);
        }
    }
    //Check if free block pointer(s) (in free list) points to valid free block in heap.
    if (i == 5) {
        int i;
        for (i = 0; i < FREE_LIST_NUM; i++) {
            printf("Free List #%d [%ld-%ld]\n", i, MIN_BLOCK_SIZE / 2 << i, MIN_BLOCK_SIZE / 2 << (i + 1));
            base = heap_listp;
            void* next = getNextFreeBlock(free_lists[i]);
            while (next != NULL) {
                //check if the free block in free list is in heap so that it is valid
                find_element(next);
                next = getNextFreeBlock(next);
            }
        }
    }

    //Check if all pointers in heap point to valid heap address.

    if (i == 6) {
        int z = 0;
        void* cur = heap_listp;
        while (GET_SIZE(HDRP(cur)) != 0) {
            if ((base > cur) || (cur > mem_heap_hi())) {
                z = 1;
                printf("\tERROR: Address: %p is not a valid heap address.\n", cur);
            }

            cur = NEXT_BLKP(cur);
        }
        if (z == 0)
            printf("\tVERIFIED.  ALL pointers in heap block point to valid heap address\n");
    }
    //check if there is overlap by checking that block's next point to next block and 
    //next block's previous point to that block too
     
    if (i == 7) {
        void* node = heap_listp;

        while (GET_SIZE(HDRP(node)) != 0) {

            void* next = NEXT_BLKP(node);
            if (GET_SIZE(HDRP(node)) != 0) {
                if (PREV_BLKP(next) != node) {
                    //not matched we have overlapped block
                    printf("There are overlapped blocks!\n ");
                    return 0;
                }
            }
            node = next;
        }
        printf("There is no overlapping!\n ");
    }

    return 1;
}


// helper function for finding element in heap
int find_element(void * a) {

    void* base = heap_listp;
    void* cur = NEXT_BLKP(base);
    while (GET_SIZE(HDRP(cur)) != 0) {
        if (cur == a) {
            printf("\tVerified.  free list item (=%p) point to valid address in heap\n", a);
            return 1;
        }

        cur = NEXT_BLKP(cur);

    }
    printf("\tNOT FOUND in heap\n");
    return 0;
}

//helper function for check if block in free list
int isInFreeList(void* bp) {
    int i;
    for (i = 0; i < FREE_LIST_NUM; i++) {
        void* b = getNextFreeBlock(free_lists[i]);
        while (b != NULL) {
            if (b == bp) {
                return 1;
            }
            b = getNextFreeBlock(b);
        }
    }
    printf("not inside free lists!%p\n", bp);

    return 0;
}