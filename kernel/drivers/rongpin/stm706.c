/*
 * GPIO driver for RICOH583 power management chip.
 *
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 * Author: Laxman dewangan <ldewangan@nvidia.com>
 *
 * Based on code
 *	Copyright (C) 2011 RICOH COMPANY,LTD
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <linux/module.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/version.h>

struct stm_gpio{
	int reset_gpio;
	int wdt_gpio;
};

static struct timer_list mytimer;
static struct stm_gpio stm706_gpio;
static int wdi_status = 1;
static int panic_test = 0;

static ssize_t wtd_write(struct file *file, const char *buffer,size_t count, loff_t *data)
{
//	int * add;

	panic_test = 1;
	return count;
/*
	add = ioremap(0x0, 0x100);
	*add = 0;
			
	return count;
*/
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops wtd = {
	.proc_write          = wtd_write,
};
#else
static const struct file_operations wtd = {
	.owner		= THIS_MODULE,
	.write		= wtd_write,
};
#endif

static struct proc_dir_entry *wtd_ctl_entry;

void wdt_function(struct timer_list* list){
//void wdt_function(unsigned long data){
	wdi_status ^= 1;
	
	if (1 == panic_test){
		return;
	}

	gpio_direction_output(stm706_gpio.wdt_gpio,wdi_status);
	gpio_direction_output(stm706_gpio.reset_gpio,1);
	mod_timer(&mytimer, jiffies + msecs_to_jiffies(200));
	
	return;
}

static int stm706_probe(struct platform_device *pdev)
{
	enum of_gpio_flags flags;
	struct device_node *node = pdev->dev.of_node;
	printk("start stm706 probe");

	stm706_gpio.reset_gpio = of_get_named_gpio_flags(node, "reset_gpio", 0, &flags);
	if (!gpio_is_valid(stm706_gpio.reset_gpio)){
		printk("reset_gpio invalid gpio: %d\n",stm706_gpio.reset_gpio);
	}
	gpio_request(stm706_gpio.reset_gpio, "reset_gpio");
	gpio_direction_output(stm706_gpio.reset_gpio,0);
	printk("reset_gpio gpio: %d\n",stm706_gpio.reset_gpio);

	stm706_gpio.wdt_gpio = of_get_named_gpio_flags(node, "wdt_gpio", 0, &flags);
	if (!gpio_is_valid(stm706_gpio.wdt_gpio)){
		printk("wdt_gpio invalid gpio: %d\n",stm706_gpio.wdt_gpio);
	}
	gpio_request(stm706_gpio.wdt_gpio, "wdt_gpio");
	gpio_direction_output(stm706_gpio.wdt_gpio,wdi_status);
	printk("wdt_gpio gpio: %d\n",stm706_gpio.wdt_gpio);	

/*
	init_timer(&mytimer);
	mytimer.expires = jiffies + jiffies_to_msecs(2);
	mytimer.function = wdt_function;
	mytimer.data = 0;
	add_timer(&mytimer);
*/
                timer_setup(&mytimer, wdt_function, 0);
                mytimer.expires = jiffies + msecs_to_jiffies(200);
                add_timer(&mytimer);

	wtd_ctl_entry = proc_mkdir("wtd", NULL);
	proc_create("wtd_ctrl",0666,wtd_ctl_entry,&wtd);

	return 0;
}

static int stm706_remove(struct platform_device *pdev)
{
	del_timer(&mytimer);
	return 0;
}
static int stm706_suspend(struct platform_device *pdev, pm_message_t state)
{
	del_timer(&mytimer);
	return 0;
}
static int stm706_resume(struct platform_device *pdev)
{
	add_timer(&mytimer);
	return 0;
}

static struct of_device_id stm706_dt_ids[] = {
	{ .compatible = "stm706" },
	{}
};
MODULE_DEVICE_TABLE(of, stm706_dt_ids);

static struct platform_driver stm706_driver = {
	.driver = {
		.name    = "stm706",
		.owner   = THIS_MODULE,
		.of_match_table = of_match_ptr(stm706_dt_ids),
	},
	.suspend = stm706_suspend,
	.resume = stm706_resume,
	.probe		= stm706_probe,
	.remove		= stm706_remove,
};

static int __init stm706_init(void)
{
	return platform_driver_register(&stm706_driver);
}
subsys_initcall(stm706_init);

static void __exit stm706_exit(void)
{
	platform_driver_unregister(&stm706_driver);
}
module_exit(stm706_exit);

MODULE_LICENSE("GPL");

