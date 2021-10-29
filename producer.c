// CS33211: Operating Systems
// producer.c
// Written by Justin Dannemiller
// 10/21/2021
// This program is one part of a two program system that simulates the 
// producer-consumer problem. Specifically, this file defines the 
// behavior of the producer which produces items and puts them in a 2-item
// table provided while it is not full
// Semaphores and shared memory are used in this implementation

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/types.h> // Needed for key types
#include <sys/ipc.h> // Needed for shared memory
#include <sys/shm.h> // Needed for shared memory
#include <semaphore.h>
#include <fcntl.h> // Needed for semaphore O_flags
#include <unistd.h>
#include <time.h> 

// Define the files associated with the semaphores and shared memory segment
#define SEM_PRODUCER_FILE "/producer"
#define SEM_CONSUMER_FILE "consumer"
#define FILE1_NAME "producer.c" // file used for keys needed to get shared 
                               // memory blocks
#define FILE2_NAME "consumer.c" // file used for sharedIndex memory segment


#define IPC_ERROR (-1) // Indicates failure of operation of IPC operations
                       // such as ftok(), shmget(), etc.

#define NUM_PRODUCTS 8 // Number of items to be added be producer before it
                        // teminates

// This function takes the name of the file to which the shared memory block 
// is to be attached as well as an integer size that specifies the size
// of the block to be created
// Returns the blockID of the shared memory block on success
// Otherwise returns -1 on failure
static int get_shared_block(char *filename, int size)
{
    key_t key;

    // Request a key associated with filename
    key = ftok(filename, 0);
    
    // If ftok couldn't get a key, return error code 
    if (key == IPC_ERROR)
        return IPC_ERROR;

    // Get shared block; create it if it doesn't exist. 
    return shmget(key, size, 0644 | IPC_CREAT);

}

// Attaches a file to a shared memory block It takes the filename to specify 
// which file to associate the memory block with, as well as the size of the block
int* attach_memory_block(char* filename, int size)
{
    // get shared block ID
    int shared_blockID = get_shared_block(filename, size);
    int *result; // pointer to shared block; NULL on failure

    // Return NULL if blockID could not be retrieved for the file
    if (shared_blockID == IPC_ERROR)
        return NULL;

    // Otherwise, attach the sharedd block to the address space of the process
    // and return a pointer to it. 
    result = shmat(shared_blockID, NULL, 0);

    // End function upon failure to attach mem block
    if (result == (int*)IPC_ERROR)
        return NULL;
    
    // Otherwise, return the pointer to the shared memory block
    return result;
}

int main()
{
    srand(time(NULL));

    // Reset semaphore states by unlinking Semaphore files
    sem_unlink(SEM_CONSUMER_FILE);
    sem_unlink(SEM_PRODUCER_FILE);

    // Create the Producer and Consumer semaphores
    const int TableItems = 2;
    const int initProd = 0; // initial state of producer semaphore
    // Producer semaphore starts with state 0 (nothing in table initially) and
    // consumer semaphore starts with state 2 (initially 2 empty spots in table);
    sem_t *sem_prod = sem_open(SEM_PRODUCER_FILE, O_CREAT, 0660, initProd);
    sem_t *sem_consum = sem_open(SEM_CONSUMER_FILE, O_CREAT, 0660, TableItems);
    

    // Test for semaphore creation. Give error message and terminate
    // on failure to create either semaphore
    if (sem_prod == SEM_FAILED)
    {
        printf("Failure to create sem_prod using sem_open()\n");
        exit(EXIT_FAILURE);
    }
    
    if (sem_consum == SEM_FAILED)
    {
        printf("Failure to create sem_consum using sem_open()\n");
        exit(EXIT_FAILURE);
    }

    // Create a shared memory segment of integers and attach it to the 
    // process's address space
    int* table; // Stores address of shared memory segment
    table = attach_memory_block(FILE1_NAME, TableItems*sizeof(int));

    int* sharedIndex = attach_memory_block(FILE2_NAME, sizeof(int));
    *sharedIndex = 0; 

    printf("\n");
    // Produce items and insert them onto the table
    for (int i = 0; i < NUM_PRODUCTS; ++i)
    {
        sem_wait(sem_consum); // wait if table is full
        sleep(1);
        // Get random integer to insert
        int randomValue = 1 + (rand() % 1000); // random int 1-1000
        table[*sharedIndex]= randomValue; // Insert item onto table
        ++(*sharedIndex);
        // Indicate that producer produced
        printf("Producer: New Item in table;");
        printf(" there are %d item(s) in total. \t\t", *sharedIndex);
        printf("Table: [%d | %d]\n", table[0], table[1]);
        sem_post(sem_prod); // Signal that product has been added
    }

    // Signal to consumer to stop consuming
    sem_wait(sem_consum);
    table[0] = -1; // -1 is a flag that signals end of production
    sem_post(sem_prod);


    // Detach the shared memory segments from the process's address 
    // space
    shmdt(table);
    shmdt(sharedIndex);
    
    // Release semaphores
    sem_close(sem_prod);
    sem_close(sem_consum);


}