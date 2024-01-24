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

//int rk_make_ext4fs(const char *filename, s64 len)
//{
//    const char *const mke2fs_argv[] = { "/sbin/mke2fs", "-t", "ext4", "-O", "^huge_file", "-m", "0", "-q", filename, NULL };
//    printf("format '%s' to ext4 filesystem\n", filename);
//    return run(mke2fs_argv[0], (char **) mke2fs_argv);
//}

int rk_make_ext4fs(const char *filename,long long len, const char *mountpoint)
{
    int result;

    const char *const mke2fs_argv[] = { "/usr/sbin/mke2fs", "-t", "ext4", "-b", "4096", "-O", "^huge_file", "-m", "0", "-q", "-F", filename, NULL };
    const char *const e2fsck_argv[] = { "/usr/sbin/e2fsck", "-fy", filename, NULL };
    printf("format '%s' to ext4 filesystem\n", filename);
    result = run(mke2fs_argv[0], (char **) mke2fs_argv);
    if(result) {
        printf("format '%s' to ext4 error!\n", filename);
        return result;
    }

    result = run(e2fsck_argv[0], (char **) e2fsck_argv);
    if(result) {
        printf("e2fsck check '%s' fail!\n", filename);
        return result;
    }

    if(mountpoint != NULL) {
        int result2 = mount(filename, mountpoint, "ext4",
                                   MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");
        if(result2) {
            printf("mount '%s' to %s fail!\n", filename, mountpoint);
        }
    }

    return result;
}

int rk_check_and_resizefs(const char *filename) {
    int result;

    const char *const e2fsck_argv[] = { "/usr/sbin/e2fsck", "-fy", filename, NULL };
    const char *const resizefs_argv[] = { "/usr/sbin/resize2fs", filename, NULL  };

    result = run(e2fsck_argv[0], (char **) e2fsck_argv);
    if(result) {
        printf("e2fsck check '%s' failed!\n", filename);
        return result;
    }

    result = run(resizefs_argv[0], (char **) resizefs_argv);
    if(result) {
        printf("resizefs '%s' failed!\n", filename);
    }

    return result;
}

int rk_check_and_resizefs_f2fs(const char *filename) {
	int result;

	const char *const e2fsck_argv[] = { "fsck_f2fs", filename, NULL };
	const char *const resizefs_argv[] = { "resize.f2fs", filename, NULL  };

	result = run(e2fsck_argv[0], (char **) e2fsck_argv);
	if(result) {
		printf("fsck_f2fs check '%s' failed!\n", filename);
		return result;
	}

	result = run(resizefs_argv[0], (char **) resizefs_argv);
	if(result) {
		printf("resize.f2fs '%s' failed!\n", filename);
	}

	return result;
}


int rk_make_ext3fs(const char *filename)
{
    int result;

    const char *const mke2fs_argv[] = { "mke2fs", "-t", "ext3", "-b", "4096", "-O", "^huge_file", "-m", "0", "-q", filename, NULL };
    const char *const e2fsck_argv[] = { "e2fsck", "-fy", filename, NULL };
    printf("format '%s' to ext3 filesystem\n", filename);
    result = run(mke2fs_argv[0], (char **) mke2fs_argv);
    if(result) {
        printf("format '%s' to ext3 fail!\n", filename);
        return result;
    }

    result = run(e2fsck_argv[0], (char **) e2fsck_argv);
    if(result) {
        printf("e2fsck check '%s' fail!\n", filename);
    }

    return result;
}

int rk_make_ext2fs(const char *filename)
{
    int result;

    const char *const mke2fs_argv[] = { "/sbin/mke2fs", "-t", "ext2", "-b", "4096", "-O", "^huge_file", "-m", "0", "-q", filename, NULL };
    const char *const e2fsck_argv[] = { "/sbin/e2fsck", "-fy", filename, NULL };
    printf("format '%s' to ext2 filesystem\n", filename);
    result = run(mke2fs_argv[0], (char **) mke2fs_argv);
    if(result) {
        printf("format '%s' to ext2 fail!\n", filename);
        return result;
    }

    result = run(e2fsck_argv[0], (char **) e2fsck_argv);
    if(result) {
        printf("e2fsck check '%s' fail!\n", filename);
    }

    return result;
}

int make_vfat(const char *filename,const char* volumelabel)
{
    printf("format '%s' to vfat filesystem\n", filename);
    if(volumelabel == NULL){
         const char *const mke2fs_argv[] = { "/sbin/mkdosfs", filename, NULL };
         return run(mke2fs_argv[0], (char **) mke2fs_argv);
    }else{
         const char *const mke2fs_withLabel_argv[] = { "/sbin/mkdosfs", "-n", volumelabel, filename, NULL };
         return run(mke2fs_withLabel_argv[0], (char **) mke2fs_withLabel_argv);
    }
}

int make_ntfs(const char *filename,const char* volumelabel) {
    printf("format '%s' to NTFS filesystem.\n", filename);

    if(volumelabel == NULL){
         const char *const mkntfs_argv[] = { "mkntfs", "-f", filename, NULL };
         return run(mkntfs_argv[0], (char **) mkntfs_argv);
    }else{
         const char *const mkntfs_withLabel_argv[] = { "mkntfs", "-f", "-L", volumelabel, filename, NULL };
         return run(mkntfs_withLabel_argv[0], (char **) mkntfs_withLabel_argv);
    }
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

