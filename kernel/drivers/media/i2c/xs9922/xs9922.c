// SPDX-License-Identifier: GPL-2.0
/*
 * xs9922 driver
 *
 * Copyright (C) 2021 Rockchip Electronics Co., Ltd.
 *
 * V0.0X01.0X00 first version.
 */

#define DEBUG
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/rk-camera-module.h>
#include <media/media-entity.h>
#include <media/v4l2-async.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-subdev.h>
#include <linux/pinctrl/consumer.h>
#include <linux/rk-preisp.h>
#include <linux/sched.h>
#include <linux/kthread.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/tlv.h>

#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <drm/drm_modes.h>

#include "ni_type.h"
#include "xs9922_reg_cfg.h"

#define RP_AHD_1080P

#define __CLOSE_SENSOR__ 0

#define DRIVER_VERSION				KERNEL_VERSION(0, 0x01, 0x0)
#define XS9922_TEST_PATTERN 		0
#define XS9922_XVCLK_FREQ			27000000
#define XS9922_LINK_FREQ_1500M		(1500000000UL >> 1) //1.5G
#define XS9922_LINK_FREQ_1200M		(1200000000UL >> 1) //1.2G
#define MIPI_FREQ_297M              297000000   //rpdzkj test mipi rate
#define XS9922_LANES				4
#define XS99222_BITS_PER_SAMPLE		8
#define XS9922_NAME					"xs9922"
#define OF_CAMERA_PINCTRL_STATE_DEFAULT		"rockchip,camera_default"
#define OF_CAMERA_PINCTRL_STATE_SLEEP		"rockchip,camera_sleep"

#define REG_NULL			0xFFFF
#define REG_DELAY			0xFFFE
#define XS9922_REG_VALUE_08BIT		1
#define XS9922_REG_VALUE_16BIT		2
#define XS9922_REG_VALUE_24BIT		3

#define XS9922_MIPI_DEV_MAX_NUM     (1)

#define XS9922_SET_FMT	\
	_IOR('V', BASE_VIDIOC_PRIVATE + 32, struct v4l2_subdev_format )
/*
static enum xs9922_max_pad {
	PAD0,
	PAD1,
	PAD2,
	PAD3,
	PAD_MAX,
};
*/
enum{
	CH_1=0,
	CH_2=1,
	CH_3=2,
	CH_4=3,
	CH_ALL=4,
	MIPI_PAGE=8,
};

struct xs9922_mode {
	u32 bus_fmt;
	u32 width;
	u32 height;
	struct v4l2_fract max_fps;
	u32 mipi_freq_idx;
	u32 bpp;
	const struct regval *global_reg_list;
	const struct regval *reg_list;
	u32 hdr_mode;
	u32 lanes;
	u32 vc[PAD_MAX];
	u32 channel_reso[PAD_MAX];
};

struct xs9922 {
	struct i2c_client	*client;
	struct clk		*xvclk;
	struct gpio_desc	*reset_gpio;
	struct gpio_desc	*power_gpio;
	struct gpio_desc	*cam_gpio;

	struct pinctrl		*pinctrl;
	struct pinctrl_state	*pins_default;
	struct pinctrl_state	*pins_sleep;

	struct v4l2_subdev	subdev;
	struct media_pad	pad;
	struct v4l2_ctrl_handler ctrl_handler;
	struct v4l2_ctrl	*pixel_rate;
	struct v4l2_ctrl	*link_freq;
	struct mutex		mutex;
	bool			power_on;
	const struct xs9922_mode *cur_mode;

	u32			module_index;
	u32			cfg_num;
	const char		*module_facing;
	const char		*module_name;
	const char		*len_name;
	bool			lost_video_status;

	int streaming;
	struct task_struct *detect_thread;	//for hotplug detect work
	struct input_dev* input_dev;
	unsigned char detect_status;
	unsigned char last_detect_status;
	u8 is_reset;
};

/////////////////////////////////////////////////////////
#define NI_ID0                 (0x22)
#define NI_ID1                 (0x99)
#define NI_40F0_DEVICE_ID_1                               (0x40F0)
#define NI_40F1_DEVICE_ID_0                               (0x40F1)

#define NI_4345_CDT_STATUS_CH0                            (0x4345)
#define NI_4346_CDT_STATUS_CH1                            (0x4346)
#define NI_4347_CDT_STATUS_CH2                            (0x4347)
#define NI_4348_CDT_STATUS_CH3                            (0x4348)

#define NI_VIDEO_STATUS_CH0                            (0x0000)
#define NI_VIDEO_STATUS_CH1                            (0x1000)
#define NI_VIDEO_STATUS_CH2                            (0x2000)
#define NI_VIDEO_STATUS_CH3                            (0x3000)

#define to_xs9922(sd) container_of(sd, struct xs9922, subdev)

static const s64 link_freq_items[] = {
	XS9922_LINK_FREQ_1500M,
	XS9922_LINK_FREQ_1200M,
    MIPI_FREQ_297M,
};

static int xs9922_read_reg(struct i2c_client *client, u16 reg, unsigned int len, u32 *val);

static void __maybe_unused dumpChxReg(struct i2c_client *client)
{
    u16 reg=0;
	u32 val = 0;
    int i=0,j=0;

    for (i=0; i<4; i++) {
        // read status
        for (j=0; j<=0x29;j++) {
            reg = (i<<12)|j;
            xs9922_read_reg(client, reg, XS9922_REG_VALUE_08BIT, &val);
            dev_err(&client->dev, "{0x%04x, 0x%04x}\n", reg, val);
        }

        // hd regs
        for (j=0x100; j<=0x1e2;j++) {
            reg = (i<<12)|j;
            xs9922_read_reg(client, reg, XS9922_REG_VALUE_08BIT, &val);
            dev_err(&client->dev, "{0x%04x, 0x%04x}\n", reg, val);
        }
    }
}

// detect_status: bit 0~3 means channels plugin status : 0, no pluged in; 1, pluged in
static ssize_t show_hotplug_status(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct xs9922 *xs9922 = to_xs9922(sd);
	return sprintf(buf, "%d\n", xs9922->detect_status);
}

static ssize_t xs9922_campower(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    int enable;
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
    struct xs9922 *xs9922 = to_xs9922(sd);

    if (!IS_ERR(xs9922->cam_gpio)) {
        sscanf(buf, "%d", &enable);
        if (2 == enable) {
            dumpChxReg(client);
        } else
        gpiod_set_value_cansleep(xs9922->cam_gpio, !!enable);
    }

    return count;
}

static DEVICE_ATTR(hotplug_status, S_IRUSR|S_IWUSR, show_hotplug_status, xs9922_campower);
static struct attribute *dev_attrs[] = {
	&dev_attr_hotplug_status.attr,
	NULL,
};
static struct attribute_group dev_attr_grp = {
	.attrs = dev_attrs,
};

static const struct xs9922_mode supported_modes[] = {
#ifndef RP_AHD_1080P	
	{
		.bus_fmt = MEDIA_BUS_FMT_UYVY8_2X8,//MEDIA_BUS_FMT_UYVY8_2X8,
		.width = 1280,
		.height = 720,
		.max_fps = {
			.numerator = 10000,
			.denominator = 250000,
		},
		.global_reg_list = xs9922_init_cfg,
		.reg_list = xs9922_720p_4lanes_25fps,
		.mipi_freq_idx = 0,
		.bpp = 8,
		.hdr_mode = NO_HDR,
		.lanes = 4,	// 1,
		//.lanes = 1,
		.vc[PAD0] = V4L2_MBUS_CSI2_CHANNEL_0,
		.vc[PAD1] = V4L2_MBUS_CSI2_CHANNEL_1,
		.vc[PAD2] = V4L2_MBUS_CSI2_CHANNEL_2,
		.vc[PAD3] = V4L2_MBUS_CSI2_CHANNEL_3,
	},
#endif
	{
		.bus_fmt = MEDIA_BUS_FMT_UYVY8_2X8,//MEDIA_BUS_FMT_UYVY8_2X8,
		.width = 1920,
		.height = 1080,
		.max_fps = {
			.numerator = 10000,
			.denominator = 250000,
		},
		.global_reg_list = xs9922_init_cfg,
		.reg_list = xs9922_1080p_4lanes_25fps,
		.mipi_freq_idx = 0,
		.bpp = 8,
		.hdr_mode = NO_HDR,
		.lanes = 4,
		.vc[PAD0] = V4L2_MBUS_CSI2_CHANNEL_0,
		.vc[PAD1] = V4L2_MBUS_CSI2_CHANNEL_1,
		.vc[PAD2] = V4L2_MBUS_CSI2_CHANNEL_2,
		.vc[PAD3] = V4L2_MBUS_CSI2_CHANNEL_3,
	}
};


#define config_file "/etc/board.conf"
char board_config_buf[1024] = {0};
struct drm_display_mode *hdmiCusMode=NULL;

static int xs9922_write_reg(struct i2c_client *client, u16 reg,
			    u32 len, u32 val)
{
	u32 buf_i, val_i;
	u8 buf[6];
	u8 *val_p;
	__be32 val_be;

	if (len > 4)
		return -EINVAL;

	buf[0] = reg >> 8;
	buf[1] = reg & 0xff;

	val_be = cpu_to_be32(val);
	val_p = (u8 *)&val_be;
	buf_i = 2;
	val_i = 4 - len;

	while (val_i < 4)
		buf[buf_i++] = val_p[val_i++];

	if (i2c_master_send(client, buf, len + 2) != len + 2) {
		dev_err(&client->dev, "xs9922 write reg(0x%x) failed !\n", reg);
		return -EIO;
	}

	return 0;
}

/* Read registers up to 4 at a time */
static int xs9922_read_reg(struct i2c_client *client,
			    u16 reg,
			    unsigned int len,
			    u32 *val)
{
	struct i2c_msg msgs[2];
	u8 *data_be_p;
	__be32 data_be = 0;
	__be16 reg_addr_be = cpu_to_be16(reg);
	int ret;

	//printk("%p ---- 0x%04x %d %d\n", client, reg, len, *val);

	if (len > 4 || !len)
		return -EINVAL;

	data_be_p = (u8 *)&data_be;
	/* Write register address */
	//dev_dbg(&client->dev, "xs9922 i2c addr (0x%x) !\n", client->addr);
	msgs[0].addr = client->addr;
	msgs[0].flags = 0;
	msgs[0].len = 2;
	msgs[0].buf = (u8 *)&reg_addr_be;

	/* Read data from register */
	msgs[1].addr = client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = len;
	msgs[1].buf = &data_be_p[4 - len];

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs))
	{
		dev_err(&client->dev, "xs9922 read reg(0x%x) failed !\n", reg);
		return -EIO;
	}

	//printk("---- 0x%08x\n", data_be);

	*val = be32_to_cpu(data_be);
	return 0;
}

static int xs9922_write_array(struct i2c_client *client,
			       const struct regval *regs)
{
	u32 i;
	int ret = 0;
	// u32 val = 0;

	for (i = 0; ret == 0 && regs[i].addr != REG_NULL; i++) {
		ret |= xs9922_write_reg(client, regs[i].addr,
			XS9922_REG_VALUE_08BIT, regs[i].val);

        if (0 != regs[i].nDelay) msleep(regs[i].nDelay);
		// dev_dbg(&client->dev, "xs9922 write reg(0x%04x) : 0x%02x!\n", regs[i].addr, regs[i].val);
		// udelay(10 * 1000);
		// xs9922_read_reg(client, regs[i].addr, XS9922_REG_VALUE_08BIT, &val);
		// if ((u8)val != regs[i].val)
		// 	dev_err(&client->dev, "xs9922 write reg(0x%04x) : happen err!\n", regs[i].addr);
	}
	return ret;
}

static int __maybe_unused xs9922_read_array(struct i2c_client *client,
			       const struct regval *regs)
{
	u32 i;
	int ret = 0;
	u32 val = 0;

	for (i = 0; ret == 0 && regs[i].addr != REG_NULL; i++) {
		ret |= xs9922_read_reg(client, regs[i].addr,
			XS9922_REG_VALUE_08BIT, &val);

		dev_dbg(&client->dev, "xs9922 read reg(0x%04x) : 0x%02x!\n", regs[i].addr, (u8)val);
		udelay(10 * 1000);
		// xs9922_read_reg(client, regs[i].addr, XS9922_REG_VALUE_08BIT, &val);
		if ((u8)val != regs[i].val)
			dev_err(&client->dev, "xs9922 write reg(0x%04x) : happen err!\n", regs[i].addr);
	}
	return ret;
}

void switch_mode(struct xs9922 *xs9922)
{
    int ret = 0;
    struct i2c_client *client = xs9922->client;

    dev_dbg(&client->dev, "%s IN --->>>\n", __func__);

    ret = xs9922_write_array(client, xs9922->cur_mode->reg_list);
    if (ret)
    {
        dev_dbg(&client->dev, "%s write xs9922 register array error!\n", __func__);
    }
}

static int __maybe_unused detect_thread_start(struct xs9922 *xs9922);

static int read_config(void *data)
{
    struct xs9922 *xs9922 = (struct xs9922 *) data;
    struct file *fp = NULL;
    mm_segment_t fs;
    loff_t pos = 0;
    int tries=10;

    printk("__xs9922_init in\n");
    if (!IS_ERR(xs9922->cam_gpio)) {
        gpiod_set_value_cansleep(xs9922->cam_gpio, 1);
    }
    xs9922_write_array(xs9922->client, xs9922->cur_mode->global_reg_list);

    do {
        fp = filp_open(config_file,O_RDONLY,0);
        if(!IS_ERR(fp)){
            break;
        }
        printk(KERN_ERR "open "config_file" fail!!!\n");
        tries--;
        msleep(100);
    } while(tries > 0);

    if(IS_ERR(fp)){
        printk(KERN_ERR "open "config_file" fail!!!\n");
        //return -1;
    } else {
        fs = get_fs();
        set_fs(KERNEL_DS);
        pos = 0;
        vfs_read(fp,board_config_buf,sizeof(board_config_buf),&pos);
        filp_close(fp,NULL);
        set_fs(fs);

        //printk(KERN_ERR "[hardy] config:\n%s\n", board_config_buf);
        if (strlen(board_config_buf) > 0) {
            char *mode = strstr(board_config_buf, "camode=");
            char *timing = strstr(board_config_buf, "outiming=");
            int width=0, height=0, vrefresh=0;
            int i=0;

            if (NULL != mode) {
                sscanf(mode, "camode=%dx%d-%d", &width, &height, &vrefresh);

                for (i = 0; i < xs9922->cfg_num; i++) {
                    int fps=supported_modes[i].max_fps.denominator/supported_modes[i].max_fps.numerator;

                    if (width==supported_modes[i].width
                        && height==supported_modes[i].height
                        && vrefresh==fps) {
                        xs9922->cur_mode = (struct xs9922_mode *)&supported_modes[i];
                        break;
                    }
                }
            }

            if (NULL != timing) {
                static struct drm_display_mode mode;
                memset(&mode, 0, sizeof(struct drm_display_mode));
                sscanf(timing, "outiming=%s %hd %hd %hd %hd %hd %hd %hd %hd %d %d",
                    mode.name, //&mode.vrefresh,
                    &mode.hdisplay, &mode.hsync_start, &mode.hsync_end, &mode.htotal,
                    &mode.vdisplay, &mode.vsync_start, &mode.vsync_end, &mode.vtotal,
                    &mode.clock, &mode.flags);
                if (strlen(mode.name) > 0 ) hdmiCusMode = &mode;
            }
        }
    }

    switch_mode(xs9922);
	detect_thread_start(xs9922);

    printk("__xs9922_init out\n");

    return 0;
}

static int xs9922_get_reso_dist(const struct xs9922_mode *mode,
				struct v4l2_mbus_framefmt *framefmt)
{
	return abs(mode->width - framefmt->width) +
	       abs(mode->height - framefmt->height);
}

static const struct xs9922_mode *
xs9922_find_best_fit(struct xs9922 *xs9922,
                      struct v4l2_subdev_format *fmt)
{
	struct v4l2_mbus_framefmt *framefmt = &fmt->format;
	int dist;
	int cur_best_fit = 0;
	int cur_best_fit_dist = -1;
	unsigned int i;

	for (i = 0; i < xs9922->cfg_num; i++) {
		dist = xs9922_get_reso_dist(&supported_modes[i], framefmt);
		if ((cur_best_fit_dist == -1 || dist <= cur_best_fit_dist) &&
			supported_modes[i].bus_fmt == framefmt->code) {
			cur_best_fit_dist = dist;
			cur_best_fit = i;
		}
	}

	return &supported_modes[cur_best_fit];
}


static int xs9922_g_mbus_config(struct v4l2_subdev *sd, unsigned int pad,
				 struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_CSI2_DPHY;
	cfg->flags = V4L2_MBUS_CSI2_4_LANE |
		     V4L2_MBUS_CSI2_CHANNELS;

	return 0;
}


static int xs9922_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)
{
	struct xs9922 *xs9922 = to_xs9922(sd);
	const struct xs9922_mode *mode;
	u64 pixel_rate;

    dev_dbg(&xs9922->client->dev, "%s IN --->>>\n", __func__);

	mutex_lock(&xs9922->mutex);

	mode = xs9922_find_best_fit(xs9922, fmt);
	fmt->format.code = mode->bus_fmt;
	fmt->format.width = mode->width;
	fmt->format.height = mode->height;
	fmt->format.field = V4L2_FIELD_NONE;
	fmt->format.colorspace = mode->bus_fmt;

	if (fmt->which == V4L2_SUBDEV_FORMAT_TRY) {
#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
		*v4l2_subdev_get_try_format(sd, cfg, fmt->pad) = fmt->format;
#else
		mutex_unlock(&xs9922->mutex);
		return -ENOTTY;
#endif
	} else {
        xs9922->cur_mode = mode;
        __v4l2_ctrl_s_ctrl(xs9922->link_freq, mode->mipi_freq_idx);
        pixel_rate = (u32)link_freq_items[mode->mipi_freq_idx] / mode->bpp * 2 * XS9922_LANES;
        __v4l2_ctrl_s_ctrl_int64(xs9922->pixel_rate, pixel_rate);
        dev_dbg(&xs9922->client->dev, "mipi_freq_idx %d\n", mode->mipi_freq_idx);
        dev_dbg(&xs9922->client->dev, "pixel_rate %lld\n", pixel_rate);
#if (!__CLOSE_SENSOR__)
        switch_mode(xs9922);
#endif
	}

	mutex_unlock(&xs9922->mutex);
	return 0;
}

static int xs9922_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)
{
	struct xs9922 *xs9922 = to_xs9922(sd);
	struct i2c_client *client = xs9922->client;
	const struct xs9922_mode *mode = xs9922->cur_mode;

	mutex_lock(&xs9922->mutex);
	if (fmt->which == V4L2_SUBDEV_FORMAT_TRY) {
#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
		fmt->format = *v4l2_subdev_get_try_format(sd, cfg, fmt->pad);
#else
		mutex_unlock(&xs9922->mutex);
		return -ENOTTY;
#endif
	} else {
		fmt->format.width = mode->width;
		fmt->format.height = mode->height;
		fmt->format.code = mode->bus_fmt;
		fmt->format.field = V4L2_FIELD_NONE;
		if (fmt->pad < PAD_MAX && fmt->pad >= PAD0)
			fmt->reserved[0] = mode->vc[fmt->pad];
		else
			fmt->reserved[0] = mode->vc[PAD0];
	}
	mutex_unlock(&xs9922->mutex);

	dev_dbg(&client->dev, "%s: %x %dx%d\n",
		__func__, fmt->format.code,
		fmt->format.width, fmt->format.height);

	return 0;
}


static int xs9922_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_pad_config *cfg,
				 struct v4l2_subdev_mbus_code_enum *code)
{
	struct xs9922 *xs9922 = to_xs9922(sd);

	if (code->index >= xs9922->cfg_num)
		return -EINVAL;
	code->code = supported_modes[code->index].bus_fmt;

	return 0;
}

static int xs9922_enum_frame_sizes(struct v4l2_subdev *sd,
				   struct v4l2_subdev_pad_config *cfg,
				   struct v4l2_subdev_frame_size_enum *fse)
{
	struct xs9922 *xs9922 = to_xs9922(sd);
	struct i2c_client *client = xs9922->client;

	dev_dbg(&client->dev, "%s:\n", __func__);

	if (fse->index >= xs9922->cfg_num)
		return -EINVAL;

	if (fse->code != supported_modes[fse->index].bus_fmt)
		return -EINVAL;

	fse->min_width  = supported_modes[fse->index].width;
	fse->max_width  = supported_modes[fse->index].width;
	fse->max_height = supported_modes[fse->index].height;
	fse->min_height = supported_modes[fse->index].height;
	return 0;
}

static int xs9922_enum_frame_interval(struct v4l2_subdev *sd,
						  struct v4l2_subdev_pad_config *cfg,
						  struct v4l2_subdev_frame_interval_enum *fie)
{
	//struct xs9922 *xs9922 = to_xs9922(sd);

//    printk("[hardy] %s:%d index:%d\n", __func__, __LINE__, fie->index);

//	if (fie->index >= xs9922->cfg_num)
//		return -EINVAL;

//	fie->code = supported_modes[fie->index].bus_fmt;
//	fie->width = supported_modes[fie->index].width;
//	fie->height = supported_modes[fie->index].height;
//	fie->interval = supported_modes[fie->index].max_fps;
	//fie->reserved[0] = supported_modes[fie->index].hdr_mode;
	
	fie->code = supported_modes[0].bus_fmt;
	fie->width = supported_modes[0].width;
	fie->height = supported_modes[0].height;
	fie->interval = supported_modes[0].max_fps;
	//fie->reserved[0] = supported_modes[0].hdr_mode;
	return 0;
}
/*
static int xs9922_g_mbus_config(struct v4l2_subdev *sd,
				 struct v4l2_mbus_config *cfg)
{
	struct xs9922 *xs9922 = to_xs9922(sd);
	struct i2c_client *client = xs9922->client;
	const struct xs9922_mode *mode = xs9922->cur_mode;

	dev_dbg(&client->dev, "%s: mode->lanes = %d\n", __func__, mode->lanes);

	cfg->flags = V4L2_MBUS_CSI2_4_LANE |
			 V4L2_MBUS_CSI2_CONTINUOUS_CLOCK |
		     V4L2_MBUS_CSI2_CHANNELS;

	cfg->type = V4L2_MBUS_CSI2_DPHY;
	return 0;
}
*/

static int xs9922_g_frame_interval(struct v4l2_subdev *sd,
				    struct v4l2_subdev_frame_interval *fi)
{
	struct xs9922 *xs9922 = to_xs9922(sd);
	const struct xs9922_mode *mode = xs9922->cur_mode;

	mutex_lock(&xs9922->mutex);
	fi->interval = mode->max_fps;
	mutex_unlock(&xs9922->mutex);

	return 0;
}


static void xs9922_get_module_inf(struct xs9922 *xs9922,
				   struct rkmodule_inf *inf)
{
	memset(inf, 0, sizeof(*inf));
	strlcpy(inf->base.sensor, XS9922_NAME, sizeof(inf->base.sensor));
	strlcpy(inf->base.module, xs9922->module_name,
		sizeof(inf->base.module));
	strlcpy(inf->base.lens, xs9922->len_name, sizeof(inf->base.lens));
}

static __maybe_unused int xs9922_auto_detect_hotplug(struct xs9922 *xs9922)
{
	int ret = 0;
	u32 val0, val1, val2, val3;
	struct i2c_client *client = xs9922->client;
#if 0
	xs9922_read_reg(client, NI_4345_CDT_STATUS_CH0, XS9922_REG_VALUE_08BIT, &val0);
	xs9922_read_reg(client, NI_4346_CDT_STATUS_CH1, XS9922_REG_VALUE_08BIT, &val1);
	xs9922_read_reg(client, NI_4347_CDT_STATUS_CH2, XS9922_REG_VALUE_08BIT, &val2);
	xs9922_read_reg(client, NI_4348_CDT_STATUS_CH3, XS9922_REG_VALUE_08BIT, &val3);

	xs9922->detect_status = (val0 & 0x01) | (val1 & 0x01) << 1 | (val2 & 0x01) << 2 | (val3 & 0x01) << 3 ;
#else
    xs9922_read_reg(client, NI_VIDEO_STATUS_CH0, XS9922_REG_VALUE_08BIT, &val0);
    xs9922_read_reg(client, NI_VIDEO_STATUS_CH1, XS9922_REG_VALUE_08BIT, &val1);
    xs9922_read_reg(client, NI_VIDEO_STATUS_CH2, XS9922_REG_VALUE_08BIT, &val2);
    xs9922_read_reg(client, NI_VIDEO_STATUS_CH3, XS9922_REG_VALUE_08BIT, &val3);

    xs9922->detect_status = ((~(val0>>4)) & 0x01) | ((~(val1>>4)) & 0x01) << 1
        | ((~(val2>>4)) & 0x01) << 2 | ((~(val3>>4)) & 0x01) << 3 ;
#endif
    //dev_dbg(&xs9922->client->dev, "%s: auto detect: 0x%x\n", __func__, xs9922->detect_status);
	return ret;
}

static void xs9922_get_vc_hotplug_inf(struct xs9922 *xs9922,
				       struct rkmodule_vc_hotplug_info *inf)
{
	memset(inf, 0, sizeof(*inf));
//	xs9922_auto_detect_hotplug(xs9922);
	inf->detect_status = xs9922->detect_status;
}

static void xs9922_get_vicap_rst_inf(struct xs9922 *xs9922,
				   struct rkmodule_vicap_reset_info *rst_info)
{
	rst_info->is_reset = xs9922->is_reset;
	rst_info->src = RKCIF_RESET_SRC_ERR_HOTPLUG;
}

static void xs9922_set_vicap_rst_inf(struct xs9922 *xs9922,
				   struct rkmodule_vicap_reset_info rst_info)
{
	xs9922->is_reset = rst_info.is_reset;
}

static __maybe_unused int xs9922_mipi_reset_proc(struct xs9922 *xs9922)
{
	int ret = 0;
	ret = xs9922_write_reg(xs9922->client, 0x5004, XS9922_REG_VALUE_08BIT, 0x00);
	ret |= xs9922_write_reg(xs9922->client, 0x5005, XS9922_REG_VALUE_08BIT, 0x00);
	ret |= xs9922_write_reg(xs9922->client, 0x5006, XS9922_REG_VALUE_08BIT, 0x00);
	ret |= xs9922_write_reg(xs9922->client, 0x5007, XS9922_REG_VALUE_08BIT, 0x00);
	//usleep_range(50*1000, 100*1000);

	ret = xs9922_write_reg(xs9922->client, 0x5004, XS9922_REG_VALUE_08BIT, 0x00);
	ret |= xs9922_write_reg(xs9922->client, 0x5005, XS9922_REG_VALUE_08BIT, 0x00);
	ret |= xs9922_write_reg(xs9922->client, 0x5006, XS9922_REG_VALUE_08BIT, 0x00);
	ret |= xs9922_write_reg(xs9922->client, 0x5007, XS9922_REG_VALUE_08BIT, 0x01);

	return ret;
}

static long xs9922_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct xs9922 *xs9922 = to_xs9922(sd);
	long ret = 0;
	u32 stream = 0;
    //dev_dbg(&xs9922->client->dev, "%s IN --->>> cmd:0x%x\n", __func__, cmd);

	switch (cmd) {
	case RKMODULE_GET_MODULE_INFO:
		xs9922_get_module_inf(xs9922, (struct rkmodule_inf *)arg);
		break;

	case RKMODULE_GET_VC_HOTPLUG_INFO:
		xs9922_get_vc_hotplug_inf(xs9922, (struct rkmodule_vc_hotplug_info *)arg);
		dev_dbg(&xs9922->client->dev, "[chad]Get VC Hotplug info --->>> detect status: %d\n",xs9922->detect_status);
		break;

	case RKMODULE_GET_VICAP_RST_INFO:
		xs9922_get_vicap_rst_inf(xs9922, (struct rkmodule_vicap_reset_info *)arg);
		//dev_err(&xs9922->client->dev, "[chad] Get vicap reset info --->>> is_reset : %d\n", xs9922->is_reset);
		break;

	case RKMODULE_SET_VICAP_RST_INFO:
		xs9922_set_vicap_rst_inf(xs9922, *(struct rkmodule_vicap_reset_info *)arg);
		//dev_err(&xs9922->client->dev, "[chad] Set vicap reset info --->>> is_reset : %d\n", xs9922->is_reset);
		break;

	case RKMODULE_GET_START_STREAM_SEQ:
		*(int *)arg = RKMODULE_START_STREAM_FRONT;
		break;

    case XS9922_SET_FMT:
        xs9922->cur_mode = xs9922_find_best_fit(xs9922, (struct v4l2_subdev_format *) arg);
#if (!__CLOSE_SENSOR__)
        switch_mode(xs9922);
#endif
		break;
#if 1
	case RKMODULE_SET_QUICK_STREAM:

		stream = *((u32 *)arg);

		if (stream) {
			dev_info(&xs9922->client->dev, "[chad]======== quick stream on: do xs9922 mipi reset start =======\n");

			// xs9922_write_reg(xs9922->client, 0x50e0, XS9922_REG_VALUE_08BIT, 0x00);
			// xs9922_write_reg(xs9922->client, 0x50e1, XS9922_REG_VALUE_08BIT, 0x00);
			// xs9922_write_reg(xs9922->client, 0x50e2, XS9922_REG_VALUE_08BIT, 0x00);
			// xs9922_write_reg(xs9922->client, 0x50e3, XS9922_REG_VALUE_08BIT, 0x07);

			ret = xs9922_write_reg(xs9922->client, 0x5004, XS9922_REG_VALUE_08BIT, 0x00);
			ret |= xs9922_write_reg(xs9922->client, 0x5005, XS9922_REG_VALUE_08BIT, 0x00);
			ret |= xs9922_write_reg(xs9922->client, 0x5006, XS9922_REG_VALUE_08BIT, 0x00);
			ret |= xs9922_write_reg(xs9922->client, 0x5007, XS9922_REG_VALUE_08BIT, 0x01);

			// usleep_range(20*1000, 40*1000);
			ret = xs9922_write_reg(xs9922->client, 0x0e08, XS9922_REG_VALUE_08BIT, 0x01);
			ret |= xs9922_write_reg(xs9922->client, 0x1e08, XS9922_REG_VALUE_08BIT, 0x01);
			ret |= xs9922_write_reg(xs9922->client, 0x2e08, XS9922_REG_VALUE_08BIT, 0x01);
			ret |= xs9922_write_reg(xs9922->client, 0x3e08, XS9922_REG_VALUE_08BIT, 0x01);

			dev_info(&xs9922->client->dev, "[chad]======== quick stream on: do xs9922 mipi reset end =======\n");
		}
		else {
			xs9922_write_reg(xs9922->client, 0x0e08, XS9922_REG_VALUE_08BIT, 0x00);
			xs9922_write_reg(xs9922->client, 0x1e08, XS9922_REG_VALUE_08BIT, 0x00);
			xs9922_write_reg(xs9922->client, 0x2e08, XS9922_REG_VALUE_08BIT, 0x00);
			xs9922_write_reg(xs9922->client, 0x3e08, XS9922_REG_VALUE_08BIT, 0x00);

			// xs9922_write_reg(xs9922->client, 0x50e0, XS9922_REG_VALUE_08BIT, 0x00);
			// xs9922_write_reg(xs9922->client, 0x50e1, XS9922_REG_VALUE_08BIT, 0x00);
			// xs9922_write_reg(xs9922->client, 0x50e2, XS9922_REG_VALUE_08BIT, 0x00);
			// xs9922_write_reg(xs9922->client, 0x50e3, XS9922_REG_VALUE_08BIT, 0x05);

			// xs9922_write_reg(xs9922->client, 0x50e0, XS9922_REG_VALUE_08BIT, 0x00);
			// xs9922_write_reg(xs9922->client, 0x50e1, XS9922_REG_VALUE_08BIT, 0x00);
			// xs9922_write_reg(xs9922->client, 0x50e2, XS9922_REG_VALUE_08BIT, 0x00);
			// xs9922_write_reg(xs9922->client, 0x50e3, XS9922_REG_VALUE_08BIT, 0x07);

			xs9922_write_reg(xs9922->client, 0x5004, XS9922_REG_VALUE_08BIT, 0x00);
			xs9922_write_reg(xs9922->client, 0x5005, XS9922_REG_VALUE_08BIT, 0x00);
			xs9922_write_reg(xs9922->client, 0x5006, XS9922_REG_VALUE_08BIT, 0x00);
			xs9922_write_reg(xs9922->client, 0x5007, XS9922_REG_VALUE_08BIT, 0x00);
			dev_info(&xs9922->client->dev, "[chad]======== quick stream off:xs9922 mipi Disabled =======\n");
		}
		break;
#endif

	default:
		ret = -ENOTTY;
		break;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long xs9922_compat_ioctl32(struct v4l2_subdev *sd,
				   unsigned int cmd, unsigned long arg)
{
	void __user *up = compat_ptr(arg);
	struct rkmodule_inf *inf;
	struct rkmodule_awb_cfg *cfg;
	struct rkmodule_vc_hotplug_info *vc_hp_inf;
	struct rkmodule_vicap_reset_info *vicap_rst_inf;
	int *seq;
	long ret = 0;

	switch (cmd) {
	case RKMODULE_GET_MODULE_INFO:
		inf = kzalloc(sizeof(*inf), GFP_KERNEL);
		if (!inf) {
			ret = -ENOMEM;
			return ret;
		}

		ret = xs9922_ioctl(sd, cmd, inf);
		if (!ret)
			ret = copy_to_user(up, inf, sizeof(*inf));
		kfree(inf);
		break;
	case RKMODULE_AWB_CFG:
		cfg = kzalloc(sizeof(*cfg), GFP_KERNEL);
		if (!cfg) {
			ret = -ENOMEM;
			return ret;
		}

		ret = copy_from_user(cfg, up, sizeof(*cfg));
		if (!ret)
			ret = xs9922_ioctl(sd, cmd, cfg);
		kfree(cfg);
		break;

	case RKMODULE_GET_VC_HOTPLUG_INFO:
		vc_hp_inf = kzalloc(sizeof(*vc_hp_inf), GFP_KERNEL);
		if (!vc_hp_inf) {
			ret = -ENOMEM;
			return ret;
		}

		ret = xs9922_ioctl(sd, cmd, vc_hp_inf);
		if (!ret)
			ret = copy_to_user(up, vc_hp_inf, sizeof(*vc_hp_inf));
		kfree(vc_hp_inf);
		break;

	case RKMODULE_GET_VICAP_RST_INFO:
		vicap_rst_inf = kzalloc(sizeof(*vicap_rst_inf), GFP_KERNEL);
		if (!vicap_rst_inf) {
			ret = -ENOMEM;
			return ret;
		}

		ret = xs9922_ioctl(sd, cmd, vicap_rst_inf);
		if (!ret)
			ret = copy_to_user(up, vicap_rst_inf, sizeof(*vicap_rst_inf));
		kfree(vicap_rst_inf);
		break;

	case RKMODULE_SET_VICAP_RST_INFO:
		vicap_rst_inf = kzalloc(sizeof(*vicap_rst_inf), GFP_KERNEL);
		if (!vicap_rst_inf) {
			ret = -ENOMEM;
			return ret;
		}

		ret = copy_from_user(vicap_rst_inf, up, sizeof(*vicap_rst_inf));
		if (!ret)
			ret = xs9922_ioctl(sd, cmd, vicap_rst_inf);
		kfree(vicap_rst_inf);
		break;

	case RKMODULE_GET_START_STREAM_SEQ:
		seq = kzalloc(sizeof(*seq), GFP_KERNEL);
		if (!seq) {
			ret = -ENOMEM;
			return ret;
		}

		ret = xs9922_ioctl(sd, cmd, seq);
		if (!ret)
			ret = copy_to_user(up, seq, sizeof(*seq));
		kfree(seq);
		break;

	default:
		ret = -ENOIOCTLCMD;
		break;
	}

	return ret;
}
#endif

static int detect_thread_function(void *data)
{
    struct xs9922 *xs9922 = (struct xs9922 *) data;
    struct i2c_client *client = xs9922->client;

    unsigned char bits = 0, ch;
    int need_reset_wait = -1;

    if (xs9922->power_on) {
        xs9922_auto_detect_hotplug(xs9922);
        xs9922->last_detect_status = xs9922->detect_status;
        xs9922->is_reset = 0;
    }

    while (!kthread_should_stop()) {
        if (xs9922->power_on) {
            xs9922_auto_detect_hotplug(xs9922);
            if (xs9922->last_detect_status != xs9922->detect_status) {
                if (xs9922->last_detect_status < xs9922->detect_status) {
                    bits = xs9922->last_detect_status ^ xs9922->detect_status;
                    for (ch = 0; ch < 4; ch++) {
                        if (bits & (1 << ch)) {
                            dev_err(&client->dev, "[chad]================ xs9922 detect ch %d plug in!!! ===============\n", ch);
                        }
                    }
                } else {//means something channel plug out detect
                    bits = xs9922->last_detect_status ^ xs9922->detect_status;
                    for (ch = 0; ch < 4; ch++) {
                        if (bits & (1 << ch)) {
                            dev_err(&client->dev, "[chad]================ xs9922 detect ch %d plug Out!!! ===============\n", ch);
                        }
                    }
                }

                if (need_reset_wait < 0)
                    need_reset_wait = 2; //wait for 1 second to set reset status.

                if (need_reset_wait > 0) {
                    need_reset_wait--;
                }

                if (need_reset_wait == 0) { //wait 1s to set is_reset to vicap.
                    need_reset_wait = -1;
                    xs9922->is_reset = 1;

                    xs9922->last_detect_status = xs9922->detect_status;
                    input_event(xs9922->input_dev, EV_MSC, MSC_RAW, xs9922->detect_status);
                    input_sync(xs9922->input_dev);

                    dev_err(&client->dev, "[chad] =============trigger reset time up==============\n");
                }
            }
        }
        set_current_state(TASK_INTERRUPTIBLE);
        if (0x0f == xs9922->detect_status) {
            schedule_timeout(msecs_to_jiffies(100));
        } else {
            schedule_timeout(msecs_to_jiffies(1000));
        }
    }
    return 0;
}

int __maybe_unused detect_thread_start(struct xs9922 *xs9922)
{
	int ret = 0;
	struct i2c_client *client = xs9922->client;
	xs9922->detect_thread = kthread_create(detect_thread_function,
                                   xs9922, "xs9922_kthread");
	if (IS_ERR(xs9922->detect_thread)) {
		dev_err(&client->dev, "kthread_create xs9922_kthread failed\n");
		ret = PTR_ERR(xs9922->detect_thread);
		xs9922->detect_thread = NULL;
		return ret;
	}
	wake_up_process(xs9922->detect_thread);
	return ret;
}

static int __maybe_unused detect_thread_stop(struct xs9922 *xs9922)
{
	if (xs9922->detect_thread)
		kthread_stop(xs9922->detect_thread);
	xs9922->detect_thread = NULL;
	return 0;
}

////////////////////////////////////////////////////////////////
static int __xs9922_start_stream(struct xs9922 *xs9922)
{
	struct i2c_client *client = xs9922->client;
#if __CLOSE_SENSOR__
    switch_mode(xs9922);
	detect_thread_start(xs9922);
#else
    xs9922_write_array(client, xs9922_mipi_reset_new);
#endif

	xs9922_write_reg(client, 0x0e08, XS9922_REG_VALUE_08BIT, 0x01);
	xs9922_write_reg(client, 0x1e08, XS9922_REG_VALUE_08BIT, 0x01);
	xs9922_write_reg(client, 0x2e08, XS9922_REG_VALUE_08BIT, 0x01);
	xs9922_write_reg(client, 0x3e08, XS9922_REG_VALUE_08BIT, 0x01);
	usleep_range(200*1000, 400*1000);

	dev_dbg(&client->dev, "%s OUT---<<<\n", __func__);

	return 0;
}

static int __xs9922_stop_stream(struct xs9922 *xs9922)
{
	struct i2c_client *client = xs9922->client;

	dev_dbg(&client->dev, "%s In---<<<\n", __func__);
	// detect_thread_stop(xs9922);
	xs9922_write_reg(client, 0x0e08, XS9922_REG_VALUE_08BIT, 0x00);
	xs9922_write_reg(client, 0x1e08, XS9922_REG_VALUE_08BIT, 0x00);
	xs9922_write_reg(client, 0x2e08, XS9922_REG_VALUE_08BIT, 0x00);
	xs9922_write_reg(client, 0x3e08, XS9922_REG_VALUE_08BIT, 0x00);

#if __CLOSE_SENSOR__
	detect_thread_stop(xs9922);
#endif
	dev_dbg(&client->dev, "%s OUT---<<<\n", __func__);

	return 0;
}

static int xs9922_stream(struct v4l2_subdev *sd, int on)
{
	struct xs9922 *xs9922 = to_xs9922(sd);
	struct i2c_client *client = xs9922->client;

	dev_dbg(&client->dev, "%s: s_stream: %d. %dx%d\n", __func__, on,
			xs9922->cur_mode->width,
			xs9922->cur_mode->height);

	mutex_lock(&xs9922->mutex);
	on = !!on;
	if (xs9922->streaming == on)
		goto unlock;

	if (on) {
		__xs9922_start_stream(xs9922);
	} else {
		__xs9922_stop_stream(xs9922);
	}

	xs9922->streaming = on;

unlock:
	mutex_unlock(&xs9922->mutex);

	return 0;
}

static int xs9922_power(struct v4l2_subdev *sd, int on)
{
	struct xs9922 *xs9922 = to_xs9922(sd);
	struct i2c_client *client = xs9922->client;
	int ret = 0;

	dev_dbg(&client->dev, "%s: on %d\n", __func__, on);
#if (!__CLOSE_SENSOR__)
    if (xs9922->power_on) return 0;
#endif

	mutex_lock(&xs9922->mutex);

	/* If the power state is not modified - no work to do. */
	if (xs9922->power_on == !!on)
		goto exit;

	if (on) {
		ret = pm_runtime_get_sync(&client->dev);
		if (ret < 0) {
			pm_runtime_put_noidle(&client->dev);
			goto exit;
		}

		xs9922->power_on = true;
	} else {
		pm_runtime_put(&client->dev);
		xs9922->power_on = false;
	}

exit:
	mutex_unlock(&xs9922->mutex);
	dev_dbg(&client->dev, "%s: on %d ret:%d\n", __func__, on, ret);

	return ret;
}

static int __xs9922_power_on(struct xs9922 *xs9922)
{
	int ret;
	struct device *dev = &xs9922->client->dev;

	dev_dbg(dev, "%s\n", __func__);
#if (!__CLOSE_SENSOR__)
    if (xs9922->power_on ) return 0;
#endif

	if (!IS_ERR_OR_NULL(xs9922->pins_default)) {
		ret = pinctrl_select_state(xs9922->pinctrl,
					   xs9922->pins_default);
		if (ret < 0)
			dev_err(dev, "could not set pins. ret=%d\n", ret);
	}

	if (!IS_ERR(xs9922->power_gpio)) {
		gpiod_set_value_cansleep(xs9922->power_gpio, 1);
		dev_err(dev, "power gpio pull high\n");
		usleep_range(25*1000, 30*1000);
	}

	//usleep_range(1500, 2000);

	ret = clk_set_rate(xs9922->xvclk, XS9922_XVCLK_FREQ);
	if (ret < 0)
		dev_warn(dev, "Failed to set xvclk rate\n");
	if (clk_get_rate(xs9922->xvclk) != XS9922_XVCLK_FREQ)
		dev_warn(dev, "xvclk mismatched\n");
	ret = clk_prepare_enable(xs9922->xvclk);
	if (ret < 0) {
		dev_err(dev, "Failed to enable xvclk\n");
		goto err_clk;
	}

	if (!IS_ERR(xs9922->reset_gpio)) {
		gpiod_set_value_cansleep(xs9922->reset_gpio, 0);
		dev_err(dev, "reset gpio pull high\n");
		usleep_range(5*1000, 10*1000);
		gpiod_set_value_cansleep(xs9922->reset_gpio, 1);
		usleep_range(10*1000, 20*1000);
	}

	//usleep_range(10*1000, 20*1000);

	dev_dbg(dev, "%s OUT\n", __func__);

	return 0;

err_clk:
	if (!IS_ERR(xs9922->reset_gpio))
		gpiod_set_value_cansleep(xs9922->reset_gpio, 1);

	if (!IS_ERR_OR_NULL(xs9922->pins_sleep))
		pinctrl_select_state(xs9922->pinctrl, xs9922->pins_sleep);

	return ret;
}

static void __xs9922_power_off(struct xs9922 *xs9922)
{
    struct device *dev = &xs9922->client->dev;
#if __CLOSE_SENSOR__
	int ret;

	if (!IS_ERR(xs9922->reset_gpio))
		gpiod_set_value_cansleep(xs9922->reset_gpio, 0);
	clk_disable_unprepare(xs9922->xvclk);

	if (!IS_ERR_OR_NULL(xs9922->pins_sleep)) {
		ret = pinctrl_select_state(xs9922->pinctrl,
					   xs9922->pins_sleep);
		if (ret < 0)
			dev_dbg(dev, "could not set pins\n");
	}

	//if (!IS_ERR(xs9922->power_gpio))
		//gpiod_set_value_cansleep(xs9922->power_gpio, 0);
#endif
    dev_dbg(dev, "[hardy] %s:%d\n", __FUNCTION__, __LINE__);
}

/* Get status of additional camera capabilities */
static int xs9922_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct xs9922 *priv = container_of(ctrl->handler, struct xs9922, ctrl_handler);
	struct v4l2_subdev *sd = &priv->subdev;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
    int ret, value;

	dev_dbg(&client->dev, "id: %d\n", ctrl->id);

	switch (ctrl->id) {
	default:
		return -EINVAL;
    case V4L2_CID_CONTRAST:
        // 0~255  <-  0~255
        return xs9922_read_reg(client, 0x0106, XS9922_REG_VALUE_08BIT, &ctrl->val);
	case V4L2_CID_BRIGHTNESS:
        // 0~255  <-  -128~127
        ret = xs9922_read_reg(client, 0x0107, XS9922_REG_VALUE_08BIT, &value);
        ctrl->val = value+128;
        return ret;
	case V4L2_CID_SATURATION:
        // 0~255  <-  0~255
        return xs9922_read_reg(client, 0x0108, XS9922_REG_VALUE_08BIT, &ctrl->val);
	case V4L2_CID_HUE:
        // -128~127  <-  0~255
        ret = xs9922_read_reg(client, 0x0109, XS9922_REG_VALUE_08BIT, &value);
        ctrl->val = value+128;
        return ret;
	}
}

/* Set status of additional camera capabilities */
static int xs9922_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct xs9922 *priv = container_of(ctrl->handler, struct xs9922, ctrl_handler);
	struct v4l2_subdev *sd = &priv->subdev;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
    int value = 0;

	dev_dbg(&client->dev, "id: %d val:%d\n", ctrl->id, ctrl->val);

	switch (ctrl->id) {
	default:
		return -EINVAL;
    case V4L2_CID_CONTRAST:
        // 0~255  ->  0~255
        xs9922_write_reg(client, 0x1106, XS9922_REG_VALUE_08BIT, ctrl->val);
        xs9922_write_reg(client, 0x2106, XS9922_REG_VALUE_08BIT, ctrl->val);
        xs9922_write_reg(client, 0x3106, XS9922_REG_VALUE_08BIT, ctrl->val);
        return xs9922_write_reg(client, 0x0106, XS9922_REG_VALUE_08BIT, ctrl->val);
	case V4L2_CID_BRIGHTNESS:
        // 0~255  ->  -128~127
        value = ctrl->val - 128;
        xs9922_write_reg(client, 0x1107, XS9922_REG_VALUE_08BIT, value);
        xs9922_write_reg(client, 0x2107, XS9922_REG_VALUE_08BIT, value);
        xs9922_write_reg(client, 0x3107, XS9922_REG_VALUE_08BIT, value);
        return xs9922_write_reg(client, 0x0107, XS9922_REG_VALUE_08BIT, value);
	case V4L2_CID_SATURATION:
        // 0~255  ->  0~255
        xs9922_write_reg(client, 0x1108, XS9922_REG_VALUE_08BIT, ctrl->val);
        xs9922_write_reg(client, 0x2108, XS9922_REG_VALUE_08BIT, ctrl->val);
        xs9922_write_reg(client, 0x3108, XS9922_REG_VALUE_08BIT, ctrl->val);
        return xs9922_write_reg(client, 0x0108, XS9922_REG_VALUE_08BIT, ctrl->val);
	case V4L2_CID_HUE:
        // 0~255  ->  -128~127
        value = ctrl->val - 128;
        xs9922_write_reg(client, 0x1109, XS9922_REG_VALUE_08BIT, value);
        xs9922_write_reg(client, 0x2109, XS9922_REG_VALUE_08BIT, value);
        xs9922_write_reg(client, 0x3109, XS9922_REG_VALUE_08BIT, value);
        return xs9922_write_reg(client, 0x0109, XS9922_REG_VALUE_08BIT, value);
	}
}

static const struct v4l2_ctrl_ops xs9922_ctrl_ops = {
    .g_volatile_ctrl = xs9922_g_volatile_ctrl,
    .s_ctrl = xs9922_s_ctrl,
};

static int xs9922_initialize_controls(struct xs9922 *xs9922)
{
	const struct xs9922_mode *mode;
	struct v4l2_ctrl_handler *handler;
	u64 pixel_rate;
	int ret;

	handler = &xs9922->ctrl_handler;
	mode = xs9922->cur_mode;
	ret = v4l2_ctrl_handler_init(handler, 5);
	if (ret)
		return ret;
	handler->lock = &xs9922->mutex;

	xs9922->link_freq = v4l2_ctrl_new_int_menu(handler, NULL,
				V4L2_CID_LINK_FREQ,
				ARRAY_SIZE(link_freq_items) - 1, 0,
				link_freq_items);
	__v4l2_ctrl_s_ctrl(xs9922->link_freq, mode->mipi_freq_idx);

	/* pixel rate = link frequency * 2 * lanes / BITS_PER_SAMPLE */
	pixel_rate = (u32)link_freq_items[mode->mipi_freq_idx] / mode->bpp * 2 * XS9922_LANES;
	dev_err(&xs9922->client->dev,
				"pixel_rate(%u)\n", (u32)pixel_rate);

	xs9922->pixel_rate = v4l2_ctrl_new_std(handler, NULL,
		V4L2_CID_PIXEL_RATE, 0, pixel_rate,
		1, pixel_rate);

    v4l2_ctrl_new_std(handler, &xs9922_ctrl_ops, V4L2_CID_BRIGHTNESS, 0, 0xff, 1, 0x80);
    v4l2_ctrl_new_std(handler, &xs9922_ctrl_ops, V4L2_CID_CONTRAST, 0, 0xff, 1, 0x80);
    v4l2_ctrl_new_std(handler, &xs9922_ctrl_ops, V4L2_CID_SATURATION, 0, 0xff, 1, 0x80);
    v4l2_ctrl_new_std(handler, &xs9922_ctrl_ops, V4L2_CID_HUE, 0, 0xff, 1, 0x80);

	if (handler->error) {
		ret = handler->error;
		dev_err(&xs9922->client->dev,
			"Failed to init controls(%d)\n", ret);
		goto err_free_handler;
	}

	xs9922->subdev.ctrl_handler = handler;

	return 0;

err_free_handler:
	v4l2_ctrl_handler_free(handler);

	return ret;
}

#if __CLOSE_SENSOR__
static int xs9922_runtime_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct xs9922 *xs9922 = to_xs9922(sd);
	dev_dbg(&client->dev, "%s \n",__func__);

	return __xs9922_power_on(xs9922);
}

static int xs9922_runtime_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct xs9922 *xs9922 = to_xs9922(sd);
	dev_dbg(&client->dev, "%s \n",__func__);

	__xs9922_power_off(xs9922);

	return 0;
}
#endif

#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
static int xs9922_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct xs9922 *xs9922 = to_xs9922(sd);
	struct v4l2_mbus_framefmt *try_fmt =
				v4l2_subdev_get_try_format(sd, fh->pad, 0);
	const struct xs9922_mode *def_mode = xs9922->cur_mode;

	dev_dbg(&xs9922->client->dev, "%s\n", __func__);

	mutex_lock(&xs9922->mutex);
	/* Initialize try_fmt */
	try_fmt->width = def_mode->width;
	try_fmt->height = def_mode->height;
	try_fmt->code = def_mode->bus_fmt;
	try_fmt->field = V4L2_FIELD_NONE;

	mutex_unlock(&xs9922->mutex);
	/* No crop or compose */

	return 0;
}
#endif

#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
static const struct v4l2_subdev_internal_ops xs9922_internal_ops = {
	.open = xs9922_open,
};
#endif

static const struct v4l2_subdev_video_ops xs9922_video_ops = {
	.s_stream = xs9922_stream,
	//.g_mbus_config = xs9922_g_mbus_config,
  .g_frame_interval = xs9922_g_frame_interval,
};

static const struct v4l2_subdev_pad_ops xs9922_subdev_pad_ops = {
	.enum_mbus_code = xs9922_enum_mbus_code,
	.enum_frame_size = xs9922_enum_frame_sizes,
	.enum_frame_interval = xs9922_enum_frame_interval,
	.get_fmt = xs9922_get_fmt,
	.set_fmt = xs9922_set_fmt,
  .get_mbus_config = xs9922_g_mbus_config,
};

static const struct v4l2_subdev_core_ops xs9922_core_ops = {
	.s_power = xs9922_power,
	.ioctl = xs9922_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = xs9922_compat_ioctl32,
#endif
};

static const struct v4l2_subdev_ops xs9922_subdev_ops = {
	.core = &xs9922_core_ops,
	.video = &xs9922_video_ops,
	.pad   = &xs9922_subdev_pad_ops,
};

static int __maybe_unused check_chip_id(struct i2c_client *client){
	struct device *dev = &client->dev;
	u32 chip_id0 = 0;
	u32 chip_id1 = 0;


	xs9922_read_reg(client, NI_40F0_DEVICE_ID_1, XS9922_REG_VALUE_08BIT, &chip_id1);
	xs9922_read_reg(client, NI_40F1_DEVICE_ID_0, XS9922_REG_VALUE_08BIT, &chip_id0);

	dev_err(dev, "chip_id : 0x%04x\n", chip_id1 << 8| chip_id0);

	if((chip_id1 != NI_ID1) ||(chip_id0 != NI_ID0))
	{
		dev_err(dev, "the id of the ni9922 don't match\n");
		dev_err(dev, "chip_id1 = %02x should be 0x99\n",chip_id1);
		dev_err(dev, "chip_id0 = %02x should be 0x22\n",chip_id0);

		return -1;
	}

	return 0;
}

static int xs9922_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct device_node *node = dev->of_node;
	struct xs9922 *xs9922;
	struct v4l2_subdev *sd;
	int ret;

	dev_info(dev, "driver version: %02x.%02x.%02x",
		 DRIVER_VERSION >> 16,
		 (DRIVER_VERSION & 0xff00) >> 8,
		 DRIVER_VERSION & 0x00ff);

	xs9922 = devm_kzalloc(dev, sizeof(*xs9922), GFP_KERNEL);
	if (!xs9922)
		return -ENOMEM;

	ret = of_property_read_u32(node, RKMODULE_CAMERA_MODULE_INDEX,
				   &xs9922->module_index);
	ret |= of_property_read_string(node, RKMODULE_CAMERA_MODULE_NAME,
				       &xs9922->module_name);
	ret |= of_property_read_string(node, RKMODULE_CAMERA_LENS_NAME,
				       &xs9922->len_name);
	if (ret) {
		dev_err(dev, "could not get %s!\n", RKMODULE_CAMERA_LENS_NAME);
		return -EINVAL;
	}

	xs9922->client = client;
	xs9922->cur_mode = &supported_modes[0];
    xs9922->cfg_num = ARRAY_SIZE(supported_modes);

	xs9922->xvclk = devm_clk_get(dev, "xvclk");
	if (IS_ERR(xs9922->xvclk)) {
		dev_err(dev, "Failed to get xvclk\n");
		return -EINVAL;
	}

	xs9922->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(xs9922->reset_gpio))
		dev_warn(dev, "Failed to get reset-gpios\n");

	xs9922->power_gpio = devm_gpiod_get(dev, "power", GPIOD_OUT_LOW);
	if (IS_ERR(xs9922->power_gpio))
		dev_warn(dev, "Failed to get power-gpios\n");

    xs9922->cam_gpio = devm_gpiod_get(dev, "camera", GPIOD_OUT_LOW);

	xs9922->pinctrl = devm_pinctrl_get(dev);
	if (!IS_ERR(xs9922->pinctrl)) {
		xs9922->pins_default =
			pinctrl_lookup_state(xs9922->pinctrl,
					     OF_CAMERA_PINCTRL_STATE_DEFAULT);
		if (IS_ERR(xs9922->pins_default))
			dev_info(dev, "could not get default pinstate\n");

		xs9922->pins_sleep =
			pinctrl_lookup_state(xs9922->pinctrl,
					     OF_CAMERA_PINCTRL_STATE_SLEEP);
		if (IS_ERR(xs9922->pins_sleep))
			dev_info(dev, "could not get sleep pinstate\n");
	} else {
		dev_info(dev, "no pinctrl\n");
	}

	mutex_init(&xs9922->mutex);

	sd = &xs9922->subdev;
	v4l2_i2c_subdev_init(sd, client, &xs9922_subdev_ops);
	ret = xs9922_initialize_controls(xs9922);
	if (ret) {
		dev_err(dev, "Failed to initialize controls xs9922\n");
		goto err_destroy_mutex;
	}

	ret = __xs9922_power_on(xs9922);
	if (ret) {
		dev_err(dev, "Failed to power on xs9922\n");
		goto err_free_handler;
	}

	ret = check_chip_id(client);
	if (ret) {
		dev_err(dev, "Failed to check senosr id\n");
		goto err_free_handler;
  }


#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
	sd->internal_ops = &xs9922_internal_ops;
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
#endif

#if defined(CONFIG_MEDIA_CONTROLLER)
	xs9922->pad.flags = MEDIA_PAD_FL_SOURCE;
	sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret = media_entity_pads_init(&sd->entity, 1, &xs9922->pad);
	if (ret < 0)
		goto err_power_off;
#endif

	snprintf(sd->name, sizeof(sd->name), "m%02d_%s %s",
		 xs9922->module_index, XS9922_NAME, dev_name(sd->dev));

	ret = v4l2_async_register_subdev_sensor_common(sd);
	if (ret) {
		dev_err(dev, "v4l2 async register subdev failed\n");
		goto err_clean_entity;
	}

	if (sysfs_create_group(&dev->kobj, &dev_attr_grp))
		return -ENODEV;

	xs9922->input_dev = devm_input_allocate_device(dev);
	if (xs9922->input_dev == NULL) {
		dev_err(dev, "failed to allocate xs9922 input device\n");
		return -ENOMEM;
	}
	xs9922->input_dev->name = "xs9922_input_event";
	set_bit(EV_MSC,  xs9922->input_dev->evbit);
	set_bit(MSC_RAW, xs9922->input_dev->mscbit);

	ret = input_register_device(xs9922->input_dev);
	if (ret) {
		pr_err("%s: failed to register xs9922 input device\n", __func__);
		return ret;
	}
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	pm_runtime_idle(dev);

    if (1)
	{
        struct task_struct *read_thread = kthread_create(read_config, xs9922, "xs9922_readconfig");
    	if (IS_ERR(read_thread)) {
    		printk(KERN_ERR "kthread_create xs9922_readconfig failed\n");
    	} else {
        	wake_up_process(read_thread);
    	}
	}

	dev_dbg(dev, "%s run here\n", __func__);
	return 0;

err_clean_entity:
#if defined(CONFIG_MEDIA_CONTROLLER)
	media_entity_cleanup(&sd->entity);
#endif
err_power_off:
	__xs9922_power_off(xs9922);
err_free_handler:
	v4l2_ctrl_handler_free(&xs9922->ctrl_handler);
err_destroy_mutex:
	mutex_destroy(&xs9922->mutex);

	return ret;
}

static int xs9922_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct xs9922 *xs9922 = to_xs9922(sd);

    if (!IS_ERR(xs9922->cam_gpio)) {
        gpiod_set_value_cansleep(xs9922->cam_gpio, 0);
    }
    if (!IS_ERR(xs9922->power_gpio)) {
        gpiod_set_value_cansleep(xs9922->power_gpio, 0);
    }

	v4l2_async_unregister_subdev(sd);
#if defined(CONFIG_MEDIA_CONTROLLER)
	media_entity_cleanup(&sd->entity);
#endif
	v4l2_ctrl_handler_free(&xs9922->ctrl_handler);
	mutex_destroy(&xs9922->mutex);

	pm_runtime_disable(&client->dev);
	if (!pm_runtime_status_suspended(&client->dev))
		__xs9922_power_off(xs9922);
	pm_runtime_set_suspended(&client->dev);

	return 0;
}

#if __CLOSE_SENSOR__
static const struct dev_pm_ops xs9922_pm_ops = {
	SET_RUNTIME_PM_OPS(xs9922_runtime_suspend,
			   xs9922_runtime_resume, NULL)
};
#endif

#if IS_ENABLED(CONFIG_OF)
static const struct of_device_id xs9922_of_match[] = {
	{ .compatible = "xs9922" },
	{},
};
MODULE_DEVICE_TABLE(of, xs9922_of_match);
#endif

static const struct i2c_device_id xs9922_match_id[] = {
	{ "xs9922", 0 },
	{ },
};

static struct i2c_driver xs9922_i2c_driver = {
	.driver = {
		.name = XS9922_NAME,
#if __CLOSE_SENSOR__
		.pm = &xs9922_pm_ops,
#endif
		.of_match_table = of_match_ptr(xs9922_of_match),
	},
	.probe		= &xs9922_probe,
	.remove		= &xs9922_remove,
	.id_table	= xs9922_match_id,
};

module_i2c_driver(xs9922_i2c_driver);

MODULE_AUTHOR("hardy <yangjianzhong@percherry.com>");
MODULE_DESCRIPTION("xs9922 sensor driver");
MODULE_LICENSE("GPL v2");
