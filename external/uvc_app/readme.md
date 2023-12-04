### 操作流程
- 先配置uvc video节点
1. 配置uvc功能：运行uvc_MJPEG.sh
2. 运行: uvc_app 640 480 &
3. 打开AMCAP即可预览，uvc_app输出四条纯色

- 通过uevent来监听uvc video节点添加
1. 运行: uvc_app 640 480 &
2. 配置uvc功能：运行uvc_MJPEG.sh
3. 打开AMCAP即可预览，uvc_app输出四条纯色

### 接口说明
1. mpi_enc_set_format：设置MJPG编码输入源格式，没设置默认为NV12
2. uvc_read_camera_buffer：读取buffer后用于编码传输, 外部模块可以通过注册callback的方式实现数据传输
3. uvc_control_run：uevent的初始化，监听video添加，uvc的初始化等统一在这个函数实现。
4. uvc_control_join：uvc反初始化退出。