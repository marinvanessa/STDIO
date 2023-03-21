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
  
  ***Implementation Details***
  
  All the required features were implemented.

Function | Location | Brief Description
------------ | ------------- | -------------
so_fopen | so_stdio.c | Gets the flags for opening a file, allocates memory for a file structure the opens the file
so_fclose | so_stdio.c | Flushes the buffer associated with the file, closes the opened stream and frees the memory of the file structure
so_fileno | so_stdio.c | Returns the file descriptor
so_fgetc | so_stdio.c | Gets a byte from the buffer associated with a file
so_fread | so_stdio.c | Gets n elements of specified size from a file (byte by byte -- is buffered)
so_putc | so_stdio.c | Puts a byte in the buffer associated with a file
so_fwrite | so_stdio.c | Puts n elements of specified size to a file (byte by byte -- is buffered)
so_fflush | so_stdio.c | Flushes the buffer associated to a file
so_fseek | so_stdio.c | Moves the cursor associated to a file
so_ftell | so_stdio.c | Returns the position of the file cursor
so_feof | so_stdio.c | Checks for eof
so_ferror | so_stdio.c | Checks for errors
so_popen | so_stdio.c | Openes a child process to execute command, redirecting the parent input/ output
so_pclose | so_stdio.c | Closes the file stream and waits for the child
