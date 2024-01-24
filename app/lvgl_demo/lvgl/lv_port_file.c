#include "main.h"
#include "lv_port_file.h"

#if 1//LV_USE_FILESYSTEM

/**
 * Open a file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable
 * @param path path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    FILE *file_p;

    if (mode == LV_FS_MODE_WR)
        file_p = fopen(path, "wb+");
    else if (mode == LV_FS_MODE_RD)
        file_p = fopen(path, "rb");
    else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
        file_p = fopen(path, "wb+");

    return (void *)file_p;
}


/**
 * Close an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable. (opened with lv_ufs_open)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p)
{
    lv_fs_res_t res = LV_FS_RES_OK;

    /* Add your code here*/
    fclose((file_p));

    return res;
}

/**
 * Read data from an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable.
 * @param buf pointer to a memory block where to store the read data
 * @param btr number of Bytes To Read
 * @param br the real number of read bytes (Byte Read)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    lv_fs_res_t res = LV_FS_RES_OK;

    /* Add your code here*/
    *br = fread((char *)buf, 1, btr, file_p);
    if (*br == 0)
        printf("ftell %d %p %d\n", ftell(file_p), buf, btr);

    return res;
}

static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
    lv_fs_res_t res = LV_FS_RES_OK;

    /* Add your code here*/
    *bw = fwrite((char *)buf, 1, btw, file_p);

    return res;
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable. (opened with lv_ufs_open )
 * @param pos the new position of read write pointer
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    lv_fs_res_t res = LV_FS_RES_OK;

    /* Add your code here*/
    fseek(file_p, pos, whence);

    return res;
}

static lv_fs_res_t fs_tell(struct _lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    lv_fs_res_t res = LV_FS_RES_OK;

    *pos_p = ftell(file_p);

    return res;
}

void lv_port_fs_init(void)
{
    static lv_fs_drv_t fs_drv = {0};

    lv_fs_drv_init(&fs_drv);

    /*Set up fields...*/
    fs_drv.letter = 'A';
    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;

    lv_fs_drv_register(&fs_drv);
}
#endif
