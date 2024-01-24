
kernel补丁：
包括kernel 4.19/5.10内核版本，根据实际版本打补丁

DPDK补丁：
stmmac.rk.tar.bz2 //补丁基于21.11版本，放到DPDK源码包的driver/net/目录下
drivers/net/meson.build
        'qede',
        'ring',
        'sfc',
        'softnic',
+       'stmmac',
        'tap',
        'thunderx',
        'txgbe',
        'vdev_netvsc',
        'vhost',
        'virtio',
        'vmxnet3',

		
其它参考文档