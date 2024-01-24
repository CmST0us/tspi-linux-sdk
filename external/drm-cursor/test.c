#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <xf86drm.h>
#include <sys/mman.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#define CURSOR_WIDTH 64
#define CURSOR_HEIGHT 64

int main(int argc, const char **argv)
{
  int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
  int width = CURSOR_WIDTH;
  int height = CURSOR_HEIGHT;
  int crtc_id = 0;

  struct drm_mode_create_dumb create_arg = {
    .width = width,
    .height = height,
    .bpp = 32,
  };

  drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_arg);
  int handle = create_arg.handle;
  int size = create_arg.size;

  struct drm_mode_map_dumb map_arg = {
    .handle = handle,
  };

  drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_arg);

  int *ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                  fd, map_arg.offset);
  for (int i = 0; i < width * height; i++)
        ptr[i] = 0x4F000000 | (i % width) * 2 << 16 | (i / height) << 8;

  if (argc > 1)
    crtc_id = atoi(argv[1]);

  if (argc > 2)
    setenv("DRM_CURSOR_PREFER_PLANE", argv[2], 1);

  drmModeSetCursor(fd, crtc_id, handle, width, height);

  for (int i = 0; i < 100000; i++) {
    drmModeMoveCursor(fd, crtc_id, i % 1024, i % 1024);
    usleep(100000);
  }

  return 0;
}
