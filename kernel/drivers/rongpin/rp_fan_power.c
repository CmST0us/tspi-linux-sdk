#include <linux/module.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/irq.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/io.h>
#include <linux/interrupt.h>
// #include <linux/wakelock.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/thermal.h>
#include <linux/timer.h>
#define GPIO_PIN 50
#define GPIO_NUM_MAX 40
//static dev_t dev;
//static struct cdev cdev;
static struct workqueue_struct *fan_gpio_workqueue;
//static struct work_struct fan_gpio_work;
//static struct timer_list fan_gpio_timer;

struct fan_gpio{
	int gpio_num;
	int action;
	int temp_on;
	int time;
};

struct fan_gpio_control_data {
    struct fan_gpio fan_gpio_num;
    char thermal_zone_device;   
};

static char* file_name = NULL;
static struct fan_gpio_control_data *fan_gpio_data = NULL;
//static int open_now = 1;
static int fan_temp;
static int fan_time;
static const char *thermal_device;

static int fan_gpio_open(struct inode *inode, struct file *file)
{
        struct dentry* dent = file->f_path.dentry;
        //int i = 0;

        file_name = (char*)(dent->d_name.name);

        /*for (i = 0; i < fan_gpio_data->gpio_dts_num; i++){
                if(!strcmp(file_name,fan_gpio_data->fan_gpio_num[i].gpio_name)){
                        open_now = i;
                }
        }*/
        return 0;
}


static ssize_t fan_gpio_write(struct file *file, const char *buffer,size_t count, loff_t *data)
{
    char temp_str[20];  
    int ret;

    ret = copy_from_user(temp_str, buffer, min(count, sizeof(temp_str) - 1));
    if (ret < 0) {
        return ret;
    }

    temp_str[count] = '\0';

    ret = kstrtoint(temp_str, 10, &fan_temp);
    if (ret < 0) {
        pr_err("Failed to convert input to integer\n");
        return ret;
    }

	return count;
}

static ssize_t fan_gpio_read(struct file *file, char __user * buffer, size_t count, loff_t *data)
{	
    int *new_temp_on = &fan_temp;
    char temp_str[10];  
    int len;
    
    len = snprintf(temp_str, sizeof(temp_str), "%d\n", *new_temp_on);

    temp_str[len] = '\0';

    return simple_read_from_buffer(buffer, count, data, temp_str, len);
}	

static int fan_time_open(struct inode *inode, struct file *file)
{
        struct dentry* dent = file->f_path.dentry;
        //int i = 0;

        file_name = (char*)(dent->d_name.name);

        /*for (i = 0; i < fan_gpio_data->gpio_dts_num; i++){
                if(!strcmp(file_name,fan_gpio_data->fan_gpio_num[i].gpio_name)){
                        open_now = i;
                }
        }*/
        return 0;
}


static ssize_t fan_time_write(struct file *file, const char *buffer,size_t count, loff_t *data)
{
    char time_str[20];
    int ret;

    ret = copy_from_user(time_str, buffer, min(count, sizeof(time_str) - 1));
    if (ret < 0) {
        return ret;
    }

    time_str[count] = '\0';

    ret = kstrtoint(time_str, 10, &fan_time);
    if (ret < 0) {
        pr_err("Failed to convert input to integer\n");
        return ret;
    }

        return count;
}

static ssize_t fan_time_read(struct file *file, char __user * buffer, size_t count, loff_t *data)
{
    int *new_time = &fan_time;
    char time_str[10];
    int len;

    len = snprintf(time_str, sizeof(time_str), "%d\n", *new_time);

    time_str[len] = '\0';

    return simple_read_from_buffer(buffer, count, data, time_str, len);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops fan_gpio_ops = {
    .proc_open           = fan_gpio_open,
    .proc_write          = fan_gpio_write,
    .proc_read		 = fan_gpio_read,
};
static const struct proc_ops fan_time_ops = {
    .proc_open           = fan_time_open,
    .proc_write          = fan_time_write,
    .proc_read           = fan_time_read,
};
#else
static struct file_operations fan_gpio_fops = {
    .owner = THIS_MODULE,
    .open = fan_gpio_open,
    .write = fan_gpio_write,
    .read  = fan_gpio_read,
};
static struct file_operations fan_time_fops = {
    .owner = THIS_MODULE,
    .open = fan_time_open,
    .write = fan_time_write,
    .read  = fan_time_read,
};
#endif

static struct delayed_work fan_gpio_delayed_work;

static void fan_gpio_delayed_work_callback(struct work_struct *work){
	int temperature;
	struct thermal_zone_device *thermal_dev;
	int temp;
	
	thermal_dev = thermal_zone_get_zone_by_name(thermal_device);

	temp = thermal_zone_get_temp(thermal_dev, &temperature);

	if(temperature < fan_temp) {

         gpio_set_value(fan_gpio_data->fan_gpio_num.gpio_num, 0);
         queue_delayed_work(fan_gpio_workqueue, &fan_gpio_delayed_work, msecs_to_jiffies(1000));
	}
	else if(temperature >= fan_temp) {

	 gpio_set_value(fan_gpio_data->fan_gpio_num.gpio_num, 1);
	 queue_delayed_work(fan_gpio_workqueue, &fan_gpio_delayed_work, msecs_to_jiffies(fan_time));
	}
	else{
	printk("error:work_handler failed");
	queue_delayed_work(fan_gpio_workqueue, &fan_gpio_delayed_work, msecs_to_jiffies(1000));
	}

};

//static void fan_gpio_timer_callback(struct timer_list *t){
//	queue_work(fan_gpio_workqueue, &fan_gpio_work);
//}

static int fan_gpio_probe(struct platform_device *pdev){
    
    int ret;
    struct device_node *np = pdev->dev.of_node;
    struct device_node *tmp = pdev->dev.of_node;
    enum of_gpio_flags  gpio_flags;
    static struct proc_dir_entry *root_entry_fan;
    //struct device_node *child_np;

   printk("init fan_gpio probe");

   fan_gpio_data = devm_kzalloc(&pdev->dev, sizeof(struct fan_gpio_control_data), GFP_KERNEL);
   
   if (!fan_gpio_data) {
        
	   return -ENOMEM;
    }

   //Get Data from DTS
   fan_gpio_data->fan_gpio_num.gpio_num = of_get_named_gpio_flags(np, "gpio-pin", 0, &gpio_flags);
   fan_gpio_data->thermal_zone_device = of_property_read_string(tmp, "temperature-device", &thermal_device);
   of_property_read_u32(np, "temp-on",&fan_temp);
   of_property_read_u32(np, "time",&fan_time);
   

   ret = gpio_request(fan_gpio_data->fan_gpio_num.gpio_num, "fan_gpio_control");
   if(ret < 0){
        
	   pr_err("Failed to request GPIO %d\n", GPIO_PIN);
        
	   return -EINVAL;
    }

    gpio_direction_output(fan_gpio_data->fan_gpio_num.gpio_num, gpio_flags);

    //Create proc/rp_fan
    root_entry_fan = proc_mkdir("rp_fan", NULL);
    proc_create("temp", 0666 , root_entry_fan , &fan_gpio_ops);
    proc_create("time", 0666 , root_entry_fan , &fan_time_ops);

    //Work team
    fan_gpio_workqueue = create_workqueue("fan_gpio_workqueue");
    if(!fan_gpio_workqueue) {
	    
	    pr_err("Failed to get work handler");
	    return -EINVAL;
    }
    
    INIT_DELAYED_WORK(&fan_gpio_delayed_work, fan_gpio_delayed_work_callback);
    queue_delayed_work(fan_gpio_workqueue, &fan_gpio_delayed_work, msecs_to_jiffies(1000));   


    //timer
    //timer_setup(&fan_gpio_timer,fan_gpio_timer_callback,0);
    //mod_timer(&fan_gpio_timer, jiffies + msecs_to_jiffies(1000));

    platform_set_drvdata(pdev, fan_gpio_data);
    pr_info("GPIO Control Driver Loaded\n");
    return 0;
}

static int fan_gpio_remove(struct platform_device *pdev) {

    pr_info("GPIO Control Driver Unloaded\n");
    return 0;
}

static const struct of_device_id gpio_of_match[] = {
    { .compatible = "fan_gpio_control" },
    { }
};

static struct platform_driver fan_gpio_driver = {
    .probe = fan_gpio_probe,
    .remove = fan_gpio_remove,
    .driver = {
                .name           = "fan_gpio_control",
                .of_match_table = of_match_ptr(gpio_of_match),
        },
};


MODULE_LICENSE("GPL");
module_platform_driver(fan_gpio_driver);
