/*
 * This implementation replicates the implicit list implementation
 * provided in the textbook
 * "Computer Systems - A Programmer's Perspective"
 * Blocks are never coalesced or reused.
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
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
    "abc",
    /* First member's full name */
    "He Zhang",
    /* First member's email address */
    "heherbert.zhang@mail.utoronto.ca",
    /* Second member's full name (leave blank if none) */
    "Joo Young Kang",
    /* Second member's email address (leave blank if none) */
    "asd"
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
//?????shouldn't this be: (GET(P) & ~1)?????
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

void inline setFreeBLockPrev(void* bp, void* prev) {
    assert(GET_ALLOC(HDRP(bp)) == 0);
    PUT(bp, (uintptr_t)prev);
}

void inline setFreeBlockNext(void* bp, void* next) {
    assert(GET_ALLOC(HDRP(bp)) == 0);
    PUT((bp + WSIZE), (uintptr_t)next);
}

void* getNextFreeBlock(void* bp){
    return (void*)GET((bp+WSIZE));
}

void* getPrevFreeBlock(void* bp){
    return (void*)GET(bp);
}

void inline setFreeBlockPrevNext(void* bp, void* prev, void* next) {
    setFreeBLockPrev(bp, prev);
    setFreeBlockNext(bp, next);
}

void inline insertFreeBlock(void* bp, void* prev, void* next) {
    setFreeBlockPrevNext(bp, prev, next);
    setFreeBlockNext(prev, bp);
    if(next != NULL){
        setFreeBLockPrev(next, bp);
    }
    //printf("insert free do end \n");

}

void inline setBlockHeaderFooter(void* bp, size_t size, int alloc) {
    PUT(HDRP(bp), PACK(size, alloc));
    PUT(FTRP(bp), PACK(size, alloc));
}

int getWhichFreeList(size_t size){
    //the smallest free list should contain blocks of size of 4 bytes
    int freeListSize = MIN_BLOCK_SIZE;
    //go through the list to find which free list to use
    int i;
    for(i = 0; i < FREE_LIST_NUM-1; i++){
        if(size <= freeListSize){
            return i;
        }
        else
        {
            //increase to next free list which has size doubled
            freeListSize = freeListSize << 1;
        }
    }
    /*
    int middleIndex = FREE_LIST_NUM >> 1;
    int maxIndex = MIN_BLOCK_SIZE;
    int minIndex = 0;
    size_t middlesize = MIN_BLOCK_SIZE << middleIndex;
    size_t lowerboundsize = middlesize >> 1;
    if(size <= MIN_BLOCK_SIZE){
        return 0;
    }
    while(1){
        if(middlesize >= FREE_LIST_NUM-1){
            return FREE_LIST_NUM-1;
        }
        if(size > middlesize){
            minIndex = middleIndex;
            middleIndex = minIndex + ((maxIndex - minIndex) >> 1); 
            middlesize = middlesize << middleIndex;
            lowerboundsize = middlesize >> 1;
        }
        else if(size >= lowerboundsize && size <= middlesize){
            return middleIndex;
        }
        else{

            maxIndex = middleIndex;
            middleIndex = maxIndex - ((maxIndex - minIndex) >> 1); 
            middlesize = middlesize << middleIndex;
            lowerboundsize = middlesize >> 1;
        }
    }*/
    //if cannot find, go to the last free list since anything bigger than 
    //2^(FREE_LIST_NUM-1) bytes should go to the last free list
    return FREE_LIST_NUM-1;
}

void insertToFreeList(void* bp){
    //printf("insert free\n");
    size_t size = GET_SIZE(HDRP(bp));
    int which = getWhichFreeList(size);
    /*if(which == FREE_LIST_NUM -1){
        //this is a binary search tree
         void* node = free_lists[which];
         void* larger = getNextFreeBlock(node);
         void* smaller = getPrevFreeBlock(node);
         size_t currentSize = GET_SIZE(HDRP(node));
         //if head is empty insert into right side
         if(larger == NULL && smaller == NULL){
            //go to left side if smaller or equal
            if(size <= currentSize){
                setFreeBLockPrev(node, bp);
            }
            else{
                setFreeBlockNext(node, bp);
            }

            return;
        }
        else{
            
            if(size > currentSize){
                getNextFreeBlock(fre)
            }
        }
    }*/
    

    void* freeListHead = free_lists[which];
    void* next = getNextFreeBlock(freeListHead);
    if(next == NULL){
        //printf("insert free initial end\n");
        insertFreeBlock(bp, freeListHead, NULL);
        return;
    }
    else{
        /*void* prev = freeListHead;
        while(next != NULL){
            //sort by their size
            if(GET_SIZE(HDRP(bp)) >= GET_SIZE(HDRP(next))){
                insertFreeBlock(bp, prev, next);
                return;
            }
            else{
                void* nextnext = getNextFreeBlock(next);
                if(nextnext == NULL){
                    //insert to the end of list
                    insertFreeBlock(bp, next, NULL);
                    return;
                }
                else{
                    prev = next;
                    next = nextnext;
                }
            }
        }*/


        ///////////
        
        //find the correct position to insert
        void* last = NULL;
        
        while(next != NULL){
            if(bp > next){
                void* prev = getPrevFreeBlock(next);
                insertFreeBlock(bp, prev, next);
                return;
            }
            void* nextnext = getNextFreeBlock(next);
            if(nextnext == NULL){
                last = next;
                break;
            }
            next = nextnext;
        }
        //insert at the end of the free list
        //printf("insert free end\n");
        insertFreeBlock(bp, last, NULL);
        //insertFreeBlock(bp, freeListHead, next);
        return;
    }
}

void removeFromFreeList(void* bp){
    //printf("remove form free list\n");
    void* next = getNextFreeBlock(bp);
    void* prev = getPrevFreeBlock(bp);
    setFreeBlockNext(prev, next);
    if(next != NULL){
        setFreeBLockPrev(next, prev);
    }
    setFreeBlockPrevNext(bp, NULL, NULL);
}

void* splitBlock(void* bp, size_t asize){
    size_t totalsize = GET_SIZE(HDRP(bp));
    size_t delta = totalsize - asize;
    if(delta >= MIN_BLOCK_SIZE){
        //split
        PUT(HDRP(bp), PACK(asize, 0));
        //printf("asize %d\n", (int)asize);
        //void* footer = bp + asize - DSIZE;
        PUT(FTRP(bp), PACK(asize, 0));//footer
        //the rest block
        //void* theRest = footer+DSIZE;
        //void* restFooter = theRest + delta - DSIZE;
        PUT(HDRP(NEXT_BLKP(bp)), PACK(delta, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(delta, 0));
        insertToFreeList(NEXT_BLKP(bp));
        return bp;
    }
    return bp;
}

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
/*
int mm_init(void)
{
  //we need to allocate initial size to fit our free list heads.
  if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header
    heap_listp += DSIZE;

    return 0;
}*/

int mm_init(void) {
    //we need to allocate initial size to fit our free list heads.
    if ((heap_listp = mem_sbrk(INITIAL_HEAP_SIZE)) == (void *) - 1)
        return -1;
    
    //initialize the free lists
    int i;
    for(i = 0; i < FREE_LIST_NUM; i++){
        void* freeListHeadi = heap_listp+i*MIN_BLOCK_SIZE+WSIZE;
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
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(void *bp) {
    //printf("coalesce\n");
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) { /* Case 1 */
        //printf("coalesce 1\n");
        //add to free list
        insertToFreeList(bp);
        return bp;
    }
    else if (prev_alloc && !next_alloc) { /* Case 2 */
        //remove that one from free list
       // printf("coalesce 2\n");
        removeFromFreeList(NEXT_BLKP(bp));
        
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        
        //add the new block to free list
        insertToFreeList(bp);
        
        return (bp);
    }
    else if (!prev_alloc && next_alloc) { /* Case 3 */
        //printf("coalesce 3\n");
        //remove that one from free list
        removeFromFreeList(PREV_BLKP(bp));
        
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        
        //add the new block to free list
        insertToFreeList(PREV_BLKP(bp));
        
        return (PREV_BLKP(bp));
    }
    else { /* Case 4 */
        //printf("coalesce 4\n");
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
    //printf("ext\n");
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

    /* Coalesce if the previous block was free */
    //return coalesce(bp);
    return bp;
}

/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize) {
    /*void *bp;
    
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }*/
    //printf("find fit\n");
    int which = getWhichFreeList(asize);


    while(which < FREE_LIST_NUM){
        void* freeList  = free_lists[which];
        void* freeListNext = getNextFreeBlock(freeList);

        for(;freeListNext != NULL; freeListNext = getNextFreeBlock(freeListNext)){
        //if(freeListNext != NULL){
            if(asize <= GET_SIZE(HDRP(freeListNext))){
                return freeListNext;
            }
        //}
        }

        //else then go to next level of free list
        which++;
        //printf("which++ %d\n", which);
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
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp) {
    if (bp == NULL) {
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    //add to free list and coalesce
    coalesce(bp);
}

/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
 void *extend_heap_no_coalesce(size_t words);
void *mm_malloc(size_t size) {
    //printf("malloc\n");
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
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);//why dsize - 1

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        //place(bp, asize);
        //remove from the free list
        removeFromFreeList(bp);
        //break the block into smaller one if possible
        bp = splitBlock(bp, asize);
        size_t bsize = GET_SIZE(HDRP(bp));
        //printf("asize %d  bszie %d\n",(int)asize,  (int)bsize);
        //assert(bsize == asize);
        setBlockHeaderFooter(bp, bsize, 1);
         assert(GET_ALLOC(HDRP(bp)) == 1);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE+16);
    //printf("size:   esize:%d\n",extendsize);
    if ((bp = extend_heap_no_coalesce(extendsize / WSIZE)) == NULL)
        return NULL;
    //remove from the free list
    //printf("malloc extend heap and then remove\n");
    //removeFromFreeList(bp);
    splitBlock(bp, asize);
    place(bp, asize);
    assert(GET_ALLOC(HDRP(bp)) == 1);
    return bp;

}

void *extend_heap_no_coalesce(size_t words) {
    //printf("ext\n");
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

    /* Coalesce if the previous block was free */
    return bp;
}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size) {
    /* If size == 0 then this is just free, and we return NULL. */
    //printf("SIZE: %d\n",size);
    //mm_check(1);
    //printf("ptr is %p\n", ptr);
    assert(GET_ALLOC(HDRP(ptr)) == 1);
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
    size_t oldSize= GET_SIZE(HDRP(oldptr));
    size_t asize;

    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);//why dsize - 1

    if (oldSize == asize)
    {
        return ptr;
    }

    else if (oldSize > asize)
    {
        void* newptr = splitBlock(ptr,asize);
        place(newptr, asize);
        return newptr;
    }

    else if (GET_SIZE(HDRP(NEXT_BLKP(ptr))) != 0)
    {
        
        if (GET_ALLOC(HDRP(NEXT_BLKP(ptr))) == 0)
        {
            size_t msize = oldSize + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
            //printf("msize is : %d\n", msize);
            if (msize >= asize)
            {
                removeFromFreeList(NEXT_BLKP(ptr));
                PUT(HDRP(ptr), PACK(msize, 0));
                PUT(FTRP(ptr), PACK(msize, 0));
                void* newptr = splitBlock(ptr,asize);
                place(newptr, asize);
                return newptr;
            }
        //add the new block to free list
        }
    }
    
    
    else if (GET_SIZE(HDRP(NEXT_BLKP(ptr))) == 0){
        //new size larger than old size and next block is epilogue
        //we can extend the heap and then coalesce
        //mm_check(1);
        //printf("size: %d asize: %d \n", size, asize);
        assert(GET_ALLOC(HDRP(NEXT_BLKP(ptr))) == 1);
        size_t esize = asize - oldSize;

        //if (esize <= DSIZE)
        //esize = 2 * DSIZE;
        //else
        //esize = DSIZE * ((esize + (DSIZE) + (DSIZE - 1)) / DSIZE);//why dsize - 1
        //printf("asize:%d oldSize:%d esize:%d \n", asize,oldSize,esize);
    
        void* ebp = extend_heap_no_coalesce(esize/WSIZE);
        //printf ("size of new block:%d check if this is the same as esize\n", GET_SIZE(HDRP(ebp)));
        //printf ("ebp: 0x%x\n", ebp);

        //printf ("-----\n");
        //mm_check(1);
        //printf ("WTF\n");

        //assert(GET_SIZE(HDRP(ebp)) == esize);
        //printf("epilogue case check  \n");
        if(ebp !=NULL){
            //removeFromFreeList(ebp);
            //printf("epilogue case removefree\n");
            PUT(HDRP(ptr), PACK(asize, 1));
            PUT(FTRP(ptr), PACK(asize, 1));
            //printf("-----\n");
            return ptr;
        }
    }
    

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;

    /* Copy the old data. */
    copySize = GET_SIZE(HDRP(oldptr));
    //if (size < copySize)
        //copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(int i) {

void* base = heap_listp;

//PRINT ENTIRE HEAP
if (i==1)
{
    

    while (GET_SIZE(HDRP(base)) != 0)
    {
        printf("Address : 0x%x  Allocated: %d  Size:%d\n", base, GET_ALLOC(HDRP(base)), GET_SIZE(HDRP(base)));
        base = NEXT_BLKP(base);
    }
    printf("Address : 0x%x  Allocated: %d   Size:%d\n", base, GET_ALLOC(HDRP(base)), GET_SIZE(HDRP(base)));
}


if (i==2)
{

    int i;
    for(i=0;i<FREE_LIST_NUM;i++)
    {
        printf("Free List #%d [%d-%d]\n", i, MIN_BLOCK_SIZE/2<<i, MIN_BLOCK_SIZE/2<<(i+1));

        void* next = getNextFreeBlock(free_lists[i]);

        while (next!=NULL)
        {
            printf("Address:0x%x   Alloc:%d   Size:%d\n", next, GET_ALLOC(HDRP(next)), GET_SIZE(HDRP(next)));  
            next=getNextFreeBlock(next);
        }
    }
    
}


if (i==3)
{
    

    while (GET_SIZE(HDRP(base)) != 0)
    {
        if ((GET_ALLOC(HDRP(base)))==0&&(GET_ALLOC(HDRP(NEXT_BLKP(base)))==0))
        {
            printf("Address: 0x%x (alloc:%d) and  Address: 0x%x (alloc:%d) are contiguous FREE BLOCKS!\n",base,GET_ALLOC(HDRP(NEXT_BLKP(base))),base,GET_ALLOC(HDRP(NEXT_BLKP(base))));
        }
        base = NEXT_BLKP(base);
    }
}

if (i==4)
{

    while (GET_SIZE(HDRP(base)) != 0)
    {
        if (GET_ALLOC(HDRP(base))==0)
        {
        int k=getWhichFreeList(GET_SIZE(HDRP(base)));

        void* next = getNextFreeBlock(free_lists[k]);
        
        int z = 0;
        while (next!=NULL)
        {
            if (base == next)
            {
                z=1;
            }

        next=getNextFreeBlock(next);
        }

        if (z=0)
            printf("Free Block at 0x%x is not in the free list.\n", base);

        }
        base = NEXT_BLKP(base);
    }
}

    return 1;
}
