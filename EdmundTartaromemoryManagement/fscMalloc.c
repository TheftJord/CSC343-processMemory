/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cFiles/file.c to edit this template
 */


#include "fscMalloc.h"
#include <assert.h>
#include <sys/mman.h>
#include <stdlib.h> /* for rand() */
#include <time.h> /* for the init of rand */
#include <stddef.h> /* for size_t */

/**
 * this will set up the memory structure
 * @param m
 * @param am
 * @param sizeInBytes
 * @return 
 */

//verification number
#define MAGIC_NUMBER 0x3697833
void* fscMemorySetup(memoryStructure* m, fscAllocationMethod am, size_t sizeInBytes) {
    if (FIRST_FIT_RETURN_FIRST != am) {
        fprintf(stderr, "This code only supports the FIRST_FIT_RETURN_FIRST allocation method\n");
        return 0;
    }
    //makes the initial memory block
    m->head=(fsc_free_node_t*)malloc(sizeInBytes);
    //verification check
    if(m->head==NULL){
        return 0;
    }
    //assigns values to nodes
    m->head->size=sizeInBytes-sizeof(fsc_free_node_t);
    m->head->next=NULL;
    m->magicNumber=MAGIC_NUMBER;
    //return value
    return m->head+1;
}

/**
 * this is used to section out the memory
 * @param m
 * @param requestedSizeInBytes
 * @return 
 */
void* fscMalloc(memoryStructure* m, size_t requestedSizeInBytes) {
    //verification check
    if(m->magicNumber!=MAGIC_NUMBER){
        printf("verification failed\n");
        return NULL;
    }
    //sets up node
    fsc_free_node_t* current=m->head;
    fsc_free_node_t* prev=NULL;
    size_t totalSizeNeeded=requestedSizeInBytes+sizeof(fsc_alloc_header_t);
    //goes through memory
    while(current!=NULL){
        //checks if memory is big enough
        if(current->size>=totalSizeNeeded){
            //calc remain size
            size_t remainingSize=current->size-totalSizeNeeded;
            //sets up header
            fsc_alloc_header_t* header=(fsc_alloc_header_t*)((char*)current+sizeof(fsc_free_node_t));
            header->size=requestedSizeInBytes;
            header->magic=MAGIC_NUMBER;
        
        //adjusts the free list
            if(remainingSize>=sizeof(fsc_free_node_t)){
                //creates new memory node with remaining memory
                fsc_free_node_t* newFreeNode=(fsc_free_node_t*)((char*)current+totalSizeNeeded);
                newFreeNode->size=remainingSize-sizeof(fsc_free_node_t);
                newFreeNode->next=current->next;
                current->size=remainingSize;
                //links the node to the memory
                if(prev){
                    prev->next=newFreeNode;
                }
                else{
                    m->head=newFreeNode;
                }
            }
            else{
                //not enough space to make a new node
                if(prev){
                    prev->next=current->next;
                }
                else{
                    m->head=current->next;
                }
            }
            //return value
            return (void*)(header+1);
        }
        //cycles through memory
        prev=current;
        current=current->next;
    }
    //failed
    return NULL;
}

/**
 * frees memory that is no longer in use
 * @param m
 * @param returnedMemory
 */
void fscFree(memoryStructure* m, void * returnedMemory) {
    //verification check
    if(m->magicNumber!=MAGIC_NUMBER){
        printf("verification failed\n");
        return;
    }
    if(!returnedMemory){
        printf("verification failed\n");
        return;
    }
    //gets header
    fsc_alloc_header_t* header=(fsc_alloc_header_t*)((char*)returnedMemory-sizeof(fsc_alloc_header_t));
    //verification check
    if(m->magicNumber!=MAGIC_NUMBER){
        printf("verification failed\n");
        return;
    }
    //gets node to be freed
    fsc_free_node_t* nodeToFree=(fsc_free_node_t*)((char*)header-sizeof(fsc_free_node_t));
    nodeToFree->size=header->size+sizeof(fsc_alloc_header_t);
    //adds node to node list
    fsc_free_node_t* current=m->head;
    fsc_free_node_t* prev=NULL;
    //find position a release
    while(current!=NULL&&current<nodeToFree){
        prev=current;
        current=current->next;
    }
    //coalesce with the next block
    if(current&&(char*)nodeToFree+nodeToFree->size==(char*)current){
        nodeToFree->size+=current->size+sizeof(fsc_free_node_t);
        nodeToFree->next=current->next;
    }
    else{
        nodeToFree->next=current;
    }
    //insert node
    if(prev){
        prev->next=nodeToFree;
        if((char*)prev+prev->size==(char*)nodeToFree){
            prev->size+=nodeToFree->size+sizeof(fsc_free_node_t);
            prev->next=nodeToFree->next;
        }
    }
    else{
        m->head=nodeToFree;
    }
}

/**
 * to clean up the memory to make sure that there aren't any leftovers 
 * @param m
 */
void fscMemoryCleanup(memoryStructure* m){
    //verification check
    if(m->magicNumber!=MAGIC_NUMBER){
        printf("verification failed\n");
        return;
    }
    //clears head of the list
    free(m->head);
    m->head=NULL;
    m->magicNumber=0;
}

/* Given a node, prints the list for you. */
void printFreeList(FILE * out, fsc_free_node_t* head) {
    /* given a node, prints the list. */
    fsc_free_node_t* current = head;
    fprintf(out, "About to dump the free list:\n");
    while (0 != current) {
        fprintf(out,
                "Node address: %'u\t Node size (stored): %'u\t Node size (actual) %'u\t Node next:%'u\n",
                current,
                current->size,
                current->size + sizeof (fsc_free_node_t),
                current->next);
        current = current->next;
    }
}