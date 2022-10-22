#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"

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

	freeProgramMemory(); // free any existing block of memory and any existing structs/nodes in the linked list

	myMemory = malloc(sz);

    // create a new MemList struct and make all global pointers point to this at first
    head = malloc(sizeof (MemList));
    tail = head;
    next = head;

    // initialize values
    head->size = (int) mySize;
    head->alloc = 0;
    head->next = NULL;
    head->prev = NULL;
    head->ptr = myMemory;
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
	            return allocateMem(findFirstFit(requested),requested);
	  case Best:
	            return allocateMem(findBestFit(requested),requested);
	  case Worst:
	            return allocateMem(findWorstFit(requested),requested);
	  case Next:
	            return allocateMem(findNextFit(requested),requested);
	  }
	return NULL;
}

// returns NULL if memory cannot be allocated, otherwise returns ptr to memory location (void*) of allocated block
void* allocateMem(MemList *allocatedBlock, size_t requestedSize) {
    if(allocatedBlock == NULL || allocatedBlock->size < requestedSize)
        return NULL; // return null if block does not exit or if search algorithm found a too small block (should not happen)

    allocatedBlock->alloc = 1; // repurpose the found block by changing its alloc status. We will change its size later

    if(allocatedBlock->size > requestedSize) {
        // if requested size < block size, there will be a block of left-over memory, so we need a new struct
        MemList *newBlock = malloc(sizeof(MemList)); // newblock will store information about the left-over chunk

        // update pointers in the linked list (insert newBlock after allocatedBlock)
        newBlock->next = allocatedBlock->next;
        newBlock->prev = allocatedBlock;
        if (allocatedBlock->next != NULL)
            allocatedBlock->next->prev = newBlock;
        allocatedBlock->next = newBlock;

        // initialize newBlock data
        newBlock->size = allocatedBlock->size - (int) requestedSize;
        newBlock->alloc = 0;
        newBlock->ptr = allocatedBlock->ptr + requestedSize; // the new block's starting location is oldBlockLocation + oldBlockSize

        // update the size of the allocatedBlock
        allocatedBlock->size = (int) requestedSize;

        // update global tail pointer if the new block is at the end of the list
        if (tail == allocatedBlock) {
            tail = newBlock;
            // for NextFit, we use a circular linked list. When we update the tail, these linkages must also be updated
            if(myStrategy == Next) {
                tail->next = head;
                head->prev = tail;
            }
        }
    }
    next = allocatedBlock->next;

    if(myStrategy == Next && next == NULL) // this is to ensure that the global *next is never null under nextFit
        next = head;                       // this could occur if after init, the whole block is allocated all at once

    return allocatedBlock->ptr;
}

// returns NULL pointer if no eligible block is found, otherwise returns pointer to the block to allocate
MemList* findWorstFit(size_t requested) {
    MemList *biggestBlockPtr = NULL, *current = head;
    int biggestBlockSize = 0;  // initialize to the smallest possible size
    while(current != NULL) { // iterate over the whole list - save the largest eligible block found thus far
        if(current->alloc == 0 && current->size >= requested && current->size > biggestBlockSize) {
            biggestBlockPtr = current;
            biggestBlockSize = current->size;
        }
        current = current->next;
    }
    return biggestBlockPtr;
}

// returns NULL pointer if no eligible block is found, otherwise returns pointer to the block to allocate
MemList* findBestFit(size_t requested) {
    MemList *current = head, *bestBlockPtr = NULL;
    size_t smallestFeasibleBlock = mySize; // initialize to the largest possible value
    while(current != NULL) {  // iterate over the whole list - save the smallest eligible block found thus far
        if(current->alloc == 0 && current->size >= requested && (current->size < smallestFeasibleBlock || current->size == mySize)) {
            bestBlockPtr = current;
            smallestFeasibleBlock = current->size;
        }
        current = current->next;
    }
    return bestBlockPtr;
}

// returns NULL pointer if no eligible block is found, otherwise returns pointer to the block to allocate
MemList* findFirstFit(size_t requested) {
    MemList *firstBlockPtr = NULL;
    MemList *current = head;

    while(current != NULL) {
        if (current->alloc == 0 && current->size >= requested) {
            firstBlockPtr = current;
            return firstBlockPtr;
        }
        current = current->next;
    }
}

//TODO: implement next fit algorithm
MemList* findNextFit(size_t requested)
{
    //MemList *current = next;
    MemList *startBlockPtr = next;
    MemList *nextBlockPtr = NULL;

    if (next->next != NULL) {
        next = next->next;
        while(next != startBlockPtr )
        {
            printf("Here \n");
            if(next->alloc == 0 && next->size >= requested)
            {
                //nextBlockPtr = next;
                return next;
            }
            next = next->next;
        }
    }
    else
    {
        printf("Else Here \n");
        nextBlockPtr = startBlockPtr;
    }

    return nextBlockPtr;
}

/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void *block)
{
    MemList *freeing = getStructPtr(block); //Get the pointer for the struct corresponding to the mem location ptr
    if ((freeing != NULL) && (freeing->alloc == 0)) //If the block isn't in use, return
        return;

    freeing->alloc = 0;

    if ((freeing->prev != NULL) && (freeing->prev != tail) && (freeing->prev->alloc == 0)) { //If there is a previous, free block in a non-circular manner, combine them
        MemList *left = freeing->prev;
        if (left->prev != NULL) { //If the left block isn't the first
            left->prev->next = freeing; //Update links
            freeing->prev = left->prev;
        } else {
            freeing->prev = NULL; //Update link
            head = freeing; //Update head
        }
        freeing->ptr = left->ptr; //Update memory location ptr
        freeing->size += left->size; //Add the size of the joined blocks
        if (left == next) { //If the next pointer is pointing at the link about to be deleted, move it
            next = freeing;
        }
        free(left);
    }

    if ((freeing->next != NULL) && (freeing->next != head) && (freeing->next->alloc == 0)) { //If there is a next, free block in a non-circular manner, combine them
        MemList *right = freeing->next;
        if (right->next != NULL) { //If the right block isn't the last
            right->next->prev = freeing;  //Update links
            freeing->next = right->next;
        } else {
            freeing->next = NULL; //Update link
            tail = freeing; //Update head
        }
        freeing->size += right->size; //Add the size of the joined blocks
        if (right == next) { //If the next pointer is pointing at the link about to be deleted, move it
            next = freeing;
        }
        free(right);
    }
}

// this function takes a mem location ptr to the beginning of a block and returns a pointer to the corresponding struct
// returns null if a struct with the given memory ptr cannot be found
MemList* getStructPtr(void *memLocation) {
    if(memLocation == NULL || head == NULL)
        return NULL;
    MemList *memStruct = head;
    while(memStruct != NULL) { // traverse the list to find the relevant block whose ptr = *memLocation
        if(memStruct->ptr == memLocation)
            return memStruct;
        memStruct = memStruct->next;
        if(memStruct == head) // in this case, we have a circular list and have looped all the way back to the start without finding a struct
            return NULL; // thus should return null
    }
    return NULL;
}

void freeProgramMemory() {
    if (myMemory != NULL)
        free(myMemory); /* in case this is not the first time initmem2 is called */

    // release memory used to store each of the nodes in the linked list
    if (head != NULL) {
        MemList *current = head->next, *temp;
        while(current != NULL && current != head) { // use the head as a sentinel in the loop (in case of a circular list)
            temp = current;
            current = current->next;
            free(temp);
        }
        free(head); // free the head last
    }
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when refered to "memory" here, it is meant that the 
 * memory pool this module manages via initmem/mymalloc/myfree. 
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes()
{
    MemList *current = head;
    int count = 0;
    while ( current != NULL ) {
        if ((int)current->alloc == 0) // traverse the list and add to the counter if alloc is 0
            count++;
        current = current->next;
        if(current == head)
            break;  // this will only happen if we have a circular list and have looped back to the beginning - so we should exit the loop
    }
	return count;
}

/* Get the number of bytes allocated */
int mem_allocated()
{
    MemList *current = head;
    int countBytes = 0;
    while ( current != NULL) {
        if ((int)current->alloc == 1) // traverse the list and add size if alloc is 1
            countBytes += current->size;
        current = current->next;
        if(current == head)
            break;  // this will only happen if we have a circular list and have looped back to the beginning - so we should exit the loop
    }
    return countBytes;
}

/* Number of non-allocated bytes */
int mem_free()
{
    MemList *current = head;
    int countBytes = 0;
    while ( current != NULL) {
        if ((int)current->alloc == 0) // traverse the list and add size if alloc is 0
            countBytes += current->size;
        current = current->next;
        if(current == head)
            break;  // this will only happen if we have a circular list and have looped back to the beginning - so we should exit the loop
    }
	return countBytes;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{
    MemList *current = head;
    int biggestBlockSize = 0;  // initialize to the smallest possible size
    while(current != NULL) { // iterate over the whole list - save the largest free block's size
        if(current->alloc == 0 && current->size > biggestBlockSize)
            biggestBlockSize = current->size;
        current = current->next;
        if(current == head)
            break;  // this will only happen if we have a circular list and have looped back to the beginning - so we should exit the loop
    }
    return biggestBlockSize;
}

/* Number of free blocks smaller than or equal to "size" bytes. */
int mem_small_free(int size)
{
    int count = 0;
    MemList *current = head;
    while(current != NULL) {
        if(current->alloc == 0 && current->size <= size)
            count++;
        current = current->next;
        if(current == head)
            break;  // this will only happen if we have a circular list and have looped back to the beginning - so we should exit the loop
    }
    return count;
}

/* Allocation status of a particular byte. */
char mem_is_alloc(void *ptr)
{
    MemList *current = head;
    while(current != NULL) {
        if(ptr >= current->ptr && ptr < (current->ptr + current->size))
            return current->alloc;
        current = current->next;
        if(current == head) // this should not happen
            break; // it would mean that we have looped thru the whole (circular) list without finding the ptr anywhere
    }
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

/* 
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
    MemList *current = head;
    /* Print all the elements in the linked list */
    printf("The blocks in memory are:\n");
    if (myStrategy != Next) {
        while (current != NULL) {
            printf("allocStatus : %d\tsize: %d\n", current->alloc, current->size);
            current = current->next;
        }
    }
    else
    {
        printf("allocStatus : %d\tsize: %d\n", current->alloc, current->size);
        if (current->next != NULL) {
            current = current->next;
            while (current != head) {
                printf("allocStatus : %d\tsize: %d\n", current->alloc, current->size);
                current = current->next;
            }
        }
    }
    printf("\n");

    // Count the number of nodes in a linked list
    int count = 0;
    current = head;
    if (myStrategy != Next) {
        while ( current != NULL) {
            count++;
            current = current->next;
        }
    }
    else
    {
        if (current->next != NULL) {
            count++;
            current = current->next;
            while (current != head) {
                count++;
                current = current->next;
            }
        }
    }

    printf("The number of nodes in the list is: %d\n", cnt);
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
    mymalloc(100);
    mymalloc(80);
    mymalloc(220);
    mymalloc(99);
    mymalloc(2); // this should not be allocated - not enough space

    print_memory();
    printf("number of allocated bytes: %d",mem_allocated());
    printf("\nnumber of non-allocated bytes: %d",mem_free());
    printf("\nnumber of holes: %d\n",mem_holes());


    freeProgramMemory();

}
