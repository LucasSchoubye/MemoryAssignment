#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>


/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

strategies myStrategy = NotSet;    // Current strategy

size_t mySize;
void *myMemory = NULL;

static MemList *head;
static MemList *tail;
static MemList *next;

/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeuction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
		- "best" (best-fit)
		- "worst" (worst-fit)
		- "first" (first-fit)
		- "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all mymalloc requests.
*/

void initmem(strategies strategy, size_t sz)
{
	myStrategy = strategy;

	/* all implementations will need an actual block of memory to use */
	mySize = sz;

	freeProgramMemory();

	myMemory = malloc(sz);

    // create a new MemList struct and make both head and tail pointers point to this
    head = malloc(sizeof (MemList));
    tail = head;
    next = NULL;

    // initialize values
    head->size = (int) mySize;
    head->alloc = 0;
    head->next = NULL;
    head->prev = NULL;
    head->ptr = myMemory;

    // for NextFit, we use a circular linked list  TODO - this code should probably be moved to somewhere else
    if(myStrategy == Next) {
        tail->next = head;
        head->prev = tail;
    }
}

int allocateMem(void *location, size_t requestedSize) {
    if(location == NULL)
        return -1; // -1 error code if no memory was allocated
    MemList *allocatedBlock = getStructPtr(location);
    while(allocatedBlock != NULL) { // traverse the list to find the block from where memory will be allocated
        if(allocatedBlock->ptr == location)
            break;
        allocatedBlock = allocatedBlock->next;
    }

    if(allocatedBlock == NULL)
        return NULL; // return null if block to be allocated does not exit

    allocatedBlock->alloc = 1;

    if(allocatedBlock->size == requestedSize) { // it is a perfect fit, allocate it all, and return
        return allocatedBlock->ptr;
    } else if (allocatedBlock->size < requestedSize)
        return NULL; // this should not happen if find() methods work correctly!
    else { // here we know there will be left-over memory after allocation, so we add a new struct after the allocatedBlock
        // create new struct
        MemList *newBlock = malloc(sizeof (MemList));

        // update pointers
        newBlock->next = allocatedBlock->next;
        newBlock->prev = allocatedBlock;
        if(allocatedBlock->next != NULL)
            allocatedBlock->next->prev = newBlock;
        allocatedBlock->next = newBlock;

        // initialize newBlock data
        newBlock->size = allocatedBlock->size - (int) requestedSize;
        newBlock->alloc = 0;
        newBlock->ptr = allocatedBlock->ptr + allocatedBlock->size; // next block's allocatedBlock is oldBlockLocation + oldBlockSize

        // update the size of the allocatedBlock
        allocatedBlock->size = (int)requestedSize;

        // update global tail and next pointers
        if(tail == allocatedBlock)
            tail = newBlock;
        next = newBlock;

        return allocatedBlock->ptr;
    }
}


/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1
 */

void *mymalloc(size_t requested)
{
	assert((int)myStrategy > 0);

	switch (myStrategy)
	  {
	  case NotSet:
	            return NULL;
	  case First:
          findFirstFit(requested);
	            return NULL;
	  case Best:
          findBestFit(requested);
	            return NULL;
	  case Worst:
          findWorstFit(requested);
	            return NULL;
	  case Next:
          //findNextFit(requested);
	            return NULL;
	  }
	return NULL;
}

/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void* block)
{
	return;
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when refered to "memory" here, it is meant that the 
 * memory pool this module manages via initmem/mymalloc/myfree. 
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes()
{
	return 0;
}

/* Get the number of bytes allocated */
int mem_allocated()
{
	return 0;
}

/* Number of non-allocated bytes */
int mem_free()
{
	return 0;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{
	return 0;
}

/* Number of free blocks smaller than or equal to "size" bytes. */
int mem_small_free(int size)
{
	return 0;
}       

char mem_is_alloc(void *ptr)
{
        return 0;
}

/* 
 * Feel free to use these functions, but do not modify them.  
 * The test code uses them, but you may find them useful.
 */


//Returns a pointer to the memory pool.
void *mem_pool()
{
	return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total()
{
	return mySize;
}


// Get string name for a strategy. 
char *strategy_name(strategies strategy)
{
	switch (strategy)
	{
		case Best:
			return "best";
		case Worst:
			return "worst";
		case First:
			return "first";
		case Next:
			return "next";
		default:
			return "unknown";
	}
}

// Get strategy from name.
strategies strategyFromString(char * strategy)
{
	if (!strcmp(strategy,"best"))
	{
		return Best;
	}
	else if (!strcmp(strategy,"worst"))
	{
		return Worst;
	}
	else if (!strcmp(strategy,"first"))
	{
		return First;
	}
	else if (!strcmp(strategy,"next"))
	{
		return Next;
	}
	else
	{
		return 0;
	}
}
// returns NULL pointer if no eligible block is found, otherwise returns pointer to the block to allocate
void* findFirstFit(size_t requested) {
    void *firstBlockPtr = NULL;
    MemList *current = head;

    while(current != NULL) {
        if(current->alloc == 0 && current->size >= requested) {
            firstBlockPtr = current->ptr;
        }
        current = current->next;
    }

    return firstBlockPtr;
}

// returns NULL pointer if no eligible block is found, otherwise returns pointer to the block to allocate
void* findWorstFit(size_t requested) {
    void *biggestBlockPtr = NULL;
    int biggestBlockSize = 0;
    MemList *current = head;
    while(current != NULL) {
        if(current->alloc == 0 && current->size >= requested && current->size > biggestBlockSize) {
            biggestBlockPtr = current->ptr;
            biggestBlockSize = current->size;
        }
        current = current->next;
    }
    return biggestBlockPtr;
}

// returns NULL pointer if no eligible block is found, otherwise returns pointer to the block to allocate
void* findBestFit(size_t requested) {
    void *bestBlockPtr = NULL;
    size_t smallestFeasibleBlock = mySize; // initialize to the largest possible value
    MemList *current = head;
    while(current != NULL) {
        if(current->alloc == 0 && current->size >= requested
        && (current->size < smallestFeasibleBlock || current->size == mySize)) {
            bestBlockPtr = current->ptr;
            smallestFeasibleBlock = current->size;
        }
        current = current->next;
    }
    return bestBlockPtr;
}
//TODO: implement next fit algorithm
void* findNextFit(size_t requested) {

}

void freeProgramMemory() {
    if (myMemory != NULL)
        free(myMemory); /* in case this is not the first time initmem2 is called */

    /* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */
    // release memory used to store each of the nodes in the linked list
    MemList *current = head, *temp;
    while(current != NULL) {
        temp = current;
        current = current->next;
        free(temp);
    }
}

// this function can be used to get hold of a struct ptr when you only have a mem address
// returns null if a struct with the given memory ptr cannot be found
MemList* getStructPtr(void *memLocation) {
    if(memLocation == NULL)
        return NULL;
    MemList *memStruct = head;
    if(myStrategy == Next) {
        size_t sentinel = 0; // this is to prevent infinite loops if a circular list is used
        while(sentinel <= mySize) { // traverse the list to find the relevant block
            if(memStruct->ptr == memLocation)
                break;
            sentinel += memStruct->size;
            memStruct = memStruct->next;
        }
        if(sentinel > mySize)
            return NULL; // this is reached if in a circular list, the struct cannot be found
    } else {
        while(memStruct != NULL) { // traverse the list to find the relevant block
            if(memStruct->ptr == memLocation)
                break;
            memStruct = memStruct->next;
        }
    }
    return memStruct;
}

/* 
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
	return;
}

/* Use this function to track memory allocation performance.  
 * This function does not depend on your implementation, 
 * but on the functions you wrote above.
 */ 
void print_memory_status()
{
	printf("%d out of %d bytes allocated.\n",mem_allocated(),mem_total());
	printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
	printf("Average hole size is %f.\n\n",((float)mem_free())/mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {
        strategies strat;
	void *a, *b, *c, *d, *e;
	if(argc > 1)
	  strat = strategyFromString(argv[1]);
	else
	  strat = First;
	
	
	/* A simple example.  
	   Each algorithm should produce a different layout. */
	
	initmem(strat,500);
	
	a = mymalloc(100);
	b = mymalloc(100);
	c = mymalloc(100);
	myfree(b);
	d = mymalloc(50);
	myfree(a);
	e = mymalloc(25);
	
	print_memory();
	print_memory_status();
	
}

int main()
{
    initmem(First,500);

    allocateMem(findFirstFit(100),100);
    allocateMem(findFirstFit(200),200);
    /*allocateMem(findWorstFit(100),100);
    allocateMem(findWorstFit(50),50);
    allocateMem(findWorstFit(250),250);
    allocateMem(findWorstFit(99),99);*/

    MemList *current_node = head;
    /* Print all the elements in the linked list */
    printf("The blocks in memory are:\n");
    while ( current_node != NULL) {
        printf("allocStatus : %d\tsize: %d\n", current_node->alloc,current_node->size);
        current_node = current_node->next;
    }
    printf("\n");

    /* Count the number of nodes in a linked list */
    int cnt = 0;
    current_node = head;
    while ( current_node != NULL) {
        cnt++;
        current_node = current_node->next;
    }
    printf("The number of nodes in the list is: %d", cnt);
    freeProgramMemory();
}
