/******************************************************************************\
 *  $Id: util-file.c,v 1.1 2001/09/06 21:50:52 dun Exp $
 *    by Chris Dunlap <cdunlap@llnl.gov>
 ******************************************************************************
 *  Refer to "util-file.h" for documentation on public functions.
\******************************************************************************/


#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "errors.h"
#include "util-file.h"


static int get_file_lock(int fd, int cmd, int type);
static pid_t test_file_lock(int fd, int type);


void set_descriptor_nonblocking(int fd)
{
    int fval;

    if ((fval = fcntl(fd, F_GETFL, 0)) < 0)
        err_msg(errno, "fcntl(F_GETFL) failed");
    if (fcntl(fd, F_SETFL, fval | O_NONBLOCK) < 0)
        err_msg(errno, "fcntl(F_SETFL) failed");
    return;
}


int get_read_lock(int fd)
{
    return(get_file_lock(fd, F_SETLK, F_RDLCK));
}


int get_readw_lock(int fd)
{
    return(get_file_lock(fd, F_SETLKW, F_RDLCK));
}


int get_write_lock(int fd)
{
    return(get_file_lock(fd, F_SETLK, F_WRLCK));
}


int get_writew_lock(int fd)
{
    return(get_file_lock(fd, F_SETLKW, F_WRLCK));
}


int release_lock(int fd)
{
    return(get_file_lock(fd, F_SETLK, F_UNLCK));
}


pid_t is_read_lock_blocked(int fd)
{
    return(test_file_lock(fd, F_RDLCK));
}


pid_t is_write_lock_blocked(int fd)
{
    return(test_file_lock(fd, F_WRLCK));
}


static int get_file_lock(int fd, int cmd, int type)
{
    struct flock lock;

    assert(fd >= 0);

    lock.l_type = type;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;

    return(fcntl(fd, cmd, &lock));
}


static pid_t test_file_lock(int fd, int type)
{
    struct flock lock;

    assert(fd >= 0);

    lock.l_type = type;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;

    if (fcntl(fd, F_GETLK, &lock) < 0)
        err_msg(errno, "Unable to test for file lock");
    if (lock.l_type == F_UNLCK)
        return(0);
    return(lock.l_pid);
}


ssize_t read_n(int fd, void *buf, size_t n)
{
    size_t nleft;
    ssize_t nread;
    unsigned char *p;

    p = buf;
    nleft = n;
    while (nleft > 0) {
        if ((nread = read(fd, p, nleft)) < 0) {
            if (errno == EINTR)
                continue;
            else
                return(-1);
        }
        else if (nread == 0) {		/* EOF */
            break;
        }
        nleft -= nread;
        p += nread;
    }
    return(n - nleft);
}


ssize_t write_n(int fd, void *buf, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    unsigned char *p;

    p = buf;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, p, nleft)) < 0) {
            if (errno == EINTR)
                continue;
            else
                return(-1);
        }
        nleft -= nwritten;
        p += nwritten;
    }
    return(n);
}


ssize_t read_line(int fd, void *buf, size_t maxlen)
{
    ssize_t n, rc;
    unsigned char c, *p;

    n = 0;
    p = buf;
    while (n < maxlen - 1) {		/* reserve space for NUL-termination */

        if ((rc = read(fd, &c, 1)) == 1) {
            n++;
            *p++ = c;
            if (c == '\n')
                break;			/* store newline, like fgets() */
        }
        else if (rc == 0) {
            if (n == 0)			/* EOF, no data read */
                return(0);
            else			/* EOF, some data read */
                break;
        }
        else {
            if (errno == EINTR)
                continue;
            return(-1);
        }
    }

    *p = '\0';				/* NUL-terminate, like fgets() */
    return(n);
}