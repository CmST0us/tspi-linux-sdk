#include <stdio.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdint.h>
#include <cstdlib>
#include <RTTaskNodeContext.h>
#include <RTTaskNode.h>
#include <RTMediaBuffer.h>

#define kStubRockitDemo  MKTAG('r', 'k', 'd', 'm')

// 创建外部节点， 外部节点需要继承RTTaskNode
// 基础接口需要完成open/process/close
class RTRockitDemoNode : public RTTaskNode {
 public:
    RTRockitDemoNode() {}
    virtual ~RTRockitDemoNode() {}

    virtual RT_RET open(RTTaskNodeContext *context) { return RT_OK; }
    virtual RT_RET process(RTTaskNodeContext *context);
    virtual RT_RET close(RTTaskNodeContext *context) { return RT_OK; }

 private:
    RT_RET doAIProcess(RTMediaBuffer *src, RTMediaBuffer *dst);
};

// 用于节点创建， 该函数指针将存于RTNodeStub.mCreateObj中
static RTTaskNode* createRockitDemoNode() {
    return new RTRockitDemoNode();
}

RT_RET RTRockitDemoNode::doAIProcess(RTMediaBuffer *src, RTMediaBuffer *dst) {
    return RT_OK;
}

// 节点处理函数，将输入的数据处理， 然后输出到下级节点
RT_RET RTRockitDemoNode::process(RTTaskNodeContext *context) {
    RTMediaBuffer *inputBuffer  = RT_NULL;
    RTMediaBuffer *outputBuffer = RT_NULL;

    // 判断输入是否为空
    while (!context->inputIsEmpty()) {
        // 取出输入的buffer
        inputBuffer = context->dequeInputBuffer();
        // 取出一块未被使用的输出Buffer
        outputBuffer = context->dequeOutputBuffer(RT_TRUE, inputBuffer->getLength());
        // 执行处理函数
        doAIProcess(inputBuffer, outputBuffer);
        // 设置输出buffer的范围
        outputBuffer->setRange(0, inputBuffer->getLength());
        // 标记EOS
        outputBuffer->getMetaData()->setInt32(kKeyFrameEOS, 1);
        // 输入Buffer使用完成，调用释放
        inputBuffer->release();
        // 将输出buffer带出，完成处理流程
        context->queueOutputBuffer(outputBuffer);
    }

    return RT_OK;
}

//节点信息存根， 用于完成节点注册
RTNodeStub node_stub_rockit_demo {
    // 节点uid， 节点的唯一标识符 (0~1000)
    .mUid          = kStubRockitDemo,
    // 节点名， 主要用于节点查找、创建
    // corp_role_name，命名保证唯一
    .mName         = "rockit_demo",
    // 版本号
    .mVersion      = "v1.0",
    // 节点创建方法; 改成宏定义
    .mCreateObj    = createRockitDemoNode,
    .mCapsSrc      = { "video/x-raw", RT_PAD_SRC, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL} },
    .mCapsSink     = { "video/x-raw", RT_PAD_SINK, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL} },
};

RT_NODE_FACTORY_REGISTER_STUB(node_stub_rockit_demo);
