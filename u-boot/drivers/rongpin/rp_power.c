/*
* (C) Copyright 2008-2017 Fuzhou Rockchip Electronics Co., Ltd
*
* SPDX-License-Identifier:    GPL-2.0+
*/


#include <config.h>
#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <video.h>
#include <backlight.h>
#include <asm/gpio.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dm/uclass.h>
#include <dm/uclass-id.h>
#include <asm/io.h>

#define GPIO_FUNCTION_OUTPUT 0
#define GPIO_FUNCTION_INPUT 1
#define GPIO_FUNCTION_IRQ 2
#define GPIO_FUNCTION_FLASH 3
#define GPIO_FUNCTION_OUTPUT_CTRL 4


struct rp_power_priv {
    struct gpio_desc rp_power_gpio;
    u32 gpio_flag;
    u32 gpio_function;
};


static struct rp_power_priv gpio_priv;


static int rp_power_probe(struct udevice *dev)
{
    int ret;
    bool has_children, has_ofnode;
    ofnode rp_power_node, child_node;
    rp_power_node = dev->node; //udevice 结构体中可以获取到dts节点
    const char* node_name;
        
    has_children = device_has_children(dev);
    if (has_children)
        printf("rp_power have children !\n");
    else{
        printf("rp_power not have children !\n");
    }
    
    has_ofnode = dev_has_of_node(dev); //判断是否有节点
    if (has_ofnode)
        printf("rp_power have node! \n");
    else{
        printf("rp_power not have node !\n");
        return 0;
    }
    
    ofnode_for_each_subnode(child_node, rp_power_node){ //ofnode_for_each_subnode 宏 可以遍历node 节点
        node_name = ofnode_get_name(child_node);
        ret = ofnode_read_u32(child_node, "gpio_function", &gpio_priv.gpio_function);
        if(ret){
            printf("rp_power: %s gpio function get error, set gpio default dir to output\n", node_name);
            gpio_priv.gpio_function = GPIOD_IS_OUT;
        }
        ret = gpio_request_by_name_nodev(child_node, "gpio_num", 0, &gpio_priv.rp_power_gpio, gpio_priv.gpio_flag);
        if (ret){
            printf("rp_power: %s gpio get error\n", node_name);
        }else{
            if (dm_gpio_is_valid(&gpio_priv.rp_power_gpio)){
                
                switch(gpio_priv.gpio_function){
                    case GPIO_FUNCTION_OUTPUT:
                    case GPIO_FUNCTION_OUTPUT_CTRL:
                    case GPIOD_IS_OUT:
                        dm_gpio_set_dir_flags(&gpio_priv.rp_power_gpio, GPIOD_IS_OUT); //设置GPIO方向
                        
                        //GPIO_ACTIVE_HIGH: 0
                        //GPIO_ACTIVE_LOW: 1
                        dm_gpio_set_value(&gpio_priv.rp_power_gpio, !gpio_priv.gpio_flag);
                        
                        printf("rp_power: %s gpio set output\n", node_name);
                        break;
                        
                    default:
                        printf("rp_power: %s gpio function is %d, not realize!\n", node_name, gpio_priv.gpio_function);
                        break;
                }
                
            }else{
                printf("rp_power: %s gpio is not valid\n", node_name);
            }
        }
    }
    return 0;
}


static const struct udevice_id rp_power_ids[] = {
    {
        .compatible = "rp_power" //和 dts 的 compatible 对应
    }
};


U_BOOT_DRIVER(rp_power) = {
    .name = "rp_power",
    .id = UCLASS_GPIO,
    .of_match = rp_power_ids,
    .probe = rp_power_probe,
};
