/*-
 * Copyright 2003-2005 Colin Percival
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if 0
__FBSDID("$FreeBSD: src/usr.bin/bsdiff/bspatch/bspatch.c,v 1.1 2005/08/06 01:59:06 cperciva Exp $");
#endif

#include <bzlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <stdbool.h>
#include "md5sum.h"

static off_t offtin(unsigned char *buf)
{
    off_t y;

    y = buf[7] & 0x7F;
    y = y * 256; y += buf[6];
    y = y * 256; y += buf[5];
    y = y * 256; y += buf[4];
    y = y * 256; y += buf[3];
    y = y * 256; y += buf[2];
    y = y * 256; y += buf[1];
    y = y * 256; y += buf[0];

    if (buf[7] & 0x80) y = -y;

    return y;
}

/* Return:
 * -1 patching error,
 *  0 not a diff img,
 * >0 the new image size
 * dst_file size if patch successfully
 */
int do_patch_rkimg(const char *img, ssize_t offset, ssize_t size,
                   const char *blk_dev, const char *dst_file)
{
#define TAIL_SIZE   80
#define TID_HEAD    0
#define TID_NAME    1
#define TID_OLD_SIZE    2
#define TID_NEW_SIZE    3
#define TID_MD5SUM  4
#define TOKENS      5
#define MAGIC_TAIL  "DIFF"
#define MD5SUM_LEN  32

    // For tail parsing.
    // Tail size is 80 bytes as like,
    //   "DIFF:%-15s:%-12s:%-12s:%-32s:"
    //   $name $old_size $new_size $md5sum
    int fd_img;
    const char *split = ": "; //space is a split char too
    char tail[TAIL_SIZE];
    ssize_t len, ret;
    ssize_t oldsize, newsize;
    char *saveptr, *str, *name, *md5sum, *token[TOKENS];
    int j;

    // For patching
    FILE * f = NULL, * cpf = NULL, * dpf = NULL, * epf = NULL;
    BZFILE * cpfbz2, * dpfbz2, * epfbz2;
    int cbz2err, dbz2err, ebz2err;
    int fd;
    ssize_t bzctrllen, bzdatalen;
    unsigned char header[32], buf[8];
    unsigned char *old  = NULL, *new_ptr = NULL;
    off_t oldpos, newpos;
    off_t ctrl[3];
    off_t lenread;
    off_t i;
    struct stat old_stat, dst_stat;

    if ((fd_img = open(img, O_RDONLY, 0)) < 0) {
        printf("open %s failed\n", img);
        return -1;
    }
    if (lseek(fd_img, offset + size - TAIL_SIZE, SEEK_SET) < 0) {
        printf("%s: lseek to: %ld failed: %s\n", img,
               offset + size - TAIL_SIZE, strerror(errno));
        close(fd_img);
        return -1;
    }
    len = 0;
    while (len != TAIL_SIZE) {
        ret = read(fd_img, tail + len, TAIL_SIZE - len);
        if (ret < 0) {
            printf("read %s tail err\n", img);
            close(fd_img);
            return -1;
        }
        len += ret;
    }
    close(fd_img);

    tail[TAIL_SIZE - 1] = '\0';
    for (j = 0, str = tail; j < TOKENS; j++, str = NULL) {
        token[j] = strtok_r(str, split, &saveptr);
        if (token[j] == NULL)
            break;
    }
    name = token[TID_NAME];
    md5sum = token[TID_MD5SUM];

    /* When unexpected reboot during patching/writing happened,
     * if dst_file is in correct state, then old image may already broken
     */
    if (stat(dst_file, &dst_stat) == 0 &&
        compareMd5sum(dst_file, (unsigned char *)md5sum, 0, dst_stat.st_size)) {
        printf("Recovery from unecptected reboot successfully.");
        return dst_stat.st_size;
    }
    /* If dst_file exist but md5sum is wrong, old image file is clean, hopefully */

    //check tail magic, return 0 if not exist
    if (j == 0 || strncmp(MAGIC_TAIL, token[TID_HEAD], strlen(MAGIC_TAIL)) != 0) {
        printf("Not a diff image, ret = %ld\n", ret);
        return 0;
    }
    printf("This is a diff image, patching...\n");
    if (j != TOKENS ||
        (oldsize = strtol(token[TID_OLD_SIZE], &saveptr, 10)) == 0 ||
        (errno == ERANGE && (oldsize == LONG_MAX || oldsize == LONG_MIN)) ||
        saveptr == token[TID_OLD_SIZE] ||
        (newsize = strtol(token[TID_NEW_SIZE], &saveptr, 10)) == 0 ||
        (errno == ERANGE && (newsize == LONG_MAX || newsize == LONG_MIN)) ||
        saveptr == token[TID_NEW_SIZE] ||
        strlen(token[TID_MD5SUM]) != MD5SUM_LEN) {
        printf("Bad Tail header of bsdiff patch\n");
        return -1;
    }

    //TODO: check dst_file dir size, return -1 if space too small.

    /* Open patch file */
    if ((f = fopen(img, "r")) == NULL) {
        printf("fopen %s err\n", img);
        return -1;
    }
    if (fseeko(f, offset, SEEK_SET)) {
        printf("fseeko %s err\n", img);
        fclose(f);
        return -1;
    }

    /*
    File format:
        0   8   "BSDIFF40"
        8   8   X
        16  8   Y
        24  8   sizeof(newfile)
        32  X   bzip2(control block)
        32+X    Y   bzip2(diff block)
        32+X+Y  ??? bzip2(extra block)
    with control block a set of triples (x,y,z) meaning "add x bytes
    from oldfile to x bytes from the diff block; copy y bytes from the
    extra block; seek forwards in oldfile by z bytes".
    */

    /* Read header */
    if (fread(header, 1, 32, f) < 32) {
        printf("Read header err\n");
        fclose(f);
        return -1;
    }
    fclose(f);
    f = NULL;

    /* Check for appropriate magic */
    if (memcmp(header, "BSDIFF40", 8) != 0) {
        printf("Bad header, Corrupt patch\n");
        return -1;
    }

    /* Read lengths from header */
    bzctrllen = offtin(header + 8);
    bzdatalen = offtin(header + 16);
    newsize = offtin(header + 24);
    if ((bzctrllen < 0) || (bzdatalen < 0) || (newsize <= 0)) {
        printf("Bad header len, Corrupt patch\n");
        return -1;
    }

    /* re-open patch file via libbzip2 at the right places */
    if ((cpf = fopen(img, "r")) == NULL)
        return -1;
    if (fseeko(cpf, offset + 32, SEEK_SET)) {
        printf("fseeko(%s, %lld) err\n", img, (long long)(32 + offset));
        goto cleanup;
    }
    if ((cpfbz2 = BZ2_bzReadOpen(&cbz2err, cpf, 0, 0, NULL, 0)) == NULL) {
        printf("BZ2_bzReadOpen, bz2err = %d, err\n", cbz2err);
        goto cleanup;
    }
    if ((dpf = fopen(img, "r")) == NULL)
        goto cleanup;
    if (fseeko(dpf, offset + 32 + bzctrllen, SEEK_SET)) {
        printf("fseeko(%s, %lld) err\n", img,
               (long long)(offset + 32 + bzctrllen));
        goto cleanup;
    }
    if ((dpfbz2 = BZ2_bzReadOpen(&dbz2err, dpf, 0, 0, NULL, 0)) == NULL) {
        printf("BZ2_bzReadOpen, bz2err = %d, err\n", dbz2err);
        goto cleanup;
    }
    if ((epf = fopen(img, "r")) == NULL) {
        printf("fopen(%s) err\n", img);
        goto cleanup;
    }
    if (fseeko(epf, offset + 32 + bzctrllen + bzdatalen, SEEK_SET)) {
        printf("fseeko(%s, %lld) err\n", img,
               (long long)(offset + 32 + bzctrllen + bzdatalen));
        goto cleanup;
    }
    if ((epfbz2 = BZ2_bzReadOpen(&ebz2err, epf, 0, 0, NULL, 0)) == NULL) {
        printf("BZ2_bzReadOpen, bz2err = %d\n", ebz2err);
        goto cleanup;
    }

    if (((fd = open(blk_dev, O_RDONLY, 0)) < 0) ||
        ((old = (unsigned char *)mmap(NULL, oldsize, PROT_READ,
                                      MAP_SHARED | MAP_POPULATE, fd, 0)) == MAP_FAILED)) {
        printf("open %s err\n", blk_dev);
        goto cleanup;
    }
    close(fd);
    fd = -1;

    /* mmap the new file */
    if (((fd = open(dst_file, O_CREAT | O_TRUNC | O_RDWR, 0666)) < 0) ||
        (lseek(fd, newsize - 1, SEEK_SET) != (newsize - 1)) ||
        (write(fd, "E", 1) != 1) ||
        (lseek(fd, 0, SEEK_SET) != 0) ||
        ((new_ptr = (unsigned char *)mmap(NULL, newsize, PROT_READ | PROT_WRITE,
                                          MAP_SHARED, fd, 0)) == MAP_FAILED)) {
        printf("mmap %s err\n", dst_file);
        goto cleanup;
    }
    close(fd);
    fd = -1;

    oldpos = 0; newpos = 0;
    while (newpos < newsize) {
        /* Read control data */
        for (i = 0; i <= 2; i++) {
            lenread = BZ2_bzRead(&cbz2err, cpfbz2, buf, 8);
            if ((lenread < 8) || ((cbz2err != BZ_OK) &&
                                  (cbz2err != BZ_STREAM_END))) {
                printf("Read control data: Corrupt patch\n");
                goto cleanup;
            }
            ctrl[i] = offtin(buf);
        };

        /* Sanity-check */
        if (newpos + ctrl[0] > newsize) {
            printf("Sanity-check: Corrupt patch\n");
            goto cleanup;
        }

        /* Read diff string */
        lenread = BZ2_bzRead(&dbz2err, dpfbz2, new_ptr + newpos, ctrl[0]);
        if ((lenread < ctrl[0]) ||
            ((dbz2err != BZ_OK) && (dbz2err != BZ_STREAM_END))) {
            printf("Read diff string: Corrupt patch\n");
            goto cleanup;
        }

        /* Add old data to diff string */
        for (i = 0; i < ctrl[0]; i++)
            if ((oldpos + i >= 0) && (oldpos + i < oldsize))
                new_ptr[newpos + i] += old[oldpos + i];

        /* Adjust pointers */
        newpos += ctrl[0];
        oldpos += ctrl[0];

        /* Sanity-check */
        if (newpos + ctrl[1] > newsize) {
            printf("Sanity-check: Corrupt patch\n");
            goto cleanup;
        }

        /* Read extra string */
        lenread = BZ2_bzRead(&ebz2err, epfbz2, new_ptr + newpos, ctrl[1]);
        if ((lenread < ctrl[1]) ||
            ((ebz2err != BZ_OK) && (ebz2err != BZ_STREAM_END))) {
            printf("Read extra string: Corrupt patch\n");
            goto cleanup;
        }

        /* Adjust pointers */
        newpos += ctrl[1];
        oldpos += ctrl[2];
    };

    /* Clean up the bzip2 reads */
    BZ2_bzReadClose(&cbz2err, cpfbz2);
    BZ2_bzReadClose(&dbz2err, dpfbz2);
    BZ2_bzReadClose(&ebz2err, epfbz2);
    fclose(cpf);
    fclose(dpf);
    fclose(epf);

    munmap(new_ptr, newsize);
    munmap(old, oldsize);
    sync();

    //check md5sum
    if (!compareMd5sum(dst_file, (unsigned char *)md5sum, 0, newsize))
        return -1;

    printf("Diff patch apply successfully for %s, size: %ld\n",
           name, newsize);
    return newsize;
cleanup:
    if (new_ptr != NULL)
        munmap(new_ptr, newsize);
    if (old != NULL)
        munmap(old, oldsize);
    if (fd >= 0)
        close(fd);
    if (cpf != NULL)
        fclose(cpf);
    if (dpf != NULL)
        fclose(dpf);
    if (epf != NULL)
        fclose(epf);
    return -1;
}

#if 0
int main(int argc, char *argv[])
{
    do_patch_rkimg("./update.img", 676276, 16964, "OLD/Image/uboot.img", "uboot.img");
    do_patch_rkimg("./update.img", 672180, 3589, "OLD/Image/trust.img", "trust.img");
    do_patch_rkimg("./update.img", 743860, 532546, "OLD/Image/boot.img", "boot.img");
    do_patch_rkimg("./update.img", 15630772, 88186412, "OLD/Image/rootfs.img", "rootfs.img");

    return 0;
}
#endif
