#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/spinlock.h>
#include <dt-bindings/input/gpio-keys.h>
#include <linux/device.h>

#define rptips(str, ...) printk("rptips: "str, ##__VA_ARGS__)

struct gpio_keys_button {
    unsigned int code;
    int active_low;
    const char *label;
    unsigned int type;
    int wakeup;
    int debounce_interval;
    int value;
    /*unsigned int trigger;*/
    unsigned int press_type;// >0:long | 0:short
};

struct gpio_keys_platform_data {
    const struct gpio_keys_button *buttons;
    int nbuttons;
    unsigned int rep:1;
    const char *label;
};

struct gpio_button_data {
        const struct gpio_keys_button *button;
        struct input_dev *input;
        struct gpio_desc *gpiod;
        unsigned short *code;
        struct delayed_work work;
        unsigned int press;	
        unsigned int irq;
};

struct gpio_keys_drvdata {
        const struct gpio_keys_platform_data *pdata;
        struct input_dev *input;
        unsigned short *keymap;	
        struct gpio_button_data data[];
};

static int gpio_keys_enable_wakeup(struct gpio_keys_drvdata *ddata);
static int gpio_keys_button_enable_wakeup(struct gpio_button_data *bdata);
static void gpio_keys_gpio_work_func(struct work_struct *work);
static irqreturn_t gpio_keys_gpio_isr(int irq, void *dev_id);
static struct gpio_keys_platform_data* gpio_keys_get_data_from_devtree(struct device *dev);
static int gpio_keys_setup_key(struct platform_device *pdev, struct input_dev *input,
                struct gpio_keys_drvdata *ddata, const struct gpio_keys_button *button,
                int idx, struct fwnode_handle *child);
static int gpio_keys_probe(struct platform_device *pdev);

static void gpio_keys_gpio_work_func(struct work_struct *work){
	struct gpio_button_data *bdata =
        container_of(work, struct gpio_button_data, work.work);
	struct input_dev *input = bdata->input;
	int val;
	
	val = gpiod_get_value_cansleep(bdata->gpiod);
    	if (val < 0) {
        	rptips("err get gpio val: %d\n", val);
        	return;
    	}
        input_event(input, EV_KEY, *bdata->code, !!val);
        input_sync(input);
	bdata->press = !!val;
	if (bdata->button->wakeup)
        	pm_relax(bdata->input->dev.parent);

}

static irqreturn_t gpio_keys_gpio_isr(int irq, void *dev_id){
// interrupt service routine
	struct gpio_button_data *bdata = dev_id;

	if(bdata->button->wakeup) 
		pm_stay_awake(bdata->input->dev.parent);
	
	mod_delayed_work(system_wq,
             &bdata->work,
             msecs_to_jiffies(bdata->button->debounce_interval + !bdata->press * bdata->button->press_type * 1000));

	return IRQ_HANDLED;
}

static struct gpio_keys_platform_data*
gpio_keys_get_data_from_devtree(struct device *dev){
// parse data from device tree to platform data
	int nbuttons = 0;
	struct gpio_keys_platform_data *pdata;
	struct gpio_keys_button *button;
	struct fwnode_handle *child;

	nbuttons = device_get_child_node_count(dev);
	if(!nbuttons){
		rptips("no keys dev\n");
		return ERR_PTR(-ENODEV);
	}
	rptips("button number: %d\n", nbuttons);

	pdata = devm_kzalloc(dev, sizeof(*pdata) + nbuttons * sizeof(*button), GFP_KERNEL);
	if(!pdata){
		rptips("data alloc failed\n");
		return ERR_PTR(-ENOMEM);
	}

	button = (struct gpio_keys_button*)(pdata + 1);

	pdata->buttons = button;
	pdata->nbuttons = nbuttons;

	device_property_read_string(dev, "label", &pdata->label);
	pdata->rep = device_property_read_bool(dev, "autorepeat");

	device_for_each_child_node(dev, child){
		fwnode_property_read_string(child, "label", &button->label);

		button->type = EV_KEY;
		if(fwnode_property_read_u32(child, "code", &button->code)){
			rptips("use default code : 1");
			button->code = 1;
		}
		rptips("code = %u\n", button->code);

		button->wakeup = fwnode_property_read_bool(child, "wakeup");
		rptips("wakeup=%d\n", button->wakeup);

		if(fwnode_property_read_u32(child, "debounce_interval", &button->debounce_interval)){
			button->debounce_interval = 10;
		}
		rptips("debounce interval=%d\n", button->debounce_interval);

		
		if(fwnode_property_read_u32(child, "press_type", &button->press_type)){
			button->press_type = 0;
		}

		button ++;
	}

	return pdata;
}


static int gpio_keys_setup_key(struct platform_device *pdev,
                struct input_dev *input,
                struct gpio_keys_drvdata *ddata,
                const struct gpio_keys_button *button,
                int idx,
                struct fwnode_handle *child){
// setup key
	const char *label = button->label ? button->label : "rp_keys";
	struct device *dev = &pdev->dev;
	struct gpio_button_data *bdata = &ddata->data[idx];
	irq_handler_t isr;
	unsigned long irqflags;
	int gpio = -1, bank = -1, group = -1;
	int irq;
	int error;
	bool active_low;
	char gpioname[10];

	bdata->input = input;
	bdata->button = button;
	
	bdata->gpiod = 
		devm_fwnode_get_gpiod_from_child(dev, NULL, child, GPIOD_IN, label);
	if(IS_ERR(bdata->gpiod)){
		rptips("failed to get gpio, errnum:%ld\n", PTR_ERR(bdata->gpiod));
		return PTR_ERR(bdata->gpiod);
	}
	gpio = desc_to_gpio(bdata->gpiod);
	
	group = gpio / 32;
	bank = (gpio - (group * 32)) / 8;
	sprintf(gpioname, "GPIO%d%c%d", bank, 'A' + bank, gpio - group * 32 - bank * 8);

	rptips("gpio %d : %s\n", gpio, gpioname);

	active_low = gpiod_is_active_low(bdata->gpiod);
	rptips("active low : %d\n", active_low);
	
	irq = gpiod_to_irq(bdata->gpiod);
	if(irq < 0){
		rptips("err get irq for gpio %s\n", gpioname);
		return irq;
	}
	bdata->irq = irq;
	rptips("irq %d\n attach %s\n", irq, gpioname);

	INIT_DELAYED_WORK(&bdata->work, gpio_keys_gpio_work_func);
	
	bdata->press = 0;


	bdata->code = &ddata->keymap[idx];
	*bdata->code = button->code;
	input_set_capability(input, EV_KEY, *bdata->code);

	isr = gpio_keys_gpio_isr;
	irqflags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING;
	error = devm_request_any_context_irq(dev, bdata->irq, isr, irqflags, label, bdata);
	if(error < 0) {
		rptips("request irq %d failed\n", bdata->irq);
		return error;
	}
	
	return 0;
}

static int gpio_keys_probe(struct platform_device *pdev){
// probe
	struct device *dev = &pdev->dev;
	const struct gpio_keys_platform_data *pdata; 
	struct fwnode_handle *child = NULL;
	struct gpio_keys_drvdata *ddata;
	struct input_dev *input;
	size_t size;
	int i, error, wakeup = 0;
	
	pdata = gpio_keys_get_data_from_devtree(dev);
	if(IS_ERR(pdata))
		return PTR_ERR(pdata);

	size = sizeof(struct gpio_keys_drvdata) + 
			pdata->nbuttons * sizeof(struct gpio_button_data);
	ddata = devm_kzalloc(dev, size, GFP_KERNEL);
	if(!ddata) {
		rptips("failed to allocate ddata\n");
		return -ENOMEM;
	}

	ddata->keymap = 
		devm_kcalloc(dev, pdata->nbuttons, sizeof(ddata->keymap[0]), GFP_KERNEL);
	if(!ddata->keymap)
		return -ENOMEM;

	input = devm_input_allocate_device(dev);
	if(!input) {
		rptips("failed to allocate input dev\n");
		return -ENOMEM;
	}

	ddata->pdata = pdata;
	ddata->input = input;
	
	input->name = pdev->name;
	input->dev.parent = dev;


	input->keycode = ddata->keymap;
	input->keycodesize = sizeof(ddata->keymap[0]);
	input->keycodemax = pdata->nbuttons;

	if(pdata->rep)
		__set_bit(EV_REP, input->evbit);

	for(i = 0; i < pdata->nbuttons; i ++) {
		const struct gpio_keys_button *button = &pdata->buttons[i];

		child = device_get_next_child_node(dev, child);
		if(!child) {
			rptips("no child device node\n");
			return -EINVAL;
		}

		error = gpio_keys_setup_key(pdev, input, ddata, button, i, child);
		if(error) {
			fwnode_handle_put(child);
			return error;
		}

		if(button->wakeup)
			wakeup = 1;
	}
	fwnode_handle_put(child);

	error = input_register_device(input);
	if(error) {
		rptips("unable to register input dev\n");
		return error;
	}

	platform_set_drvdata(pdev, ddata);
        input_set_drvdata(input, ddata);

	if(wakeup){
		error = device_init_wakeup(dev, wakeup);
		rptips("init wakeup,ret = %d\n", error);
		// gpio_keys_enable_wakeup(ddata);
	}

	return 0;
}


static int
gpio_keys_button_enable_wakeup(struct gpio_button_data *bdata)
{
    int error;

    error = enable_irq_wake(bdata->irq);
    if (error) {
        rptips("failed setup wakeup source IRQ: %d by err: %d\n",
            bdata->irq, error);
        return error;
    }

    error = irq_set_irq_type(bdata->irq, IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING);
    if (error) {
        rptips("failed to set wakeup trigger for IRQ %d: %d\n", bdata->irq, error);
        disable_irq_wake(bdata->irq);
        return error;
    }

    return 0;
}

static int 
gpio_keys_enable_wakeup(struct gpio_keys_drvdata *ddata)
{
    struct gpio_button_data *bdata;
    int error;
    int i;

    for (i = 0; i < ddata->pdata->nbuttons; i++) {
        bdata = &ddata->data[i];
        if (bdata->button->wakeup) {
            error = gpio_keys_button_enable_wakeup(bdata);
            if (error)
		return error;
        }
    }

    return 0;

}

static void __maybe_unused
gpio_keys_button_disable_wakeup(struct gpio_button_data *bdata)
{
        int error;

        error = disable_irq_wake(bdata->irq);
        if (error)
                rptips("failed to disable wakeup src IRQ %d: %d\n", bdata->irq, error);
}

static void __maybe_unused
gpio_keys_disable_wakeup(struct gpio_keys_drvdata *ddata)
{
        struct gpio_button_data *bdata;
        int i;

        for (i = 0; i < ddata->pdata->nbuttons; i++) {
                bdata = &ddata->data[i];
                if (irqd_is_wakeup_set(irq_get_irq_data(bdata->irq)))
                        gpio_keys_button_disable_wakeup(bdata);
        }
}

static int __maybe_unused gpio_keys_suspend(struct device *dev)
{
        struct gpio_keys_drvdata *ddata = dev_get_drvdata(dev);
        int error;
	
        if (device_may_wakeup(dev)) {
                error = gpio_keys_enable_wakeup(ddata);
                if (error)
                        return error;
        }
        return 0;
}

static int __maybe_unused gpio_keys_resume(struct device *dev)
{
	struct gpio_keys_drvdata *ddata = dev_get_drvdata(dev);

        if (device_may_wakeup(dev)) {
                gpio_keys_disable_wakeup(ddata);
	}

        return 0;
}

static SIMPLE_DEV_PM_OPS(gpio_keys_pm_ops, gpio_keys_suspend, gpio_keys_resume);

// device match table
static const struct of_device_id gpio_keys_of_match[] = {
    { .compatible = "rp-keys", },
    { },
};

MODULE_DEVICE_TABLE(of, gpio_keys_of_match);

// driver descrition
static struct platform_driver gpio_keys_device_driver = {
    .probe      = gpio_keys_probe,
    .driver     = {
        .name   = "rp-keys",
        .of_match_table = gpio_keys_of_match,
	.pm = &gpio_keys_pm_ops,

    }
};

static int __init gpio_keys_init(void)
{
    return platform_driver_register(&gpio_keys_device_driver);
}

static void __exit gpio_keys_exit(void)
{
    platform_driver_unregister(&gpio_keys_device_driver);
}

late_initcall_sync(gpio_keys_init);
module_exit(gpio_keys_exit);

MODULE_LICENSE("GPL");

