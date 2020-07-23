#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"
team_t team = {
    "Ravenclaw",
    "Cedric",
    "@Hogwarts",
    "",
    
    ""
};

typedef struct header
{
    //int free;
    size_t size;
    struct header *next;
    struct header *prev;
} h;


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define offset 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(h)))

#define ret_chunk(ptr) ((void *)(ptr))
#define ret_hchunk(ptr) ((h *)(ptr))

#define find_prev(ptr) ret_hchunk((char *)(ptr)+(ptr->size))  //- sizeof(ptr->size))) //functioning same as find_nxt just for name sake
#define find_nxt(ptr)  ret_hchunk((char *)(ptr)+(ptr->size))//  - sizeof(ptr->size)))
/* 
 * mm_init - initialize the malloc package.
 */
void *curr = NULL;
size_t curr_size = 0x1000;
h *heap_head = NULL; //ptr to the beginning of th heap
int tobias = 0;
h *current = NULL;
h *head = NULL;
int free_blocks = 0; //dec in delete function and incr in mm_free function
//void *recent = NULL;
void delete( h *ptr)
{
    
    if(ptr == NULL) //error
        return;

    else
    {
        free_blocks--;
        if(ptr->next ==NULL && ptr->prev==NULL)//head chunk
            head = NULL;

        else if(ptr->prev ==NULL) //head chunk with next chunk avlbl
           {
            head = ptr->next;
            ptr->next->prev=NULL;
            ptr->next=NULL;
           } 
        else if(ptr->next == NULL)
        {
            ptr->prev->next = NULL;
        }
        else
            {
                ptr->prev->next = ptr->next;
                ptr->next->prev = ptr->prev;

            }
    }

        //setting up the pointers for an allocated chunk as it's not in the free list anymore   
        //ptr->size=0;
        ptr->next=NULL;
        ptr->prev=NULL;
    

        return;
}

h *split( h *ptr, size_t size)
{
    h *nptr = ret_hchunk((unsigned long)(ret_chunk(ptr)) + size);
    nptr->size = ptr->size-size;
    ptr->size = size;


    return nptr;
}
h *merge(void *ptr)
{
    //fucntions calling merge check for whether the next or the prev is free or not/
    //if both are free the function is not called
    //check ur next and prev ptrs
    h *next_t = NULL;
    h *prev_t = NULL;
    h *nptr = ret_hchunk(ptr); 

    //if (nptr->next == NULL && nptr->prev == NULL) //absence of both next and prev
      //  return nptr;

    if (nptr->next != NULL) // check for next==NULL
        next_t = find_nxt(nptr);

    if( nptr->prev !=NULL) // check for prev == NULL
        prev_t = find_prev(nptr->prev);
    
    if( (next_t != nptr->next || next_t == NULL) && (prev_t == NULL || prev_t != nptr->prev->next ))
        return nptr; //next and prev not as same or either is null

    if( next_t == nptr->next && next_t!=NULL) //forward coaelscing
    {
        nptr->size += next_t->size;
        next_t->size = 0;
        delete(next_t);
        return nptr;
    }

    if(nptr->prev->next==prev_t) //backward coaelscing
    {
        nptr->prev->size+= nptr->size;
        nptr->size=0;
        prev_t = nptr->prev;
        delete(nptr);
        return prev_t;
    }

    /*else if(next_t == nptr->next && nptr->prev->next == prev_t)
    {
        nptr->prev->size = nptr->size + nptr->next->size;
        delete(nptr);
        delete(nptr->next);
        return nptr->prev;
    }*/

    else
    {
        return nptr;
    }
}

void add(h *ptr)
{
    free_blocks+=1;
    if( head == NULL )
    {
        head = ptr;
        head->next=NULL;
        head->prev=NULL;
    }

    else if((unsigned long)ptr < (unsigned long)head)
    {
        //add as per the sorted addr of the blocks so as to easily get a cntns memory
        head->prev = ptr;
        ptr->next = head;
        ptr->prev=NULL;
        head = ptr;

    }

    else
    {

        h *nptr = head;
        while(nptr->next!=NULL && ((unsigned long)ptr > (unsigned long)nptr->next))
        {
            nptr = nptr->next;

        }
        if(nptr->next!=NULL)
            nptr->next->prev = ptr;   
        ptr->next = nptr->next;
        ptr->prev = nptr;
        nptr->next = ptr;
        //printf("lol");
        
        
        
        //nptr->size|=1;
    }

   /* if(ptr->next == ptr || ptr->prev ==ptr)
    {
        ptr->next = find_nxt(ptr);
        ptr->prev = NULL;
    }*/

}

void *find_free(size_t size)
{
    h *ptr = head;
    void *chunk = NULL;
    void *nchunk = NULL;
    while(ptr != NULL) //freelist is empty
    {
        if(ptr->size == size || ptr->size-size <=32) //to avoid fragmentation
        {
            chunk = ret_chunk(ptr);
            delete(ptr);
            break;
        }

        else if(ptr->size>=size)
        {
            delete(ptr);
            nchunk = split(ptr,size);
            add(nchunk);
            chunk = ret_chunk(ptr);
            break;
        }

        else
            ptr = ptr->next;
    }

    return chunk;
}

int mm_init(void)
{
    //curr = mem_sbrk(curr_size); //overlapping chunks error
    head =NULL;
    free_blocks = 0;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE );
    //void  *p = mem_sbrk(newsize);

    void *p = NULL;
    //if (recent!= NULL && ret_hchunk(recent)->size== size)
        //p = recent;

    if (free_blocks != 0)
        p = find_free(newsize);
    if(p == NULL)
        {                
            p = mem_sbrk(newsize);
        }    
        current = ret_hchunk(p);
        if(current->size !=0 && current->size-newsize <= 32); //save the current size as it's greater than the newsize
            
        else
            current->size = newsize;
        //current->size &= 0xFE;
        //current->next = NULL;
        //curr = newsize + p;

        //*(size_t *)p = size;
        return (void *)((char *)current + SIZE_T_SIZE);
}


//0xf69f5008

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    free_blocks +=1;
    h *temp = ret_hchunk((void *)((char *)ptr-SIZE_T_SIZE));
    add(temp);
    //recent = ptr;

    if(temp->next!=NULL || temp->prev!=NULL)
        merge(temp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t _size)
{
    //when mem_sbrk is called where does it allocate the new memory?


    int size = ALIGN(_size + SIZE_T_SIZE ); //Alignment of given size
    void *oldptr =(void *)((char *)ptr-SIZE_T_SIZE); //finding the orig ptr (rmv header)
    h *temp1 = ret_hchunk(oldptr); 
    void *newptr;
    //tobias=_size;
    if(oldptr == NULL) //malloc 
    {
        newptr = mm_malloc(size);
        if (newptr == NULL)
            return NULL;
        else 
            return newptr;
    }

    else if(size == 32) // free the pointer
    {
        mm_free(ptr);
        return NULL;
    }

    else if(temp1->size == size ) //when realloc and alloc size remains the same
    {
        return (void *)ptr;
    }

    else 
    {
        add(temp1); //adding to the free list so as to get possibility of merge
        if(temp1->next!=NULL || temp1->prev!=NULL) //checking whether head chunk or not
        {
            temp1 = merge(temp1); //o/wise merge...error!

            if(temp1->size == size || (temp1->size-size <= 32 && temp1->size-size>=0)) //pefect match
                {
                    delete(temp1);
               }

            else if(temp1->size-size >= 32) //split and then merge? fix!
                {
                    delete(temp1);
                    oldptr = split(temp1,size); //returning the second part
                    add(oldptr);

                    //temp1->size = size;
                    //add(ret_hchunk((void *)((char *)temp1 + size + SIZE_T_SIZE)));
                }

                
            else if(temp1->size < size)
            {
                return NULL;
            }

            memcpy((char *)newptr + SIZE_T_SIZE,(char *)ptr + SIZE_T_SIZE, size+1);
            return (void *)((char *)temp1 + SIZE_T_SIZE);


        }   


        /*if(temp1->size < size) //new mem to be allocated
        {
            
            newptr = mm_malloc(size);
            if (newptr == NULL)
                return NULL;
            else
               { strcpy((char *)newptr + SIZE_T_SIZE,(char *)ptr + SIZE_T_SIZE); //memcpy?
                 ptr = NULL;
                return (void *)((char *)newptr + SIZE_T_SIZE) ;
               }
        }*/

        /*mm_free((void *)((char *)ptr+SIZE_T_SIZE));
        newptr = mm_malloc(_size);
        memcpy((char *)newptr + SIZE_T_SIZE,(char *)ptr + SIZE_T_SIZE, _size);
        return (void *)((char *)newptr + SIZE_T_SIZE) ;*/
    }
        return NULL;
}
