#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/init.h>
#include <linux/i2c-algo-bit.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>


#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_graph.h>

#include <linux/of_gpio.h>
#include <linux/of_platform.h>

#define lt8912_i2c_NAME "lt8912_i2c"
#define lt8912_i2c_SPEED 100*1000 

#define HTU_ADDR		0x80
#define HTU_ADDR_WR (HTU_ADDR & 0xfe)
#define HTU_ADDR_RD (HTU_ADDR | 0x01)

//zhuji
#define HTU_TEMP1    0xe3
#define HTU_HUMi1    0Xe5

//非主机模式
#define HTU_TEMP2    0xf3
#define HTU_HUMI2    0Xf5

#define HTU_SOFTWARE_RESET  0xfe

struct i2c_client *lt8912_i2c_client;

//static struct gpio_desc *reset_gpio;


static int lt8912_i2c_write_reg(struct i2c_client *client, int reg, int val)
{
	int ret;
	
	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret < 0) {
		printk("write reg 0x%x failed\n",reg);
		return ret;
	}

	return ret;
}

static int lt8912_i2c_read_reg(struct i2c_client *client, int reg, int *buf)
{
	int ret;

	ret = i2c_smbus_read_byte_data(client, reg);
	if(ret < 0)	{
		printk("read_reg: recv failed! ret = %d\n",ret);
		return ret;
	}
	*buf = ret;
	return 0;
}



static int lt8912_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int version[2];
	int hactive, hfp, hsync, hbp, vfp, vsync, vbp, htotal, vtotal;
	int lanes;
	
	unsigned int reset_flag, reset_gpio;
	struct device_node *node;

	node = client->dev.of_node;
	
	
	printk("-------------lt8912_i2c probe");
		
	
	lt8912_i2c_client = client;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		printk("----------------i2c fail");
		return -1;
	}
	else 
	{
		printk("-----------------i2c success");
	}
	
	
	reset_gpio = of_get_named_gpio_flags(node, "reset-gpios", 0, &reset_flag);
	/*
	reset_gpio = devm_gpiod_get_optional(&client->dev, "reset", GPIOD_ASIS);
	if (IS_ERR(reset_gpio)) {
		dev_err(&client->dev, "lt8912 failed to request reset GPIO\n");
	}
	
	
		gpiod_direction_output(reset_gpio, 0);
		msleep(100);
		gpiod_direction_output(reset_gpio, 1);
		msleep(100);
	*/
	
	
	
	if(gpio_is_valid(reset_gpio)){
		gpio_request(reset_gpio, "reset-gpio");
		gpio_direction_output(reset_gpio, ~reset_flag);
		gpio_set_value(reset_gpio, ~reset_flag);
		msleep(100);
		gpio_set_value(reset_gpio, reset_flag);
		msleep(100);
	}
	
	hactive = 1920;
	hfp = 88;
	hsync = 44;
	hbp = 148;
	vfp = 4;
	vsync = 5;
	vbp = 36;
	htotal = 2200;
	vtotal = 1125;
	
	
	printk("******************************\n");
	printk("hactive %d\n",hactive);
	printk("hfp %d\n",hfp);
	printk("hsync %d\n",hsync);
	printk("hbp %d\n",hbp);
	printk("vfp %d\n",vfp);
	printk("vsync %d\n",vsync);
	printk("vbp %d\n",vbp);
	printk("htotal %d\n",htotal);
	printk("vtotal %d\n",vtotal);
	
	printk("******************************\n");
	
	lanes = 4;
	
	version[0] = 0;
	version[1] = 1;
	
	lt8912_i2c_read_reg(client, 0x00, &version[0]);
	lt8912_i2c_read_reg(client, 0x01, &version[1]);

	printk("LT8912 ID: %02x, %02x\n", version[0], version[1]);
		 
		 
	client->addr = 0x48;

	/* DigitalClockEn */
	lt8912_i2c_write_reg(client, 0x08, 0xff);
	lt8912_i2c_write_reg(client, 0x09, 0x81);
	lt8912_i2c_write_reg(client, 0x0a, 0xff);
	lt8912_i2c_write_reg(client, 0x0b, 0x64);
	lt8912_i2c_write_reg(client, 0x0c, 0xff);

	lt8912_i2c_write_reg(client, 0x44, 0x31);
	lt8912_i2c_write_reg(client, 0x51, 0x1f);

	/* TxAnalog */
	lt8912_i2c_write_reg(client, 0x31, 0xa1);
	lt8912_i2c_write_reg(client, 0x32, 0xa1);
	lt8912_i2c_write_reg(client, 0x33, 0x03);
	lt8912_i2c_write_reg(client, 0x37, 0x00);
	lt8912_i2c_write_reg(client, 0x38, 0x22);
	lt8912_i2c_write_reg(client, 0x60, 0x82);

	/* CbusAnalog */
	lt8912_i2c_write_reg(client, 0x39, 0x45);
	lt8912_i2c_write_reg(client, 0x3b, 0x00);

	/* HDMIPllAnalog */
	lt8912_i2c_write_reg(client, 0x44, 0x31);
	lt8912_i2c_write_reg(client, 0x55, 0x44);
	lt8912_i2c_write_reg(client, 0x57, 0x01);
	lt8912_i2c_write_reg(client, 0x5a, 0x02);


	client->addr = 0x49;
	
	/* MipiBasicSet */
	lt8912_i2c_write_reg(client, 0x10, 0x01);
	lt8912_i2c_write_reg(client, 0x11, 0x08);
	lt8912_i2c_write_reg(client, 0x12, 0x04);
	lt8912_i2c_write_reg(client, 0x13, lanes % 4);
	lt8912_i2c_write_reg(client, 0x14, 0x00);

	lt8912_i2c_write_reg(client, 0x15, 0x00);
	lt8912_i2c_write_reg(client, 0x1a, 0x03);
	lt8912_i2c_write_reg(client, 0x1b, 0x03);

	/* MIPIDig */
	lt8912_i2c_write_reg(client, 0x18, hsync);
	lt8912_i2c_write_reg(client, 0x19, vsync);
	lt8912_i2c_write_reg(client, 0x1c, hactive);
	lt8912_i2c_write_reg(client, 0x1d, hactive >> 8);

	lt8912_i2c_write_reg(client, 0x1e, 0x67);
	lt8912_i2c_write_reg(client, 0x2f, 0x0c);

	lt8912_i2c_write_reg(client, 0x34, htotal);
	lt8912_i2c_write_reg(client, 0x35, htotal >> 8);
	lt8912_i2c_write_reg(client, 0x36, vtotal);
	lt8912_i2c_write_reg(client, 0x37, vtotal >> 8);
	lt8912_i2c_write_reg(client, 0x38, vbp);
	lt8912_i2c_write_reg(client, 0x39, vbp >> 8);
	lt8912_i2c_write_reg(client, 0x3a, vfp);
	lt8912_i2c_write_reg(client, 0x3b, vfp >> 8);
	lt8912_i2c_write_reg(client, 0x3c, hbp);
	lt8912_i2c_write_reg(client, 0x3d, hbp >> 8);
	lt8912_i2c_write_reg(client, 0x3e, hfp);
	lt8912_i2c_write_reg(client, 0x3f, hfp >> 8);

	/* DDSConfig */
	lt8912_i2c_write_reg(client, 0x4e, 0x52);
	lt8912_i2c_write_reg(client, 0x4f, 0xde);
	lt8912_i2c_write_reg(client, 0x50, 0xc0);
	lt8912_i2c_write_reg(client, 0x51, 0x80);
	lt8912_i2c_write_reg(client, 0x51, 0x00);

	lt8912_i2c_write_reg(client, 0x1f, 0x5e);
	lt8912_i2c_write_reg(client, 0x20, 0x01);
	lt8912_i2c_write_reg(client, 0x21, 0x2c);
	lt8912_i2c_write_reg(client, 0x22, 0x01);
	lt8912_i2c_write_reg(client, 0x23, 0xfa);
	lt8912_i2c_write_reg(client, 0x24, 0x00);
	lt8912_i2c_write_reg(client, 0x25, 0xc8);
	lt8912_i2c_write_reg(client, 0x26, 0x00);
	lt8912_i2c_write_reg(client, 0x27, 0x5e);
	lt8912_i2c_write_reg(client, 0x28, 0x01);
	lt8912_i2c_write_reg(client, 0x29, 0x2c);
	lt8912_i2c_write_reg(client, 0x2a, 0x01);
	lt8912_i2c_write_reg(client, 0x2b, 0xfa);
	lt8912_i2c_write_reg(client, 0x2c, 0x00);
	lt8912_i2c_write_reg(client, 0x2d, 0xc8);
	lt8912_i2c_write_reg(client, 0x2e, 0x00);

	client->addr = 0x48;
	lt8912_i2c_write_reg(client, 0x03, 0x7f);
	usleep_range(10000, 20000);
	lt8912_i2c_write_reg(client, 0x03, 0xff);
	
	
	client->addr = 0x49;

	lt8912_i2c_write_reg(client, 0x42, 0x64);
	lt8912_i2c_write_reg(client, 0x43, 0x00);
	lt8912_i2c_write_reg(client, 0x44, 0x04);
	lt8912_i2c_write_reg(client, 0x45, 0x00);
	lt8912_i2c_write_reg(client, 0x46, 0x59);
	lt8912_i2c_write_reg(client, 0x47, 0x00);
	lt8912_i2c_write_reg(client, 0x48, 0xf2);
	lt8912_i2c_write_reg(client, 0x49, 0x06);
	lt8912_i2c_write_reg(client, 0x4a, 0x00);
	lt8912_i2c_write_reg(client, 0x4b, 0x72);
	lt8912_i2c_write_reg(client, 0x4c, 0x45);
	lt8912_i2c_write_reg(client, 0x4d, 0x00);
	lt8912_i2c_write_reg(client, 0x52, 0x08);
	lt8912_i2c_write_reg(client, 0x53, 0x00);
	lt8912_i2c_write_reg(client, 0x54, 0xb2);
	lt8912_i2c_write_reg(client, 0x55, 0x00);
	lt8912_i2c_write_reg(client, 0x56, 0xe4);
	lt8912_i2c_write_reg(client, 0x57, 0x0d);
	lt8912_i2c_write_reg(client, 0x58, 0x00);
	lt8912_i2c_write_reg(client, 0x59, 0xe4);
	lt8912_i2c_write_reg(client, 0x5a, 0x8a);
	lt8912_i2c_write_reg(client, 0x5b, 0x00);
	lt8912_i2c_write_reg(client, 0x5c, 0x34);
	lt8912_i2c_write_reg(client, 0x1e, 0x4f);
	lt8912_i2c_write_reg(client, 0x51, 0x00);

	client->addr = 0x48;
	lt8912_i2c_write_reg(client, 0xb2, 0x01);
	
	
	client->addr = 0x4a;

	/* AudioIIsEn */
	lt8912_i2c_write_reg(client, 0x06, 0x08);
	lt8912_i2c_write_reg(client, 0x07, 0xf0);

	lt8912_i2c_write_reg(client, 0x34, 0xd2);

	lt8912_i2c_write_reg(client, 0x3c, 0x41);


	client->addr = 0x48;
	/* MIPIRxLogicRes */
	lt8912_i2c_write_reg(client, 0x03, 0x7f);
	usleep_range(10000, 20000);
	lt8912_i2c_write_reg(client, 0x03, 0xff);

	client->addr = 0x49;
	lt8912_i2c_write_reg(client, 0x51, 0x80);
	usleep_range(10000, 20000);
	lt8912_i2c_write_reg(client, 0x51, 0x00);


	return 0;
}

static int lt8912_i2c_remove(struct i2c_client *client)
{
	return 0;
}



static const struct of_device_id lt8912_i2c_match_table[] = {
                {.compatible = "lt8912_i2c",},
                { },
};

static const struct i2c_device_id lt8912_i2c_id[] = {
    { lt8912_i2c_NAME, 0 },
    { }
};



static struct i2c_driver lt8912_i2c_driver = {
    .probe      = lt8912_i2c_probe,
    .remove     = lt8912_i2c_remove,
    .id_table   = lt8912_i2c_id,
    .driver = {
        .name     = lt8912_i2c_NAME,
        .owner    = THIS_MODULE,
        .of_match_table = lt8912_i2c_match_table,
    },
};



module_i2c_driver(lt8912_i2c_driver);

MODULE_DESCRIPTION("lt8912_i2c  Driver");
MODULE_LICENSE("GPL");

