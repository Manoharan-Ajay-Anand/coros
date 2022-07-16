# Coros

## About

Coros is a simple TCP server framework built with C++ 20 coroutines. The use of coroutines enables Coros to be highly concurrent even with a small number of threads.  

## How it Works

In order to understand how Coros works, you need to understand how coroutines work. Coroutines are 
simply functions that can be suspended and resumed while keeping it's state intact. This is 
especially useful for concurrency as a coroutine can be suspended when it's performing a blocking 
operation and be resumed from the suspension point after the operation is complete. During 
the suspension, the underlying thread that is running the coroutine can perform other operations 
without idling.

For every new socket connection, Coros spawns a new coroutine which is picked up by a thread in 
it's thread pool. The coroutine is able to read/write from the socket. When a socket operation ends 
up blocking, such as when waiting for a client to send data, the coroutine is suspended and 
Coros's event monitor is signalled. The underlying thread picks up other jobs such as 
serving another socket connection. Coros's event monitor will then resume the coroutine, on any one 
of the free threads in the thread pool, once the socket is ready for read/write operations.
