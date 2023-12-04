#include "rkaiq_protocol.h"

#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <regex>

#ifdef __ANDROID__
    #include <cutils/properties.h>
    #include <rtspserver/RtspServer.h>
#endif

#include "domain_tcp_client.h"
#include "tcp_server.h"
#include "tcp_client.h"
#include "rkaiq_socket.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

extern int g_app_run_mode;
extern int g_width;
extern int g_height;
extern int g_rtsp_en;
extern int g_rtsp_en_from_cmdarg;
extern int g_device_id;
extern int g_allow_killapp;
extern uint32_t g_offlineFrameRate;
extern DomainTCPClient g_tcpClient;
extern struct ucred* g_aiqCred;
extern std::string iqfile;
extern std::string g_sensor_name;
extern std::shared_ptr<RKAiqMedia> rkaiq_media;
extern std::string g_stream_dev_name;
extern std::shared_ptr<TCPServer> tcpServer;
extern int ConnectAiq();

bool RKAiqProtocol::is_recv_running = false;
std::unique_ptr<std::thread> RKAiqProtocol::forward_thread = nullptr;
std::unique_ptr<std::thread> RKAiqProtocol::offlineRawThread = nullptr;
std::mutex RKAiqProtocol::mutex_;
static int startOfflineRawFlag = 0;

#define MAX_PACKET_SIZE 8192
#pragma pack(1)
typedef struct FileTransferData_s {
    uint8_t RKID[8]; // "SendFile"
    unsigned long long packetSize;
    int commandID;
    int commandResult;
    int targetDirLen;
    uint8_t targetDir[256];
    int targetFileNameLen;
    uint8_t targetFileName[128];
    unsigned long long dataSize;
    char* data;
    unsigned int dataHash;
} FileTransferData;

typedef struct OfflineRAW_s {
    uint8_t RKID[8]; // "OffRAW"
    unsigned long long packetSize;
    int commandID;
    int commandResult;
    int offlineRawModeControl;
} OfflineRAW;

#pragma pack()

static void HexDump(unsigned char* data, size_t size)
{
    printf("####\n");
    int i;
    size_t offset = 0;
    while (offset < size) {
        printf("%04x  ", offset);
        for (i = 0; i < 16; i++) {
            if (i % 8 == 0) {
                putchar(' ');
            }
            if (offset + i < size) {
                printf("%02x ", data[offset + i]);
            } else {
                printf("   ");
            }
        }
        printf("   ");
        for (i = 0; i < 16 && offset + i < size; i++) {
            if (isprint(data[offset + i])) {
                printf("%c", data[offset + i]);
            } else {
                putchar('.');
            }
        }
        putchar('\n');
        offset += 16;
    }
    printf("####\n\n");
}

static int ProcessExists(const char* process_name)
{
    FILE* fp;
    char cmd[1024] = {0};
    char buf[1024] = {0};
    snprintf(cmd, sizeof(cmd), "ps -ef | grep %s | grep -v grep", process_name);
    fp = popen(cmd, "r");
    if (!fp) {
        LOG_DEBUG("popen ps | grep %s fail\n", process_name);
        return -1;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        LOG_DEBUG("ProcessExists %s\n", buf);
        if (strstr(buf, process_name)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int StopProcess(const char* process, const char* str)
{
    int count = 0;
    while (ProcessExists(process) > 0) {
        LOG_DEBUG("StopProcess %s... \n", process);
        system(str);
        sleep(1);
        count++;
        if (count > 3) {
            return -1;
        }
    }
    return 0;
}

int WaitProcessExit(const char* process, int sec)
{
    int count = 0;
    LOG_DEBUG("WaitProcessExit %s... \n", process);
    while (ProcessExists(process) > 0) {
        LOG_DEBUG("WaitProcessExit %s... \n", process);
        sleep(1);
        count++;
        if (count > sec) {
            return -1;
        }
    }
    return 0;
}

void RKAiqProtocol::KillApp()
{
#ifdef __ANDROID__
    if (g_allow_killapp) {
        unlink(LOCAL_SOCKET_PATH);
        property_set("ctrl.stop", "cameraserver");
        property_set("ctrl.stop", "vendor.camera-provider-2-4");
        property_set("ctrl.stop", "vendor.camera-provider-2-4-ext");
        system("stop cameraserver");
        system("stop vendor.camera-provider-2-4");
        system("stop vendor.camera-provider-2-4-ext");
    }
#else
    if (g_allow_killapp) {
        if (g_aiqCred != nullptr) {
            kill(g_aiqCred->pid, SIGTERM);
            delete g_aiqCred;
            g_aiqCred = nullptr;
        }
    }
#endif
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int RKAiqProtocol::StartApp()
{
    int ret = -1;
#ifdef __ANDROID__
    if (g_allow_killapp) {
        property_set("ctrl.start", "cameraserver");
        system("start cameraserver");
        system("start vendor.camera-provider-2-4");
        system("start vendor.camera-provider-2-4-ext");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
#endif
    return 0;
}

int RKAiqProtocol::StartRTSP()
{
    int ret = -1;

    LOG_DEBUG("Starting RTSP !!!");
    KillApp();
    ret = rkaiq_media->LinkToIsp(true);
    if (ret) {
        LOG_ERROR("link isp failed!!!\n");
        return ret;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    media_info_t mi = rkaiq_media->GetMediaInfoT(g_device_id);
#ifdef __ANDROID__
    int readback = 0;
    std::string isp3a_server_cmd = "/vendor/bin/rkaiq_3A_server -mmedia=";
    if (mi.isp.media_dev_path.length() > 0) {
        isp3a_server_cmd.append(mi.isp.media_dev_path);
        LOG_DEBUG("#### using isp dev path.\n");
    } else if (mi.cif.media_dev_path.length() > 0) {
        isp3a_server_cmd.append(mi.cif.media_dev_path);
        LOG_DEBUG("#### using cif dev path.\n");
    } else {
        isp3a_server_cmd.append("/dev/media2");
        LOG_DEBUG("#### using default dev path.\n");
    }
    isp3a_server_cmd.append(" --sensor_index=");
    isp3a_server_cmd.append(std::to_string(g_device_id));
    isp3a_server_cmd.append(" &");
    system("pkill rkaiq_3A_server*");
    system(isp3a_server_cmd.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
#endif
    if (g_stream_dev_name.empty()) {
        int isp_ver = rkaiq_media->GetIspVer();
        LOG_DEBUG(">>>>>>>> isp ver = %d\n", isp_ver);
        if (isp_ver == 4) {
            ret = init_rtsp(mi.ispp.pp_scale0_path.c_str(), g_width, g_height);
        } else if (isp_ver == 5) {
            ret = init_rtsp(mi.isp.main_path.c_str(), g_width, g_height);
        } else {
            ret = init_rtsp(mi.isp.main_path.c_str(), g_width, g_height);
        }
    } else {
        ret = init_rtsp(g_stream_dev_name.c_str(), g_width, g_height);
    }
    if (ret) {
        LOG_ERROR("init_rtsp failed!!");
        return ret;
    }

    LOG_DEBUG("Started RTSP !!!");
    return 0;
}

int RKAiqProtocol::StopRTSP()
{
    LOG_DEBUG("Stopping RTSP !!!");
    deinit_rtsp();
#ifdef __ANDROID__
    system("pkill rkaiq_3A_server*");
#endif
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    LOG_DEBUG("Stopped RTSP !!!");
    return 0;
}

int RKAiqProtocol::DoChangeAppMode(appRunStatus mode)
{
    std::lock_guard<std::mutex> lg(mutex_);
    int ret = -1;
    LOG_DEBUG("Switch to mode %d->%d\n", g_app_run_mode, mode);
    if (g_app_run_mode == mode) {
        return 0;
    }
    if (mode == APP_RUN_STATUS_CAPTURE) {
        LOG_DEBUG("Switch to APP_RUN_STATUS_CAPTURE\n");
        if (g_rtsp_en) {
            ret = StopRTSP();
            if (ret) {
                LOG_ERROR("stop RTSP failed!!!\n");
                g_app_run_mode = APP_RUN_STATUS_INIT;
                return ret;
            }
        }
        KillApp();
        ret = rkaiq_media->LinkToIsp(false);
        if (ret) {
            LOG_ERROR("unlink isp failed!!!\n");
            g_app_run_mode = APP_RUN_STATUS_INIT;
            return ret;
        }
    } else {
        LOG_DEBUG("Switch to APP_RUN_STATUS_TUNRING\n");
        ret = rkaiq_media->LinkToIsp(true);
        if (ret) {
            LOG_ERROR("link isp failed!!!\n");
            g_app_run_mode = APP_RUN_STATUS_INIT;
            // return ret;
        }

        if (!g_rtsp_en) {
            ret = StartApp();
            if (ret) {
                LOG_ERROR("start app failed!!!\n");
                g_app_run_mode = APP_RUN_STATUS_INIT;
                return ret;
            }
        }
    }
    g_app_run_mode = mode;
    LOG_DEBUG("Change mode to %d exit\n", g_app_run_mode);
    return 0;
}

static void InitCommandPingAns(CommandData_t* cmd, int ret_status)
{
    strncpy((char*)cmd->RKID, RKID_CHECK, sizeof(cmd->RKID));
    cmd->cmdType = DEVICE_TO_PC;
    cmd->cmdID = CMD_ID_CAPTURE_STATUS;
    cmd->datLen = 1;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++) {
        cmd->checkSum += cmd->dat[i];
    }
}

static void DoAnswer(int sockfd, CommandData_t* cmd, int cmd_id, int ret_status)
{
    char send_data[MAXPACKETSIZE];
    LOG_DEBUG("enter\n");

    strncpy((char*)cmd->RKID, TAG_OL_DEVICE_TO_PC, sizeof(cmd->RKID));
    cmd->cmdType = DEVICE_TO_PC;
    cmd->cmdID = cmd_id;
    strncpy((char*)cmd->version, RKAIQ_TOOL_VERSION, sizeof(cmd->version));
    cmd->datLen = 4;
    memset(cmd->dat, 0, sizeof(cmd->dat));
    cmd->dat[0] = ret_status;
    cmd->checkSum = 0;
    for (int i = 0; i < cmd->datLen; i++) {
        cmd->checkSum += cmd->dat[i];
    }

    memcpy(send_data, cmd, sizeof(CommandData_t));
    send(sockfd, send_data, sizeof(CommandData_t), 0);
    LOG_DEBUG("exit\n");
}

void RKAiqProtocol::HandlerCheckDevice(int sockfd, char* buffer, int size)
{
    CommandData_t* common_cmd = (CommandData_t*)buffer;
    CommandData_t send_cmd;
    char send_data[MAXPACKETSIZE];
    int ret = -1;

    LOG_DEBUG("HandlerCheckDevice:\n");

    // for (int i = 0; i < common_cmd->datLen; i++) {
    //   LOG_DEBUG("DATA[%d]: 0x%x\n", i, common_cmd->dat[i]);
    // }

    if (strcmp((char*)common_cmd->RKID, RKID_CHECK) == 0) {
        LOG_DEBUG("RKID: %s\n", common_cmd->RKID);
    } else {
        LOG_DEBUG("RKID: Unknow\n");
        return;
    }

    LOG_DEBUG("cmdID: %d\n", common_cmd->cmdID);

    switch (common_cmd->cmdID) {
        case CMD_ID_CAPTURE_STATUS:
            LOG_DEBUG("CmdID CMD_ID_CAPTURE_STATUS in\n");
            if (common_cmd->dat[0] == KNOCK_KNOCK) {
                InitCommandPingAns(&send_cmd, READY);
                LOG_DEBUG("Device is READY\n");
            } else {
                LOG_ERROR("Unknow CMD_ID_CAPTURE_STATUS message\n");
            }
            memcpy(send_data, &send_cmd, sizeof(CommandData_t));
            send(sockfd, send_data, sizeof(CommandData_t), 0);
            LOG_DEBUG("cmdID CMD_ID_CAPTURE_STATUS out\n\n");
            break;
        case CMD_ID_GET_STATUS:
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, READY);
            break;
        case CMD_ID_GET_MODE:
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, g_app_run_mode);
            break;
        case CMD_ID_START_RTSP:
            if (g_rtsp_en_from_cmdarg == 1) {
                g_rtsp_en = 1;
            }
            ret = StartRTSP();
            if (ret) {
                LOG_ERROR("start RTSP failed!!!\n");
            }
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, g_app_run_mode);
            break;
        case CMD_ID_STOP_RTSP:
            if (g_rtsp_en_from_cmdarg == 1) {
                g_rtsp_en = 0;
            }
            ret = StopRTSP();
            if (ret) {
                LOG_ERROR("stop RTSP failed!!!\n");
            }
            g_app_run_mode = APP_RUN_STATUS_INIT;
            DoAnswer(sockfd, &send_cmd, common_cmd->cmdID, g_app_run_mode);
            break;
        default:
            break;
    }
}

void RKAiqProtocol::HandlerReceiveFile(int sockfd, char* buffer, int size)
{
    FileTransferData* recData = (FileTransferData*)buffer;
    LOG_DEBUG("HandlerReceiveFile begin\n");
    // HexDump((unsigned char*)buffer, size);
    // parse data
    unsigned long long packetSize = recData->packetSize;
    LOG_DEBUG("FILETRANS receive : sizeof(packetSize):%d\n", sizeof(packetSize));
    // HexDump((unsigned char*)&recData->packetSize, 8);
    LOG_DEBUG("FILETRANS receive : packetSize:%llu\n", packetSize);

    unsigned long long dataSize = recData->dataSize;
    LOG_DEBUG("FILETRANS receive : dataSize:%llu\n", dataSize);
    if (packetSize <= 0 || packetSize - dataSize > 500) {
        printf("FILETRANS no data received or packetSize error, return.\n");
        char tmpBuf[200] = {0};
        snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##TransferError##");
        std::string resultStr = tmpBuf;
        send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
        return;
    }

    char* receivedPacket = (char*)malloc(packetSize);
    memset(receivedPacket, 0, packetSize);
    memcpy(receivedPacket, buffer, size);

    unsigned long long remain_size = packetSize - size;
    int recv_size = 0;

    struct timespec startTime = {0, 0};
    struct timespec currentTime = {0, 0};
    clock_gettime(CLOCK_REALTIME, &startTime);
    LOG_DEBUG("FILETRANS get, start receive:%ld\n", startTime.tv_sec);
    while (remain_size > 0) {
        clock_gettime(CLOCK_REALTIME, &currentTime);
        if (currentTime.tv_sec - startTime.tv_sec >= 20) {
            LOG_DEBUG("FILETRANS receive: receive data timeout, return\n");
            char tmpBuf[200] = {0};
            snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##Timeout##");
            std::string resultStr = tmpBuf;
            send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
            return;
        }

        unsigned long long offset = packetSize - remain_size;

        unsigned long long targetSize = 0;
        if (remain_size > MAX_PACKET_SIZE) {
            targetSize = MAX_PACKET_SIZE;
        } else {
            targetSize = remain_size;
        }
        recv_size = recv(sockfd, &receivedPacket[offset], targetSize, 0);
        remain_size = remain_size - recv_size;

        // LOG_DEBUG("FILETRANS receive,remain_size: %llu\n", remain_size);
    }
    LOG_DEBUG("FILETRANS receive: receive success, need check data\n");

    // HexDump((unsigned char*)receivedPacket, packetSize);
    // Send(receivedPacket, packetSize); //for debug use

    // parse data
    FileTransferData receivedData;
    memset((void*)&receivedData, 0, sizeof(FileTransferData));
    unsigned long long offset = 0;
    // magic
    memcpy(receivedData.RKID, receivedPacket, sizeof(receivedData.RKID));
    // HexDump((unsigned char*)receivedData.RKID, sizeof(receivedData.RKID));
    offset += sizeof(receivedData.RKID);
    // packetSize
    memcpy((void*)&receivedData.packetSize, receivedPacket + offset, sizeof(receivedData.packetSize));
    offset += sizeof(receivedData.packetSize);
    // command id
    memcpy((void*)&(receivedData.commandID), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);
    // command result
    memcpy((void*)&(receivedData.commandResult), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);
    // target dir len
    memcpy((void*)&(receivedData.targetDirLen), receivedPacket + offset, sizeof(receivedData.targetDirLen));
    offset += sizeof(receivedData.targetDirLen);
    LOG_DEBUG("FILETRANS receive: receivedData.targetDirLen:%d\n", receivedData.targetDirLen);
    // target dir
    memcpy((void*)&(receivedData.targetDir), receivedPacket + offset, sizeof(receivedData.targetDir));
    // HexDump((unsigned char*)receivedData.targetDir, sizeof(receivedData.targetDir));
    LOG_DEBUG("FILETRANS receive: receivedData.targetDir:%s\n", receivedData.targetDir);
    offset += sizeof(receivedData.targetDir);
    // target file name len
    memcpy((void*)&(receivedData.targetFileNameLen), receivedPacket + offset, sizeof(receivedData.targetFileNameLen));
    offset += sizeof(receivedData.targetFileNameLen);
    LOG_DEBUG("FILETRANS receive: receivedData.targetFileNameLen:%d\n", receivedData.targetFileNameLen);
    // target file name
    memcpy((void*)&(receivedData.targetFileName), receivedPacket + offset, sizeof(receivedData.targetFileName));
    // HexDump((unsigned char*)receivedData.targetFileName, sizeof(receivedData.targetFileName));
    LOG_DEBUG("FILETRANS receive: receivedData.targetFileName:%s\n", receivedData.targetFileName);
    offset += sizeof(receivedData.targetFileName);

    // data size
    memcpy((void*)&(receivedData.dataSize), receivedPacket + offset, sizeof(unsigned long long));
    LOG_DEBUG("FILETRANS receive: receivedData.dataSize:%u\n", receivedData.dataSize);
    offset += sizeof(unsigned long long);
    // data
    receivedData.data = (char*)malloc(receivedData.dataSize);
    memcpy(receivedData.data, receivedPacket + offset, receivedData.dataSize);
    offset += receivedData.dataSize;
    // data hash
    memcpy((void*)&(receivedData.dataHash), receivedPacket + offset, sizeof(unsigned int));

    if (receivedPacket != NULL) {
        free(receivedPacket);
        receivedPacket = NULL;
    }

    // size check
    if (receivedData.dataSize != dataSize) {
        LOG_DEBUG("FILETRANS receive: receivedData.dataSize != target data size, return\n");
        char tmpBuf[200] = {0};
        snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##DataSizeError##");
        std::string resultStr = tmpBuf;
        send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
        return;
    }

    // hash check
    unsigned int dataHash = MurMurHash(receivedData.data, receivedData.dataSize);
    LOG_DEBUG("FILETRANS receive 2: dataHash calculated:%x\n", dataHash);
    LOG_DEBUG("FILETRANS receive: receivedData.dataHash:%x\n", receivedData.dataHash);

    if (dataHash == receivedData.dataHash) {
        LOG_DEBUG("FILETRANS receive: data hash check pass\n");
    } else {
        LOG_DEBUG("FILETRANS receive: data hash check failed\n");
        char tmpBuf[200] = {0};
        snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##HashCheckFail##");
        std::string resultStr = tmpBuf;
        send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
        return;
    }

    // save to file
    std::string dstDir = (char*)receivedData.targetDir;
    std::string dstFileName = (char*)receivedData.targetFileName;
    std::string dstFilePath = dstDir + "/" + dstFileName;

    DIR* dirPtr = opendir(dstDir.c_str());
    if (dirPtr == NULL) {
        LOG_DEBUG("FILETRANS target dir %s not exist, return \n", dstDir.c_str());
        char tmpBuf[200] = {0};
        snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##DirError##");
        std::string resultStr = tmpBuf;
        send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
        return;
    } else {
        closedir(dirPtr);
    }

    FILE* fWrite = fopen(dstFilePath.c_str(), "w");
    if (fWrite != NULL) {
        fwrite(receivedData.data, receivedData.dataSize, 1, fWrite);
    } else {
        LOG_DEBUG("FILETRANS failed to create file %s, return\n", dstFilePath.c_str());
        char tmpBuf[200] = {0};
        snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##FileSaveError##");
        std::string resultStr = tmpBuf;
        send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
        return;
    }

    fclose(fWrite);
    if (receivedData.data != NULL) {
        free(receivedData.data);
        receivedData.data = NULL;
    }

    LOG_DEBUG("HandlerReceiveFile process finished.\n");
    LOG_INFO("receive file %s finished.\n", dstFilePath.c_str());

    char tmpBuf[200] = {0};
    snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Success##%s##", dstFileName.c_str());
    std::string resultStr = tmpBuf;
    send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
}

void RKAiqProtocol::HandlerOfflineRawProcess(int sockfd, char* buffer, int size)
{
    OfflineRAW* recData = (OfflineRAW*)buffer;
    LOG_DEBUG("HandlerOfflineRawProcess begin\n");
    // HexDump((unsigned char*)buffer, size);
    // parse data
    unsigned long long packetSize = recData->packetSize;
    LOG_DEBUG("receive : sizeof(packetSize):%d\n", sizeof(packetSize));
    // HexDump((unsigned char*)&recData->packetSize, 8);
    LOG_DEBUG("receive : packetSize:%llu\n", packetSize);
    if (packetSize <= 0 || packetSize > 50) {
        printf("no data received or packetSize error, return.\n");
        // char tmpBuf[200] = {0};
        // snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##TransferError##");
        // std::string resultStr = tmpBuf;
        // send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
        return;
    }

    char* receivedPacket = (char*)malloc(packetSize);
    memset(receivedPacket, 0, packetSize);
    memcpy(receivedPacket, buffer, size);

    unsigned long long remain_size = packetSize - size;
    int recv_size = 0;

    struct timespec startTime = {0, 0};
    struct timespec currentTime = {0, 0};
    clock_gettime(CLOCK_REALTIME, &startTime);
    LOG_DEBUG("start receive:%ld\n", startTime.tv_sec);
    while (remain_size > 0) {
        clock_gettime(CLOCK_REALTIME, &currentTime);
        if (currentTime.tv_sec - startTime.tv_sec >= 20) {
            LOG_DEBUG("receive: receive data timeout, return\n");
            // char tmpBuf[200] = {0};
            // snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Failed##Timeout##");
            // std::string resultStr = tmpBuf;
            // send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
            return;
        }

        unsigned long long offset = packetSize - remain_size;

        unsigned long long targetSize = 0;
        if (remain_size > MAX_PACKET_SIZE) {
            targetSize = MAX_PACKET_SIZE;
        } else {
            targetSize = remain_size;
        }
        recv_size = recv(sockfd, &receivedPacket[offset], targetSize, 0);
        remain_size = remain_size - recv_size;

        // LOG_DEBUG("FILETRANS receive,remain_size: %llu\n", remain_size);
    }

    // HexDump((unsigned char*)receivedPacket, packetSize);
    // Send(receivedPacket, packetSize); //for debug use

    // parse data
    OfflineRAW receivedData;
    memset((void*)&receivedData, 0, sizeof(OfflineRAW));
    unsigned long long offset = 0;
    // magic
    memcpy(receivedData.RKID, receivedPacket, sizeof(receivedData.RKID));
    offset += sizeof(receivedData.RKID);
    // packetSize
    memcpy((void*)&receivedData.packetSize, receivedPacket + offset, sizeof(receivedData.packetSize));
    offset += sizeof(receivedData.packetSize);
    // command id
    memcpy((void*)&(receivedData.commandID), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);
    // command result
    memcpy((void*)&(receivedData.commandResult), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);
    // mode control
    memcpy((void*)&(receivedData.offlineRawModeControl), receivedPacket + offset, sizeof(int));
    offset += sizeof(int);

    // start process offline process
    if (receivedData.offlineRawModeControl == 1) // start
    {
        LOG_INFO("#### start offline RAW mode. ####\n");
        forward_thread = std::unique_ptr<std::thread>(new std::thread(&RKAiqProtocol::offlineRawProcess));
        forward_thread->detach();
        LOG_INFO("#### offline RAW mode stopped. ####\n");
    } else if (receivedData.offlineRawModeControl == 0) // stop
    {
        startOfflineRawFlag = 0;
    } else if (receivedData.offlineRawModeControl == 2) // remove ini file
    {
        LOG_DEBUG("#### remove offline RAW config file. ####\n");
        system("rm -f /tmp/aiq_offline.ini && sync");
    }
    LOG_DEBUG("HandlerOfflineRawProcess process finished.\n");

    // char tmpBuf[200] = {0};
    // snprintf(tmpBuf, sizeof(tmpBuf), "##StatusMessage##FileTransfer##Success##%s##", dstFileName.c_str());
    // std::string resultStr = tmpBuf;
    // send(sockfd, (char*)resultStr.c_str(), resultStr.length(), 0);
}

static void ExecuteCMD(const char* cmd, char* result)
{
    char buf_ps[2048];
    char ps[2048] = {0};
    FILE* ptr;
    strcpy(ps, cmd);
    if ((ptr = popen(ps, "r")) != NULL) {
        while (fgets(buf_ps, 2048, ptr) != NULL) {
            strcat(result, buf_ps);
            if (strlen(result) > 2048) {
                break;
            }
        }
        pclose(ptr);
        ptr = NULL;
    } else {
        printf("popen %s error\n", ps);
    }
}

void RKAiqProtocol::HandlerTCPMessage(int sockfd, char* buffer, int size)
{
    CommandData_t* common_cmd = (CommandData_t*)buffer;
    LOG_DEBUG("HandlerTCPMessage:\n");
    LOG_DEBUG("HandlerTCPMessage CommandData_t: 0x%lx\n", sizeof(CommandData_t));
    LOG_DEBUG("HandlerTCPMessage RKID: %s\n", (char*)common_cmd->RKID);

    int resetThreadFlag = 1;
    // TODO Check APP Mode
    if (strcmp((char*)common_cmd->RKID, TAG_PC_TO_DEVICE) == 0) {
        char result[2048] = {0};
        std::string pattern{"Isp online"};
        std::regex re(pattern);
        std::smatch results;
        ExecuteCMD("cat /proc/rkisp0-vir0", result);
        std::string srcStr = result;
        // LOG_INFO("#### srcStr:%s\n", srcStr.c_str());
        if (std::regex_search(srcStr, results, re)) // finded
        {
            LOG_INFO("Isp online, please use online raw capture.\n");
            return;
        }
        RKAiqRawProtocol::HandlerRawCapMessage(sockfd, buffer, size);
    } else if (strcmp((char*)common_cmd->RKID, TAG_OL_PC_TO_DEVICE) == 0) {
        RKAiqOLProtocol::HandlerOnLineMessage(sockfd, buffer, size);
    } else if (strcmp((char*)common_cmd->RKID, RKID_CHECK) == 0) {
        HandlerCheckDevice(sockfd, buffer, size);
    } else if (memcmp((char*)common_cmd->RKID, RKID_SEND_FILE, 8) == 0) {
        HandlerReceiveFile(sockfd, buffer, size);
    } else if (memcmp((char*)common_cmd->RKID, RKID_OFFLINE_RAW, 6) == 0) {
        HandlerOfflineRawProcess(sockfd, buffer, size);
    } else {
        resetThreadFlag = 0;
        if (!DoChangeAppMode(APP_RUN_STATUS_TUNRING))
            MessageForward(sockfd, buffer, size);
    }
}

int RKAiqProtocol::doMessageForward(int sockfd)
{
    is_recv_running = true;
    while (is_recv_running) {
        char recv_buffer[MAXPACKETSIZE] = {0};
        int recv_len = g_tcpClient.Receive(recv_buffer, MAXPACKETSIZE);
        if (recv_len > 0) {
            ssize_t ret = send(sockfd, recv_buffer, recv_len, 0);
            if (ret < 0) {
                LOG_ERROR("#########################################################\n");
                LOG_ERROR("## Forward socket %d failed, please check AIQ status.####\n", sockfd);
                LOG_ERROR("#########################################################\n\n");

                close(sockfd);
                std::lock_guard<std::mutex> lk(mutex_);
                is_recv_running = false;
                return -1;
            }
        } else if (recv_len < 0 && errno != EAGAIN) {
            g_tcpClient.Close();
            close(sockfd);
            std::lock_guard<std::mutex> lk(mutex_);
            is_recv_running = false;
            return -1;
        }
    }

    std::lock_guard<std::mutex> lk(mutex_);
    is_recv_running = false;
    return 0;
}

int RKAiqProtocol::offlineRawProcess()
{
    startOfflineRawFlag = 1;
    LOG_DEBUG("offlineRawProcess begin\n");
    while (startOfflineRawFlag == 1) {
        DIR* dir = opendir("/data/OfflineRAW");
        struct dirent* dir_ent = NULL;
        std::vector<std::string> raw_files;
        if (dir) {
            while ((dir_ent = readdir(dir))) {
                if (dir_ent->d_type == DT_REG) {
                    // is raw file
                    if (strstr(dir_ent->d_name, ".raw")) {
                        raw_files.push_back(dir_ent->d_name);
                    }
                }
            }
            closedir(dir);
        }
        if (raw_files.size() == 0) {
            LOG_INFO("No raw files in /data/OfflineRAW\n");
            return 1;
        }

        std::sort(raw_files.begin(), raw_files.end());
        for (auto raw_file : raw_files) {
            cout << raw_file.c_str() << endl;
            if (startOfflineRawFlag == 0) {
                break;
            }
            LOG_DEBUG("ENUM_ID_SYSCTL_ENQUEUERKRAWFILE begin\n");
            struct timeval tv;
            struct timezone tz;
            gettimeofday(&tv, &tz);
            long startTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            LOG_DEBUG("begin millisecond: %ld\n", startTime); // ms
            std::string filePath = "/data/OfflineRAW/" + raw_file;
            LOG_INFO("process raw : %s \n", filePath.c_str());
            if (RkAiqSocketClientINETSend(ENUM_ID_SYSCTL_ENQUEUERKRAWFILE, (void*)filePath.c_str(),
                                          (unsigned int)filePath.length() + 1) != 0) {
                LOG_ERROR("########################################################\n");
                LOG_ERROR("#### OfflineRawProcess failed. Please check AIQ.####\n");
                LOG_ERROR("########################################################\n\n");
                return 1;
            }

            uint32_t frameInterval = 1000 / g_offlineFrameRate;
            std::this_thread::sleep_for(std::chrono::milliseconds(frameInterval));

            gettimeofday(&tv, &tz);
            long endTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            LOG_DEBUG("end millisecond: %ld\n", endTime);                                                   // ms
            LOG_DEBUG("####################################### time spend: %ld ms\n", endTime - startTime); // ms
            LOG_DEBUG("ENUM_ID_SYSCTL_ENQUEUERKRAWFILE end\n");
        }
    }
    std::lock_guard<std::mutex> lk(mutex_);
    LOG_DEBUG("offlineRawProcess end\n");
    return 0;
}

int RKAiqProtocol::MessageForward(int sockfd, char* buffer, int size)
{
    LOG_DEBUG("[%s]got data:%d!\n", __func__, size);
    // HexDump((unsigned char*)buffer, size);
    int ret = g_tcpClient.Send((char*)buffer, size);
    if (ret < 0 && (errno != EAGAIN && errno != EINTR)) {
        if (ConnectAiq() < 0) {
            g_tcpClient.Close();
            g_app_run_mode = APP_RUN_STATUS_INIT;
            LOG_ERROR("########################################################\n");
            LOG_ERROR("#### Forward to AIQ failed! please check AIQ status.####\n");
            LOG_ERROR("########################################################\n\n");
            close(sockfd);
            is_recv_running = false;
            return -1;
        } else {
            LOG_ERROR("########################################################\n");
            LOG_ERROR("#### Forward to AIQ failed! Auto reconnect success.####\n");
            LOG_ERROR("########################################################\n\n");
        }
    }

    std::lock_guard<std::mutex> lk(mutex_);
    if (is_recv_running) {
        return 0;
    }

#if 0
  if (forward_thread && forward_thread->joinable()) forward_thread->join();
#endif

    forward_thread = std::unique_ptr<std::thread>(new std::thread(&RKAiqProtocol::doMessageForward, sockfd));
    forward_thread->detach();

    return 0;
}

void RKAiqProtocol::Exit()
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        is_recv_running = false;
    }
#if 0
  if (forward_thread && forward_thread->joinable()) {
    forward_thread->join();
  }
#endif
}
