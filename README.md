# STDIO
Implementation of a version of the C standard input/output library. The implementation is platform dependant. There is support for Linux.


Structure
-

1. Code structure

Linux 
------------------------|     
 │   ├── Makefile       |           
 │   ├── so_stdio.c     |            
 │   ├── so_stdio.h     |                           
 
 └── README.md         
 
 
 ***Flow***

The result of the project is a shared library which provides implementation for IO opperations.

All operations are buffered.

The library offers support for working with files:
  * Opening / Closing files.
  * Reading / Writing from files.
  * Adjusting the file cursor.

There is also support for creating processes:
  * A new process can be created to execute commands.
  * The process can, then, be closed safely.
  * The files created are available for read/ write opperations.
