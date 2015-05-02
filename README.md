# memcached

A simple memcached implementaion with support of getter/setter


## Prerequisite

Developed and well tested under MacOS. It doesn't work on Windows. Not yet tested on other linux family system.

## Running the server

Make sure you have 11211 port available for the application.
Check out the repository and run the following command.
``` bash
make
./server
```
Server will be running on 11211 port.

test-client/test.python is a simple client for verification.

## Design decisions
1. Multi-threading:
Main thread is responsible of accepting socket connections. After accepting TCP connection, a new thread will be created to handle the response.

2. Map implementation:
Map is implemented by using linked list. It is a bad choice since the lookup of linked list has O(n) time. It is implemented this way solely because it is easy to develop within a short time.

3. Locking:
Every entry in the map has a lock. Implemented using linux pthread rwlock.