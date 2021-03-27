#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <cassert>

#define BUFFER_LEN 1024

// open flags
// O_RDONLY read only
// O_WRONLY write only
// O_CREATE create the file if the file don't exist.
// O_TRUNC truncate the length of the file to be 0 if the file can be written(write only or read write)
// O_NONBLOCK the opening of the file and IO should be nonblocking 
// O_APPEND append to the end of file on each write

// fopen() or open()? fopen is buffered, better performance, more functionality.

// off_t lseek(int fd, off_t offset, int flags)
// SEEK_SET, seek from the beginning of file.
// SEEK_CUR, seek from the current position
// SEEK_END, seek from the end of file.

/* Data structure of a file:
1. Each process has a table of open file descriptors. 
Each table entry has some file descriptor flag and a pointer to kernel file table entry.
2. Kernel maintains a file table for all open files.
Each kernel file table entry contains file status flag(read, write, append, sync, nonblocking),
Current file offset and a pointer to the v-node table entry for the file.
More than 1 file descriptors can point to the same kernel file table entry. 
3. Each open file or device has a in memory v-node containing information of the file. 
*/

/*
append a file:
1 use iseek to get to the end of file and write to it, can't be atomic.
2 open the file woth O_APPEND and write to it, append write is atomic.
*/

/* 
atomic seek and read/write
ssize_t pread(int fd, void* buf, size_t bytes, off_t offset);
ssize_t pwrite(int fd, const void* buf, size_t nbytes, off_t offset);
Doesn't update the current file offset.
*/

bool readn(int fd, size_t n) {
    static char buffer[BUFFER_LEN];
    while (n > 0) {
        size_t bytes_to_read = std::min((size_t)BUFFER_LEN, n);
        // bytes_read can be different from bytes_to_read, 
        // because read can be interrupted or don't have enough byte to read. 
        size_t bytes_read = read(fd, buffer, bytes_to_read);
        n -= bytes_read;
    }
    std::cout << buffer << std::endl;
    return n == 0;
}

bool writen(int fd, void* buf, size_t bytes_to_write) {
    int cur_off = 0;
    while (bytes_to_write > 0) {
        int bytes_written = write(fd, buf + cur_off, bytes_to_write);
        if (bytes_written < 0) break;
        bytes_to_write -= bytes_written;
        cur_off += bytes_written;
    }
    return bytes_to_write == 0;
}

void synDirtyPages() {
    // wait for disk write to complete.
    fsync(STDOUT_FILENO);
    // queue all the disk write buffer and return immediately 
    sync();
}

void duplicateFd(int fd) {
    int newfd1;
    int newfd2 = STDOUT_FILENO;
    newfd1 = dup(fd);

    close(newfd2);
    fcntl(fd, F_DUPFD, newfd2);;
    // equvalent to
    // dup2(fd, newfd2);

}

int main() {
    int f1 = open("file1", O_CREAT | O_WRONLY | O_TRUNC);
    int f2 = open("src/ls.c", O_RDONLY);
    off_t cur_off = lseek(f2, 0, SEEK_CUR);
    off_t file_len = lseek(f2, 0, SEEK_END);
    off_t middle = lseek(f2, -10, SEEK_END);
    assert(file_len - middle == 10);
    readn(f2, 8);
    std::cout << cur_off << ", " << file_len << std::endl;
    close(f1);
    close(f2);
}