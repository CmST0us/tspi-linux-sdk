## RK356X/RK3588/RV1103/RV1106 RKNN SDK 1.4.0

### 主要修改

#### RKNN Compiler & Runtime v1.4.0:
- 新增权重压缩功能支持；（RK3588/RV1106），减少内存消耗
- RK3588新增sram功能支持，减少系统DDR负载
- RK3588新增单batch多核支持；
- 新增NPU直接输出NHWC layout的支持；
- 新增Reshape、Transpose、MatMul、 Max、Min、exGelu、exSoftmax13等op部分shape下的npu支持；
- 新增rknn_api weight 权重共享的功能；
- 完善对非4维输入支持

#### RKNN-Toolkit2 1.4.0

- 更新对 pytorch 1.10.2 版本的支持
- 更新对 tensorflow 2.6.2 版本的支持
- 升级相关依赖包到主流版本 (如onnx==1.9.0, onnxruntime==1.10.0等)
- 添加对  If / SplitToSequence / SequenceAt / Gelu / HardSwish / group_norm 等OP的支持
- 添加 2/3 维 OP 的支持
- 添加 config.remove_weight 的功能（用于 runtime 共享 weight 权重）
- 更新 onnx_optimzie 接口功能，支持导入自定义量化参数
- 优化 simulator 的内存占用，降低转换/推理模型的内存消耗，并提高其性能

###  版本号查询

- librknnrt runtime版本：1.4.0（strings librknnrt.so | grep version | grep lib）
- rknpu driver版本：0.8.2（dmesg | grep rknpu）

### 其他说明

- rknn-toolkit适用RV1109/RV1126/RK1808/RK3399Pro，rknn-toolkit2适用RK356X/RK3588/RV1103/RV1106
- rknn-toolkit2与rknn-toolkit API接口基本保持一致，用户不需要太多修改（rknn.config()部分参数有删减）
- rknpu2需要与rknn-toolkit2同步升级到1.4.0的版本。之前客户使用rknn toolkit2 1.3.0版本生成的rknn模型建议重新生成
- rknn api里面部分demo依赖MPI MMZ/RGA，使用时，需要和系统中相应的库匹配 
- 本次发布也支持RV1103/RV1106

