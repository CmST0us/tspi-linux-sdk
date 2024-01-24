下面以Intel的I350网卡为例：（对于其它厂家的PCIE网卡，需要按照下面第6点的说明，让厂家修改符合我们平台要求的驱动）

1. kernel defconfig打开如下配置：
+CONFIG_UIO=m
+CONFIG_HUGETLBFS=y

2. kernel代码修改如下：
diff --git a/include/linux/uio_driver.h b/include/linux/uio_driver.h
index 77131e8fefcc1..0a70d70bed6df 100644
--- a/include/linux/uio_driver.h
+++ b/include/linux/uio_driver.h
@@ -45,7 +45,7 @@ struct uio_mem {
 	struct uio_map		*map;
 };
 
-#define MAX_UIO_MAPS	5
+#define MAX_UIO_MAPS	13
 
 struct uio_portio;
 
@@ -65,7 +65,7 @@ struct uio_port {
 	struct uio_portio	*portio;
 };
 
-#define MAX_UIO_PORT_REGIONS	5
+#define MAX_UIO_PORT_REGIONS	13
 
 struct uio_device {
 	struct module           *owner;

3.把补丁包的igb_uio驱动，用对应的内核编译出一个igb_uio.ko
+obj-m				+= igb_uio/

4.i350网卡使用drivers/net/e1000/驱动
用补丁包里面e1000/目录的两个文件 *对比* 开源代码进行修改合并

5. 附相关命令及工具：
#驱动(下面两个ko都从实际使用的内核编译)
insmod uio.ko
insmod igb_uio.ko
#开启性能模式（命令报错忽略）
echo performance | tee $(find /sys/ -name *governor) /dev/null || true
#开启hugepages
echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages

#绑定网卡 （0000:01:00.X要改成实际的）
dpdk/usertools/dpdk-devbind.py -b igb_uio 0000:01:00.0
dpdk/usertools/dpdk-devbind.py -b igb_uio 0000:01:00.1
dpdk/usertools/dpdk-devbind.py -b igb_uio 0000:01:00.2
dpdk/usertools/dpdk-devbind.py -b igb_uio 0000:01:00.3
... ...

#测试工具（DPDK可以直接从官网下载编译）
dpdk/build/app/dpdk-testpmd
dpdk/examples/l3fwd/build/l3fwd

6. RK平台特殊说明
RK主控没有*硬件*支持网卡DMA访问内存一致性，所以开源DPDK代码网卡驱动使用的API：rte_eth_dma_zone_reserve/rte_mbuf_raw_alloc它是默认要求硬件访问内存一致性的；
比如发送数据的场景：CPU把数据写到内存，然后通知网卡DMA来搬运数据，其它平台硬件会自动刷新cache，使的网卡DMA能直接拿到最新数据，而RK平台需要手动取刷新；
DPDK内存主要有两种：
一个是给网卡BD描述符使用的内存，使用rte_eth_dma_zone_reserve来分配，由于它会被频繁使用，所以解决策略是在内核使用dma_alloc_coherent分配非cache的内存，然后映射给dpdk的网卡驱动使用；
二是网卡存放数据的内存，比如用rte_mbuf_raw_alloc分配，由于内存分配量比较大，所以直接使用arm标准的指令刷cache的命令来实现，比如写发送数据时，写完数据后主动刷新这块内存，让实际的内存写到DDR里面取，此时DMA就能拿到实际写入的数据；
！！！注意：如果要支持其它型号的网卡，请按照上述的要求让网卡厂商去修改他们的驱动即可。

