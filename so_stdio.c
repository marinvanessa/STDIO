#include "so_stdio.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE  4096
#define MODE_T  0644
#define NOPREV 0
#define ERRNO 2

typedef struct _so_file {

	int fd; // filedescriptor
	char previous_op; // ne spune care a fost ultima operatie

	char buff[BUFFER_SIZE];
	int buff_index; // ne spune unde in buffer ne aflam
	int buff_status; // ne spune daca bufferul e gol sau nu

	int eof; // ne spune daca am ajuns la sfarsitul fisierului sau nu
	int code_err;

	int file_cursor;

	int elem_written_file;
	int elem_read_file;

	pid_t pid;

	int buff_start; // prima pozitie din buffer
	int buff_end; // ultima pozitie din buffer

} SO_FILE;


// seteaza steagurile pentru operatia de open() din linux man in functie de modurile de acces
int set_flags(const char *mode)
{
	int flags = -1;

	if (strcmp(mode, "r") == 0)
		flags = O_RDONLY;

	if (strcmp(mode, "w") == 0)
		flags = O_WRONLY | O_CREAT | O_TRUNC;

	if (strcmp(mode, "a") == 0)
		flags = O_WRONLY | O_CREAT | O_APPEND;

	if (strcmp(mode, "r+") == 0)
		flags = O_RDWR;

	if (strcmp(mode, "w+") == 0)
		flags = O_RDWR | O_CREAT | O_TRUNC;

	if (strcmp(mode, "a+") == 0)
		flags = O_RDWR | O_CREAT | O_APPEND;

	return flags;
}


// deschide un fisier si returneaza structura alocata sau null daca apare vreo eroare
SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *f;

	f = (SO_FILE *)malloc(1 * sizeof(struct _so_file));

	if (f == NULL)
		return NULL;

	f->fd = open(pathname, set_flags(mode), MODE_T);

	if (f->fd < 0) {
		free(f);
		return NULL;
	}


	f->buff_index = 0;
	f->buff_start = 0;
	f->buff_end = BUFFER_SIZE - 1;
	memset(f->buff, 0, f->buff_end + 1);

	f->buff_status = 0; // buffer este gol


	f->eof = 0; // nu am ajuns la sfarsit de fisier
	f->code_err = 0; // nu am erori
	f->previous_op = NOPREV;
	f->file_cursor = 0;

	f->pid = -1;

	f->elem_read_file = 0;
	f->elem_written_file = 0;



	return f;
}


// inchide fisierul primit ca parametru si elibereaza memoria ocupata de structura mea
int so_fclose(SO_FILE *stream)
{

	int do_fflush = stream->buff_status == 1 ? so_fflush(stream) : 0;

	int res_close = close(stream->fd) == 0 ? do_fflush : -1;

	free(stream);

	return res_close;

}

// citesc din fisier in buffer
int read_buffer(SO_FILE *stream, int no_bytes_read, int curr_bytes_read)
{
	while (no_bytes_read < (stream->buff_end + 1)) {
		curr_bytes_read = read(stream->fd, stream->buff + no_bytes_read,
								(stream->buff_end + 1) - no_bytes_read);

		if (curr_bytes_read < 0) {
			stream->code_err = ERRNO;
			break;
		}


		if (curr_bytes_read == 0) {
			stream->eof = 1;
			break;
		}

		no_bytes_read += curr_bytes_read;


		if (curr_bytes_read < (stream->buff_end + 1))
			break;

	}

	return no_bytes_read;

}

// citeste din buffer numarul de biti si seteaza indexul acestuia cu numarul de biti cititi, returnez caracterul
// de la pozitia data de index in buffer
int so_fgetc(SO_FILE *stream)
{
	if (stream->buff_index == stream->buff_start || stream->buff_end < stream->buff_index) {
		int elem_read = read_buffer(stream, 0, 0);

		if (elem_read == 0)
			stream->eof = 1;

		if (so_feof(stream) == 1)
			return SO_EOF;

		stream->buff_end = elem_read - 1;
		stream->buff_index = 0;
	}
	stream->buff_status = 1; // bufferul nu e gol :C
	stream->eof = 0;

	stream->previous_op = 'r';

	unsigned char result = stream->buff[stream->buff_index];
	(stream->buff_index)++;

	return (int) result;
}

// scrie in fisiser atat timp cat numarul de elemente din buffer este mai mi decat numarul de biti pe care
// vreau sa-i scriu, pentru fiecare situatie semnalez si rezolv
int write_buff(SO_FILE *stream, int counter, int no_bytes_written, int curr_bytes)
{
	while (no_bytes_written < counter) {
		curr_bytes = write(stream->fd, stream->buff + no_bytes_written,
									counter - no_bytes_written);

		if (curr_bytes < 0) {
			stream->code_err = ERRNO;
			return -ERRNO;
		}

		if (curr_bytes == 0) {
			stream->code_err = ERRNO;
			break;
		}

		no_bytes_written += curr_bytes;

		if (no_bytes_written < curr_bytes) {
			stream->code_err = ERRNO;
			break;
		}

	}

	return no_bytes_written;
}


// citesc biti din buffer in fisier si/sau imi invalideaza bufferul
int so_fflush(SO_FILE *stream)
{
	if (stream->previous_op == 'w') {
		if (write_buff(stream, stream->buff_index, 0, 0) < stream->buff_index)
			return SO_EOF;
	}
	stream->buff_index = 0;
	stream->buff_start = 0;
	stream->buff_end = BUFFER_SIZE - 1;
	memset(stream->buff, 0, stream->buff_end + 1);
	stream->buff_status = 0; // there is no data in my buff
	return 0;

}

// scrie un singur caracter in fisier
int so_fputc(int c, SO_FILE *stream)
{
	if (stream->buff_index == stream->buff_end) {
		write_buff(stream, stream->buff_index, 0, 0);

		stream->buff_status = 0; // buffer gol
		stream->buff_index = 0;

	}

	stream->previous_op = 'w';
	stream->buff_status = 1; // bufferul nu este gol :c

	stream->buff[stream->buff_index] = c;
	stream->buff_index++;
	return (int)((unsigned char)(c));

}

int so_fileno(SO_FILE *stream)
{
	int file_descriptor = 0;

	if (stream != NULL)
		file_descriptor = stream->fd;
	else
		file_descriptor = -1;
	return file_descriptor;
}


// citeste un numar de elemente din fisier si le stocheaza in ptr
size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int elem = 0, counter = 0;

	while (elem < nmemb) {
		for (int i = 0; i < size; i++) {
			int ch = so_fgetc(stream);

			if (ch == SO_EOF)
				break;

			*((unsigned char *)ptr) = ch;
			ptr++;
		}

		// verific daca am ajuns la sfarsitul fisierului
		if (so_feof(stream) == 1)
			break;

		counter++;
		elem++;
	}

	stream->elem_read_file = counter;
	return counter;

}

// scrie un numar de elemente la adresa de memorie specificata de ptr
size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{

	int elem = 0, counter = 0;

	while (elem < nmemb) {
		for (int i = 0; i < size; i++) {
			int ch = *((unsigned char *)ptr);

			if (so_fputc(ch, stream) == SO_EOF)
				break;
			ptr++;
		}

		// verific daca am intampinat vreo problema
		if (so_ferror(stream) == ERRNO)
			break;

		counter++;
		elem++;
	}

	stream->elem_written_file = counter;
	return counter;
}


// muta cursorul fisierului
int so_fseek(SO_FILE *stream, long offset, int whence)
{
	// mai intai apelez so_fflush pentru a-mi elibera bufferul in cazul in care acesta are elemente in el
	// si am o operatie de scriere inainte, invalideaza bufferul pentru celelalte operatii
	so_fflush(stream);

	stream->file_cursor = lseek(stream->fd, offset, whence);

	if (stream->file_cursor >= 0)
		return 0;
	else
		return -1;

}


// muta cursorul fisierului in functie de elementele citite/scrie din/in fisier
int set_cur_file(SO_FILE *stream)
{
	if (stream->previous_op == 'r')
		stream->file_cursor = stream->elem_read_file;
	if (stream->previous_op == 'w')
		stream->file_cursor = stream->elem_written_file;

	return stream->file_cursor;
}


// returneaza pozitia cusorului fisierului
long so_ftell(SO_FILE *stream)
{
	int _curr = set_cur_file(stream);

	return (long) _curr;
}

// imi creeaza un proces copil pentru executarea comenzii si redirecteaza intrarea/iesirea procesului parinte
SO_FILE *so_popen(const char *command, const char *type)
{
	SO_FILE *ret_file;
	int filedes[2];

	for (int i = 0; i < 2; i++)
		filedes[i] = 0;

	int new_fd = 0;
	pid_t pid = -1;

	// creeaza pipe
	int my_pipe = pipe(filedes);

	if (my_pipe == 0) { // procesul a fost creat cu succes
		pid = fork();
		switch (pid) {
		case -1: // avem eroare
			close(filedes[1]);
			close(filedes[0]);
			return NULL;
		case 0:
			if (*type == 'r') {
				close(filedes[0]);
				dup2(filedes[1], STDOUT_FILENO);
				close(filedes[1]);
			}
			if (*type == 'w') {
				close(filedes[1]);
				dup2(filedes[0], STDIN_FILENO);
				close(filedes[0]);
			}
			execl("/bin/sh", "sh", "-c", command, NULL);
			exit(-1);
		default:
			if (*type == 'r') {
				close(filedes[1]);
				new_fd = filedes[0];
			}
			if (*type == 'w') {
				close(filedes[0]);
				new_fd = filedes[1];
			}
			ret_file = (SO_FILE *)malloc(1 * sizeof(struct _so_file));

			if (ret_file == NULL)
				return NULL;

			ret_file->fd = new_fd;

			ret_file->pid = pid;
			ret_file->buff_index = 0;

			ret_file->eof = 0;
			ret_file->buff_end = BUFFER_SIZE - 1;
			ret_file->buff_start = 0;
			memset(ret_file->buff, 0, ret_file->buff_end + 1);
			ret_file->code_err = 0;
			ret_file->previous_op = NOPREV;

			ret_file->elem_read_file = 0;
			ret_file->elem_written_file = 0;


			return ret_file;
		}


	}

	return NULL;

}

// inchide fisierul si asteapta procesul copil
int so_pclose(SO_FILE *stream)
{
	int status;
	pid_t pid = stream->pid;

	so_fclose(stream);

	if (waitpid(pid, &status, 0) == -1)
		return -1;
	return status;

}

// returneaza indicatorul de sfarsit de fisier
int so_feof(SO_FILE *stream)
{
	return stream->eof;
}


// returneaza codul de eroare
int so_ferror(SO_FILE *stream)
{
	int err = stream->code_err;
	return err;
}
