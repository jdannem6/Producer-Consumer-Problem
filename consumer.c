// CS33211: Operating Systems
// consumer.c 
// Written by Justin Dannemiller
// 10/19/2021
// This program is one part of a two program system that simulates the 
// producer-consumer problem. Specifically, this file defines the 
// behavior of the consumer which consumes items from a 2 item table
// Semaphores and shared memory are used in this implementation

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/ipc.h> // Used for interprocess communication
#include <sys/shm.h> // used for shared memory syscalls
#include <fcntl.h>
#include <unistd.h>

// Define the files associated with the semaphores and shared memory segment
#define SEM_PRODUCER_FILE "/producer"
#define SEM_CONSUMER_FILE "consumer"
#define FILE1_NAME "producer.c" // file used for keys needed to get shared 
                               // memory blocks
#define FILE2_NAME "consumer.c" // file used for sharedIndex memory segment



#define IPC_ERROR (-1) // Indicates failure of operation of IPC operations
                       // such as ftok(), shmget(), etc.

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
    const int TableItems = 2; // items in table
    const int initProd = 0; // initial state of producer
    sem_t *sem_prod = sem_open(SEM_PRODUCER_FILE, O_CREAT, 0660, initProd);
    sem_t *sem_consum = sem_open(SEM_CONSUMER_FILE, O_CREAT, 0660, TableItems);


    // Test for semaphore creation. Give error message and terminate
    // on failure to create either semaphore
    if (sem_prod == SEM_FAILED)
    {
        printf("Failure to create sem_prod using sem_open()");
        exit(EXIT_FAILURE);
    }
    
    if (sem_consum == SEM_FAILED)
    {
        printf("Failure to create sem_consum using sem_open()");
        exit(EXIT_FAILURE);
    }

    // Attach the consumer's address space to the shared memory segment
    const int Tableitems = 2;
    int* table; // Address of shared memory segment
    table = attach_memory_block(FILE1_NAME, Tableitems*sizeof(int));

    int* sharedIndex = attach_memory_block(FILE2_NAME, sizeof(int));
    // Consume items from table until a -1 item, marking the end of 
    // the production is reached
    int readValue = 0;
    while(readValue != -1)
    {
        sem_wait(sem_prod); // Wait until table has items
        sleep(1);
        // Read integer from table
        readValue = *table;
        // Remove read item from table
        //*table = 0;
        // Indicate that an item has been consumed
        --(*sharedIndex);
        table[*sharedIndex] = 0;
        printf("Consumer: Item consumed from table;");
        printf(" there are %d item(s) in total. \t", *sharedIndex);
        printf("Table: [%d | %d]\n", table[0], table[1]);
        //printf(" with value: %d\n\n", readValue);
        if (readValue == -1) break;
        sem_post(sem_consum); // signal that an item has been consumed

    }

    // Detach the shared memory segments from the process's address space
    shmdt(table);
    shmdt(sharedIndex);

    // Delete the shared memory segmenta
    int tableBlockID = get_shared_block(FILE1_NAME, Tableitems*sizeof(int));
    shmctl(tableBlockID, IPC_RMID, NULL);

    int indexID = get_shared_block(FILE2_NAME, sizeof(int));
    shmctl(indexID, IPC_RMID, NULL);

    // Release semaphores
    sem_close(sem_prod);
    sem_close(sem_consum);
    
}