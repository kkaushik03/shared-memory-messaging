# shared-memory-messaging

An implementation of a file transfer mechanism using System V IPC mechanisms shared memory and message queues in C. A "sender" reads a file and writes its contents to a shared memory. A "receiver" reads the data from the shared memory and processes it. In between, the sender-receiver communication is synchronized using message queues for data integrity and proper sequencing.

## Key Features
- Uses System V shared memory for fast data transfer between processes.
- Uses System V message queues for inter-process communication and synchronization.
- Breaks a large file into chunks to be sent across the platform to handle very large file sizes.
- Includes error handling for robust handling of edge cases.
- Shows setup, usage, and cleanup of IPC resources in a Linux environment.

## Usage Instructions
1. Compile the sender and receiver programs.
2. Run the receiver program to start listening in on incoming file data.
3. Run the sender program with the file you want to send as an argument.

## Files
- `sender.c`: The sender program reads a file and sends its data contents to the receiver.
- `receiver.c`: The receiver program reads shared memory data and processes it.
- `msg.h`: Defines the structures used for messages.
- `Makefile`: Makefile for building the project.

## Example Commands
```sh
# Compile the programs
make

# Start the receiver
./receiver

# Send a file
./sender <file_name>