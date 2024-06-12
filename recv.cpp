#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "msg.h"    /* For the message struct */

using namespace std;

/* The size of the shared memory segment */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr = NULL;

/**
* The function for receiving the name of the file
* @return - the name of the file received from the sender
*/
string recvFileName()
{
    /* The file name received from the sender */
    string fileName;
    
    /* Declare an instance of the fileNameMsg struct to be used for holding the message received from the sender. */
    fileNameMsg fileNameMessage;

    /* Receive the file name using msgrcv() */
    if(msgrcv(msqid, &fileNameMessage, sizeof(fileNameMessage) - sizeof(long), FILE_NAME_TRANSFER_TYPE, 0) == -1)
    {
        perror("msgrcv");
        exit(1);
    }

    /* Return the received file name */
    fileName = fileNameMessage.fileName;
    return fileName;
}

/**
* Sets up the shared memory segment and message queue
* @param shmid - the id of the allocated shared memory
* @param msqid - the id of the shared memory
* @param sharedMemPtr - the pointer to the shared memory
*/
void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
    /* Create a file called keyfile.txt containing string "Hello world" (you may do so manually or from the code). */
    FILE* keyFile = fopen("keyfile.txt", "w");
    if (!keyFile)
    {
        perror("fopen");
        exit(1);
    }
    fprintf(keyFile, "Hello world");
    fclose(keyFile);

    /* Use ftok("keyfile.txt", 'a') in order to generate the key. */
    key_t key = ftok("keyfile.txt", 'a');
    if (key == -1)
    {
        perror("ftok");
        exit(1);
    }

    /* Allocate a shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE. */
    shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }

    /* Attach to the shared memory */
    sharedMemPtr = shmat(shmid, NULL, 0);
    if (sharedMemPtr == (void*)-1)
    {
        perror("shmat");
        exit(1);
    }

    /* Create a message queue */
    msqid = msgget(key, IPC_CREAT | 0666);
    if (msqid == -1)
    {
        perror("msgget");
        exit(1);
    }

    /* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
    printf("Shared memory ID: %d\n", shmid);
    printf("Message queue ID: %d\n", msqid);
}

/**
* The main loop
* @param fileName - the name of the file received from the sender.
* @return - the number of bytes received
*/
unsigned long mainLoop(const char* fileName)
{
    /* The size of the message received from the sender */
    int msgSize = -1;
    
    /* The number of bytes received */
    unsigned long numBytesRecv = 0;
    
    /* The string representing the file name received from the sender */
    std::string recvFileNameStr = fileName;
    
    /* Append __recv to the end of file name */
    recvFileNameStr.append("__recv");
    
    /* Open the file for writing */
    FILE* fp = fopen(recvFileNameStr.c_str(), "w");
    
    /* Error checks */
    if (!fp)
    {
        perror("fopen");
        exit(-1);
    }
    
    /* Message buffer to receive messages */
    message rcvMsg;
    
    /* Message buffer to send acknowledgment */
    ackMessage ackMsg;
    ackMsg.mtype = RECV_DONE_TYPE;
    
    /* Keep receiving until the sender sets the size to 0, indicating that
     * there is no more data to send.
     */    
    while (msgSize != 0)
    {    
        /* Receive the message and get the value of the size field. The message will be of
         * type SENDER_DATA_TYPE. That is, a message that is an instance of the message struct with
         * mtype field set to SENDER_DATA_TYPE (the macro SENDER_DATA_TYPE is defined in
         * msg.h). If the size field of the message is not 0, then we copy that many bytes from
         * the shared memory segment to the file. Otherwise, if 0, then we close the file
         * and exit.
         */
        if (msgrcv(msqid, &rcvMsg, sizeof(rcvMsg) - sizeof(long), SENDER_DATA_TYPE, 0) == -1)
        {
            perror("msgrcv");
            exit(-1);
        }

        msgSize = rcvMsg.size;

        /* If the sender is not telling us that we are done, then get to work */
        if (msgSize != 0)
        {
            /* Count the number of bytes received */
            numBytesRecv += msgSize;
            
            /* Save the shared memory to file */
            if (fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < msgSize)
            {
                perror("fwrite");
                fclose(fp);
                exit(-1);
            }
            
            /* Tell the sender that we are ready for the next set of bytes.
             * I.e., send a message of type RECV_DONE_TYPE. That is, a message
             * of type ackMessage with mtype field set to RECV_DONE_TYPE.
             */
            if (msgsnd(msqid, &ackMsg, sizeof(ackMsg) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
                fclose(fp);
                exit(-1);
            }
        }
        /* We are done */
        else
        {
            /* Close the file */
            fclose(fp);
        }
    }
    
    return numBytesRecv;
}

/**
* Performs cleanup functions
* @param sharedMemPtr - the pointer to the shared memory
* @param shmid - the id of the shared memory segment
* @param msqid - the id of the message queue
*/
void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
    /* Detach from shared memory */
    if (shmdt(sharedMemPtr) == -1)
    {
        perror("shmdt");
        exit(1);
    }

    /* Deallocate the shared memory segment */
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(1);
    }

    /* Deallocate the message queue */
    if (msgctl(msqid, IPC_RMID, NULL) == -1)
    {
        perror("msgctl");
        exit(1);
    }
}

/**
* Handles the exit signal
* @param signal - the signal type
*/
void ctrlCSignal(int signal)
{
    /* Free system V resources */
    cleanUp(shmid, msqid, sharedMemPtr);
    exit(0);  // Ensure the program exits after cleanup
}

int main(int argc, char** argv) {
    /* Install a signal handler (see signaldemo.cpp sample file).
     * If user presses Ctrl-c, your program should delete the message
     * queue and the shared memory segment before exiting. You may add
     * the cleaning functionality in ctrlCSignal().
     */
    signal(SIGINT, ctrlCSignal);

    /* Initialize */
    init(shmid, msqid, sharedMemPtr);

    /* Receive the file name from the sender */
    string fileName = recvFileName();

    /* Go to the main loop */
    fprintf(stderr, "The number of bytes received is: %lu\n", mainLoop(fileName.c_str()));

    /* Detach from shared memory segment, and deallocate shared memory
     * and message queue (i.e. call cleanup)
     */
    cleanUp(shmid, msqid, sharedMemPtr);

    return 0;
}
