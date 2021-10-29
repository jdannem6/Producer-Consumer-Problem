# The Producer-Consumer Problem
### CS33211: Operating Systems
This project is an implementation of the Producer-Consumer Problem using shared memory and semaphores.

## The Producer-Consumer Problem
In Computer Science, concurrency, or the ability to run parts of a program or procedure at the same time, is a much desired feature due to the significantly increased computation speeds it often brings with it. It is, however, not without its limitations. Problems can arise when two or more processes, threads, etc., read and write from the same memory locations because the operations of the concurrent processes are not always atomic and operations on the memory space may not occur in the intended sequence. Such cases often require the use of synchronization primitives like semaphores or mutex locks to constrain the order of operations. The Producer-Consumer Problem is a classic example of such synchronization that consists of two processes, the producer and the consumer. THe op

![](ExampleOutput.png)
