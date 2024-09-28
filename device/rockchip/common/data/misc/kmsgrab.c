#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

static void *map_fb(int fd, int handle, size_t size) {
	struct drm_prime_handle arg;
	int ret;
	void *ptr;

	memset(&arg, 0, sizeof(arg));
	arg.handle = handle;

	ret = drmIoctl(fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &arg);
	if (ret) {
		fprintf(stderr, "DRM_IOCTL_PRIME_HANDLE_TO_FD failed\n");
		return NULL;
	}

	ptr = mmap(0, size, PROT_READ, MAP_SHARED, arg.fd, 0);
	if (ptr == MAP_FAILED) {
		fprintf(stderr, "mmap failed %s\n", strerror(errno));
		return NULL;
	}

	return ptr;
}

static int get_fb_dmafd(int fd, int handle)
{
	struct drm_prime_handle args;
	int ret;

	memset(&args, 0, sizeof(args));
	args.fd = -1;
	args.handle = handle;

	ret = drmIoctl(fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &args);
	if (ret)
		return ret;

	return args.fd;
}

static void destroy_dumb(int fd, int handle) {
	struct drm_mode_destroy_dumb arg;

	memset(&arg, 0, sizeof(arg));
	arg.handle = handle;
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
}

static int dump_fb(int fd, int fb_id) {
	drmModeFBPtr fb = drmModeGetFB(fd, fb_id);
	size_t size;
	void *ptr;
	int dmafd;

	if (!fb) {
		fprintf(stderr, "Failed to get framebuffer\n");
		return -1;
	}

	size = fb->pitch * fb->height;

	dmafd = get_fb_dmafd(fd, fb->handle);

	fprintf(stderr, "fb handle=%d size=%dx%d(%d)\n",
		fb->handle, fb->width, fb->height, fb->pitch);

	ptr = map_fb(fd, fb->handle, size);
	if (ptr) {
		fprintf(stderr, "first pixel: %x\n", *((int *)ptr));
		write(1, ptr, size);
		munmap(ptr, size);
		destroy_dumb(fd, fb->handle);
	}

	close(dmafd);
	drmModeFreeFB(fb);
	return 0;
}

static int get_crtc_fb(int fd, int crtc_id) {
	drmModeCrtcPtr crtc;
	int id;

	crtc = drmModeGetCrtc(fd, crtc_id);
	if (!crtc) {
		fprintf(stderr, "Unknown CRTC %d\n", crtc_id);
		return -1;
	}

	id = crtc->buffer_id;

	drmModeFreeCrtc(crtc);
	return id;
}

static int get_encoder_fb(int fd, int encoder_id) {
	drmModeEncoderPtr encoder;
	int id;

	encoder = drmModeGetEncoder(fd, encoder_id);
	if (!encoder) {
		fprintf(stderr, "Unknown Encoder %d\n", encoder_id);
		return -1;
	}

	id = get_crtc_fb(fd, encoder->crtc_id);

	drmModeFreeEncoder(encoder);
	return id;
}

static int get_connector_fb(int fd, int connector_id) {
	drmModeConnector *connector;
	int id;

	connector = drmModeGetConnector(fd, connector_id);
	if (!connector) {
		fprintf(stderr, "Unknown Connector %d\n", connector_id);
		return -1;
	}

	id = get_encoder_fb(fd, connector->encoder_id);

	drmModeFreeConnector(connector);
	return id;
}

static int get_plane_fb(int fd, int plane_id) {
	drmModePlanePtr plane;
	int id;

	plane = drmModeGetPlane(fd, plane_id);
	if (!plane) {
		fprintf(stderr, "Unknown Plane %d\n", plane_id);
		return -1;
	}

	id = plane->fb_id;

	drmModeFreePlane(plane);
	return id;
}

static void usage(const char *prog, int fd) {
	drmModeCrtcPtr crtc;
	drmModeResPtr res;
	int i;

	res = drmModeGetResources(fd);

	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "%s [--crtc|--connector|--plane|--fb] OBJ_ID > out\n",
		prog);
	fprintf(stderr, "Valid crtc:");
	for (i = 0; i < res->count_crtcs; i++) {
		crtc = drmModeGetCrtc(fd, res->crtcs[i]);
		if (crtc && crtc->mode_valid)
			fprintf(stderr, " %d", crtc->crtc_id);
		drmModeFreeCrtc(crtc);
	}
	fprintf(stderr, "\n");

	drmModeFreeResources(res);

	exit(-1);
}

int main(int argc, const char** argv) {
	int id, fd;

	fd = open("/dev/dri/card0", O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "drm open failed\n");
		return -1;
	}
	fcntl(fd, F_SETFD, FD_CLOEXEC);

	// while (1) {

	switch (argc) {
	case 3:
		id = atoi(argv[2]);

		if (!strcmp(argv[1], "--crtc"))
			id = get_crtc_fb(fd, id);
		else if (!strcmp(argv[1], "--connector"))
			id = get_connector_fb(fd, id);
		else if (!strcmp(argv[1], "--plane"))
			id = get_plane_fb(fd, id);
		else if (strcmp(argv[1], "--fb"))
			usage(argv[0], fd);

		break;
	case 2:
		id = atoi(argv[1]);
		if (id <= 0)
			usage(argv[0], fd);

		break;
	default:
		usage(argv[0], fd);
	}

	if (id <= 0) {
		fprintf(stderr, "Failed to get framebuffer id\n");
		return -1;
	}

	dump_fb(fd, id);

	// /* Add delay */ }

	return 0;
}
