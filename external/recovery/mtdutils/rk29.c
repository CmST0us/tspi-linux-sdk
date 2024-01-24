#define _GNU_SOURCE

#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <errno.h>
#include "mtdutils.h"
#include "rk29.h"

int run(const char *filename, char *const argv[])
{
    struct stat s;
    int status;
    pid_t pid;

    if (stat(filename, &s) != 0) {
        fprintf(stderr, "cannot find '%s'", filename);
        return -1;
    }

    printf("executing '%s'\n", filename);

    pid = fork();

    if (pid == 0) {
        setpgid(0, getpid());
        /* execute */
        execv(filename, argv);
        fprintf(stderr, "can't run %s (%s)\n", filename, strerror(errno));
        /* exit */
        _exit(0);
    }

    if (pid < 0) {
        fprintf(stderr, "failed to fork and start '%s'\n", filename);
        return -1;
    }

    if (-1 == waitpid(pid, &status, WCONTINUED | WUNTRACED)) {
        fprintf(stderr, "wait for child error\n");
        return -1;
    }

    if (WIFEXITED(status)) {
        printf("executed '%s' done\n", filename);
    }

    printf("executed '%s' return %d\n", filename, WEXITSTATUS(status));
    return 0;
}

int rk_check_and_resizefs(const char *filename)
{
    int result;

    const char *const e2fsck_argv[] = { "/sbin/e2fsck", "-fy", filename, NULL };
    const char *const resizefs_argv[] = { "/sbin/resize2fs", filename, NULL  };

    result = run(e2fsck_argv[0], (char **) e2fsck_argv);
    if (result) {
        printf("e2fsck check '%s' failed!\n", filename);
        return result;
    }

    result = run(resizefs_argv[0], (char **) resizefs_argv);
    if (result) {
        printf("resizefs '%s' failed!\n", filename);
    }

    return result;
}

int rk_check_and_resizefs_f2fs(const char *filename)
{
    int result;

    const char *const e2fsck_argv[] = { "fsck_f2fs", filename, NULL };
    const char *const resizefs_argv[] = { "resize.f2fs", filename, NULL  };

    result = run(e2fsck_argv[0], (char **) e2fsck_argv);
    if (result) {
        printf("fsck_f2fs check '%s' failed!\n", filename);
        return result;
    }

    result = run(resizefs_argv[0], (char **) resizefs_argv);
    if (result) {
        printf("resize.f2fs '%s' failed!\n", filename);
    }

    return result;
}

static int make_extfs(const char *path, const char *label, const char *type)
{
    const char *const mke2fs[] = {
        "/sbin/mke2fs", "-t", type, "-q", path, NULL,
    };

    // max-mount-counts(0) + time-dependent checking(0) + fslabel
    const char *const tune2fs[] = {
        "/sbin/tune2fs", "-c", "0", "-i", "0", "-L", label, path, NULL,
    };
    int result;

    printf("format '%s' to %s filesystem\n", path, type);
    result = run(mke2fs[0], (char **) mke2fs);
    if (result) {
        printf("failed!\n");
        return result;
    }

    result = run(tune2fs[0], (char **) tune2fs);
    if (result) {
        printf("failed!\n");
        return result;
    }

    return result;
}

int make_ext2(const char *path, const char *label)
{
    return make_extfs(path, label, "ext2");
}

int make_ext4(const char *path, const char *label)
{
    return make_extfs(path, label, "ext4");
}

int make_vfat(const char *path, const char *label)
{
    // fat32
    const char *const mkdosfs[] = {
        "/sbin/mkdosfs", "-F", "32", "-n", label, path, NULL,
    };

    printf("format '%s' to vfat filesystem\n", path);
    return run(mkdosfs[0], (char **) mkdosfs);
}

int make_ntfs(const char *path, const char *label)
{
    // compression
    const char *const mkntfs[] = {
        "mkntfs", "-F", "C", "Q", "-L", label, path, NULL,
    };

    printf("format '%s' to ntfs filesystem\n", path);
    return run(mkntfs[0], (char **) mkntfs);
}

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

size_t rk29_fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    char buf[READ_SIZE];
    int fd;
    long begin, end;
    off_t offset;
    ssize_t sz;
    size_t count = 0, total;
    char *p = ptr;

    if (!ptr)
        return 0;
    if (!size || !nmemb)
        return 0;
    if (!stream)
        return 0;
    fd = fileno(stream);
    if (fd < 0)
        return 0;

    begin = ftell(stream);
    if (begin < 0)
        begin = 0;

    total = size * nmemb;
    if (!total)
        return 0;

    end = begin + total;
    offset = begin & ~READ_MASK;

    if (begin & READ_MASK) {
        sz = pread(fd, buf, READ_SIZE, offset);
        if (sz < READ_SIZE)
            goto out;
        count = min(end, offset + READ_SIZE) - begin;
        memcpy(p, buf + (begin & READ_MASK), count);
        p += count;
        offset += READ_SIZE;
    }

    for (; offset < (end & ~READ_MASK); offset += READ_SIZE) {
        sz = pread(fd, buf, READ_SIZE, offset);
        if (sz < READ_SIZE)
            goto out;
        count += READ_SIZE;
        memcpy(p, buf, READ_SIZE);
        p += READ_SIZE;
    }

    if (count < total && (end & READ_MASK)) {
        offset = end & ~READ_MASK;
        sz = pread(fd, buf, READ_SIZE, offset);
        if (sz < READ_SIZE)
            goto out;
        memcpy(p, buf, end - offset);
        count += end - offset;
    }
out:
    count /= size;
    fseek(stream, begin + count * size, SEEK_SET);
    return count;
}

size_t rk29_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    char buf[WRITE_SIZE];
    int fd;
    long begin, end;
    off_t offset;
    ssize_t sz;
    size_t count = 0, total;
    char *p = (char *)ptr;

    if (!ptr)
        return 0;
    if (!size || !nmemb)
        return 0;
    if (!stream)
        return 0;
    fd = fileno(stream);
    if (fd < 0)
        return 0;

    begin = ftell(stream);
    if (begin < 0)
        begin = 0;

    total = size * nmemb;
    if (!total)
        return 0;

    end = begin + total;
    offset = begin & ~WRITE_MASK;

    if (begin & WRITE_MASK) {
        sz = pread(fd, buf, WRITE_SIZE, offset);
        if (sz < WRITE_SIZE)
            goto out;
        count = min(end, offset + WRITE_SIZE) - begin;
        memcpy(buf + (begin & WRITE_MASK), p, count);
        sz = pwrite(fd, buf, WRITE_SIZE, offset);
        if (sz < WRITE_SIZE)
            goto out;
        p += count;
        offset += WRITE_SIZE;
    }

    for (; offset < (end & ~WRITE_MASK); offset += WRITE_SIZE) {
        sz = pwrite(fd, p, WRITE_SIZE, offset);
        if (sz < WRITE_SIZE)
            goto out;
        count += WRITE_SIZE;
        p += WRITE_SIZE;
    }

    if (count < total && (end & WRITE_MASK)) {
        offset = end & ~WRITE_MASK;
        sz = pread(fd, buf, WRITE_SIZE, offset);
        if (sz < WRITE_SIZE)
            goto out;
        memcpy(buf, p, end - offset);
        sz = pwrite(fd, buf, WRITE_SIZE, offset);
        if (sz < WRITE_SIZE)
            goto out;
        count += end - offset;
    }
out:
    count /= size;
    fseek(stream, begin + count * size, SEEK_SET);
    return count;
}

