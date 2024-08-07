

# sShared-Memory-Messaging

An implementation of a file transfer mechanism using System V IPC mechanisms shared memory and message queues in C++. A "sender" reads a file and writes its contents to a shared memory. A "receiver" reads the data from the shared memory and processes it. In between, the sender-receiver communication is synchronized using message queues for data integrity and proper sequencing.

## Key Features
- Uses System V shared memory for fast data transfer between processes.
- Uses System V message queues for inter-process communication and synchronization.
- Breaks a large file into chunks to be sent across the platform to handle very large file sizes.
- Includes error handling for robust handling of edge cases.
- Shows setup, usage, and cleanup of IPC resources in a Linux environment.

## Usage Instructions
**If running application on Mac or Linux, should automatically be ran on the terminal, otherwise if 
using Windows it may require the additional download of Windows subsystem for Linux to run properly!**

1. Compile the sender and receiver programs.
2. Run the receiver program to start listening in on incoming file data.
3. Run the sender program with the file you want to send as an argument.

OR

1. Create keyfile.txt using the command: echo "Hello world" > keyfile.txt
2. Use the command: clang++ -o recv recv.cpp
3. Use the command: clang++ -o sender sender.cpp
4. Then run the command: ./sender keyfile.txt

## Files
- `sender.cpp`: The sender program reads a file and sends its data contents to the receiver.
- `recv.cpp`: The receiver program reads shared memory data and processes it.
- `msg.h`: Defines the structures used for messages.
- `Makefile`: Makefile for building the project.
- `signaldemo.cpp`: Simple Signal Handler.

## Example Commands
```sh
# Compile the programs
make recv.cpp
make sender.cpp

# Start the receiver
./recv

# Send a file
./sender <file_name>

#or
#Creating file
echo “Hello World” > keyfile.txt
#Compile C++ source to executables:
clang++ -o recv recv.cpp
clang -o sender sender.cpp
#Run the code 
./sender Keyfile.txt
```
# Testing Screenshots
[Showcase of Terminal Output after starting Sender]
![Showcase of Terminal Output after starting Sender](images/testSend.png)

[Showcase of Terminal Output after starting Receiver]
![Showcase of Terminal Output after starting Receiver](images/testReceive.png)

