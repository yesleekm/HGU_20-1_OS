#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#define MAXBUF 200
#define FIFO ".ddtrace"

void * pthread_mutex_lock (pthread_mutex_t *m)
{
	// void *callstack[MAXBUF];
	// int frames;
	// char **strs;
	// frames = backtrace(callstack, MAXBUF);
	// strs = backtrace_symbols(callstack, frames);
	// for (int i = 0; i < frames; i++)
	// 	printf("[%d] at %d [%p] %s\n", i, syscall(SYS_gettid), callstack[i], strs[i]);
	// free(strs);

	void* (*pthread_mutex_lock_cp)(pthread_mutex_t *mutex);
	pthread_mutex_lock_cp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;

	char buf[MAXBUF];
	pid_t tid = syscall(SYS_gettid);
	sprintf(buf, "Lock %p from %d\n", m, tid);
	int fd = open(FIFO, O_WRONLY | O_SYNC) ;
    write(fd, buf, MAXBUF);

	pthread_mutex_lock_cp(m);
}

void * pthread_mutex_unlock (pthread_mutex_t *m)
{
	void* (*pthread_mutex_unlock_cp)(pthread_mutex_t *mutex);
	pthread_mutex_unlock_cp = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;

	char buf[MAXBUF];
	pid_t tid = syscall(SYS_gettid);
	sprintf(buf, "Unlock %p from %d\n", m, tid);
	int fd = open(FIFO, O_WRONLY | O_SYNC) ;
    write(fd, buf, MAXBUF);

	pthread_mutex_unlock_cp(m);
}