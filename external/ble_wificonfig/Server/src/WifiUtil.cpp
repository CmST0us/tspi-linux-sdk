#include "WifiUtil.h"

#include <stdlib.h>
#include <unistd.h>
#include <thread>


typedef std::list<std::string> LIST_STRING;
typedef std::list<WifiInfo*> LIST_WIFIINFO;
static int network_id;

static const char *WIFI_CONFIG_FORMAT = "ctrl_interface=/var/run/wpa_supplicant\n"
                                "ap_scan=1\n\nnetwork={\nssid=\"%s\"\n"
                                "psk=\"%s\"\npriority=1\n}\n";

void execute(const char cmdline[],char recv_buff[]){
    log_info("consule_run: %s\n",cmdline);
    FILE *stream = NULL;
    char buff[1024];

    memset(recv_buff, 0, strlen(recv_buff));
    if((stream = popen(cmdline,"r"))!=NULL){
        while(fgets(buff,1024,stream)){
            strcat(recv_buff,buff);
        }
    }
    pclose(stream);
}

int get_pid(const char Name[]) {
    int len;
    char name[20] = {0};
    len = strlen(Name);
    strncpy(name,Name,len);
    name[len] ='\0';
    char cmdresult[256] = {0};
    char cmd[20] = {0};
    FILE *pFile = NULL;
    int  pid = 0;

    sprintf(cmd, "pidof %s", name);
    pFile = popen(cmd, "r");
    if (pFile != NULL)  {
        while (fgets(cmdresult, sizeof(cmdresult), pFile)) {
            pid = atoi(cmdresult);
            break;
        }
    }
    pclose(pFile);
    return pid;
}

/**
 * split buff array by '\n' into string list.
 * @parm buff[]
 */
LIST_STRING charArrayToList(char buff[]){
    LIST_STRING stringList;
    std::string item;
    for(int i=0;i<strlen(buff);i++){
        if(buff[i] != '\n'){
            item += buff[i];
        } else {
            stringList.push_back(item);
            item.clear();
        }
    }
    return stringList;
}

/**
 * format string list into wifiInfo list by specific rules
 * @parm string_list
 * @return LIST_WIFIINFO
 */
LIST_WIFIINFO wifiStringFormat(LIST_STRING string_list){
    LIST_WIFIINFO wifiInfo_list;

    LIST_STRING::iterator stringIte;

    /* delete first useless item */
    string_list.pop_front();

    for(stringIte=string_list.begin();stringIte!=string_list.end();stringIte++){
        WifiInfo *wifiInfoItem = new WifiInfo();
        std::string wifiStringItem = *stringIte;

        /* use for set wifiInfo item:bssid ssid etc*/
        std::string tempString;
        int index = 0; /* temp index,flag '\t' appear count*/

        for(int i=0;i<wifiStringItem.size();i++){
            if(wifiStringItem.at(i)!='\t' && i != (wifiStringItem.size()-1)){
                tempString += wifiStringItem.at(i);
            } else {
                switch(index){
                case 0: //bssid
                    wifiInfoItem->setBssid(tempString);
                    break;
                case 1: //frequency
                    wifiInfoItem->setFrequency(tempString);
                    break;
                case 2: //signalLevel
                    wifiInfoItem->setSignalLevel(tempString);
                    break;
                case 3: //flags
                    wifiInfoItem->setFlags(tempString);
                    break;
                case 4: //ssid
                    tempString += wifiStringItem.at(i);
                    wifiInfoItem->setSsid(tempString);
                    break;
                default:
                    break;
                }
                index ++;
                tempString.clear();
            }
        }
        wifiInfo_list.push_back(wifiInfoItem);
    }
    return wifiInfo_list;
}

/**
 * parse wifi info list into json string.
 * @parm wifiInfoList
 * @return json string
 */
std::string parseIntoJson(LIST_WIFIINFO wifiInfoList){
    LIST_WIFIINFO::iterator iterator;

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    /* 1. add return type */
    document.AddMember("type","WifiList",allocator);
    /* 2. add reutn content */
    rapidjson::Value wifiArrayValue(rapidjson::kArrayType);
    for(iterator = wifiInfoList.begin(); iterator != wifiInfoList.end(); ++iterator){
        (*iterator)->addJsonToRoot(document,wifiArrayValue);
    }
    document.AddMember("content",wifiArrayValue,allocator);

    /* parse into string */
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}

/**
 * get json substr from http respose head.
 * split by '{' and "}"
 * @parm message http.
 */
std::string getJsonFromMessage(char message[]){
    std::string str(message);
    return str.substr(str.find('{'));
}

/**
 * use wpa_cli tool to connnect wifi in alexa device
 * @parm ssid
 * @parm password
 */
bool wifiConnect(std::string ssid,std::string password){
    char ret_buff[MSG_BUFF_LEN] = {0};
    char cmdline[MSG_BUFF_LEN] = {0};
    int id = -1;
    bool execute_result = false;

    // 1. add network
    execute("wpa_cli -iwlan0 add_network",ret_buff);
    id = atoi(ret_buff);
    if(id < 0){
        log_err("add_network failed.\n");
        return false;
    }
    // 2. setNetWorkSSID
    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline,"wpa_cli -iwlan0 set_network %d ssid \\\"%s\\\"",id, ssid.c_str());
    printf("%s\n", cmdline);
    execute(cmdline,ret_buff);
    execute_result = !strncmp(ret_buff,"OK",2);
    if(!execute_result){
        log_err("setNetWorkSSID failed.\n");
        return false;
    }
    // 3. setNetWorkPWD
    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline,"wpa_cli -iwlan0 set_network %d psk \\\"%s\\\"", id,password.c_str());
    printf("%s\n", cmdline);
    execute(cmdline,ret_buff);
    execute_result = !strncmp(ret_buff,"OK",2);
    if(!execute_result){
        log_err("setNetWorkPWD failed.\n");
        return false;
    }
    // 4. selectNetWork
    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline,"wpa_cli -iwlan0 select_network %d", id);
    printf("%s\n", cmdline);
    execute(cmdline,ret_buff);
    execute_result = !strncmp(ret_buff,"OK",2);
    if(!execute_result){
        log_err("setNetWorkPWD failed.\n");
        return false;
    }

    return true;
}

bool checkWifiIsConnected(){
    char ret_buff[MSG_BUFF_LEN] = {0};
    char cmdline[MSG_BUFF_LEN] = {0};

    LIST_STRING stateSList;
    LIST_STRING::iterator iterator;   

    // udhcpc network
    int udhcpc_pid = get_pid("udhcpc");
    if(udhcpc_pid != 0){
        memset(cmdline, 0, sizeof(cmdline));
        sprintf(cmdline,"kill %d",udhcpc_pid);
        execute(cmdline,ret_buff);
    }
    execute("udhcpc -n -t 10 -i wlan0",ret_buff);

    bool isWifiConnected = false;
    int match = 0;
    /* 15s to check wifi whether connected */
    for(int i=0;i<5;i++){
        sleep(2);
        match = 0;
        execute("wpa_cli -iwlan0 status",ret_buff);
        stateSList = charArrayToList(ret_buff);
        for(iterator=stateSList.begin();iterator!=stateSList.end();iterator++){
            std::string item = (*iterator);
            if(item.find("wpa_state")!=std::string::npos){
                if(item.substr(item.find('=')+1)=="COMPLETED"){
                    match++;
                }
            }
            if(item.find("ip_address")!=std::string::npos){
                if(item.substr(item.find('=')+1)!="127.0.0.1"){
                    match++;
                }
            }
        }
        if(match >= 2){
            isWifiConnected = true;
            // TODO play audio: wifi connected
            log_info("Congratulation: wifi connected.\n");
            break;
        }
        log_info("Check wifi state with none state. try more %d/5, \n",i+1);
    }

    if(!isWifiConnected){
        // TODO play audio: wifi failed.
        log_info("wifi connect failed.please check enviroment.\n");
        system("gst-play-1.0 -q --no-interactive /usr/ap_notification/wifi_connect_failed.mp3 &");
 
    } else {
        system("gst-play-1.0 -q --no-interactive /usr/ap_notification/wifi_conneted.mp3 &");
        system("softapServer stop &");
    }
}


std::string WifiUtil::getWifiListJson(){
    char ret_buff[MSG_BUFF_LEN] = {0};
    std::string ret;
    int retry_count = 10;

    LIST_STRING wifiStringList;
    LIST_WIFIINFO wifiInfoList;

retry:
    execute("wpa_cli -i wlan0 -p /var/run/wpa_supplicant scan", ret_buff);
    /* wap_cli sacn is useable */
    if(!strncmp(ret_buff,"OK",2)){
        log_info("scan useable: OKOK\n");
        execute("wpa_cli -i wlan0 -p /var/run/wpa_supplicant scan_r", ret_buff);
        wifiStringList = charArrayToList(ret_buff);
        wifiInfoList = wifiStringFormat(wifiStringList);
    }
    
    if ((wifiInfoList.size() == 0)  && (--retry_count > 0)) {
	usleep(500000);
        goto retry;
    }
    // parse wifiInfo list into json.
    ret = parseIntoJson(wifiInfoList);
    log_info("list size: %d\n",wifiInfoList.size());
    return ret;
}

std::string WifiUtil::getDeviceContextJson() {
    std::string ret = " ";
    std::string sn1;
    std::string sn2;
    std::string sn3;
    std::string sn4;

#define SN_1 "ro.hisense.jhkdeviceid"
#define SN_2 "ro.hisense.jhldeviceid"
#define SN_3 "ro.hisense.wifiid"
#define SN_4 "ro.hisense.uuid"

    rapidjson::Document newDoc;
    FILE *myFile = fopen(DEVICE_CONFIG_FILE, "r");
    if (!myFile) {
        log_info("%s, %s not exist\n", __func__, DEVICE_CONFIG_FILE);
        return ret;
    }
    char readBuffer[65536];
    rapidjson::FileReadStream is(myFile, readBuffer, sizeof(readBuffer));
    newDoc.ParseStream<0>(is);
    fclose(myFile);

    if (newDoc.HasParseError()) {
        log_info("Json Parse error: %d\n", newDoc.GetParseError());
        return ret;
    }
    if (newDoc.HasMember(SN_1)) {
         sn1 = newDoc[SN_1].GetString();
    }
    if (newDoc.HasMember(SN_2)) {
         sn2 = newDoc[SN_2].GetString();
    }
    if (newDoc.HasMember(SN_3)) {
         sn3 = newDoc[SN_3].GetString();
    }
    if (newDoc.HasMember(SN_4)) {
         sn4 = newDoc[SN_4].GetString();
    }
    log_info("Json Parse : %s, %s, %s, %s\n", sn1.c_str(), sn2.c_str(), sn3.c_str(), sn4.c_str());

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    /* 1. add return type */
    document.AddMember("type","DeviceContext",allocator);
    /* 2. add reutn content */
    rapidjson::Value snObj(rapidjson::kObjectType);
    snObj.AddMember(SN_1, rapidjson::StringRef(sn1.c_str()), allocator);
    snObj.AddMember(SN_2, rapidjson::StringRef(sn2.c_str()), allocator);
    snObj.AddMember(SN_3, rapidjson::StringRef(sn3.c_str()), allocator);
    snObj.AddMember(SN_4, rapidjson::StringRef(sn4.c_str()), allocator);
    document.AddMember("content",snObj,allocator);
    /* parse into string */
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}

static bool saveWifiConfig(const char* name, const char* pwd)
{
    FILE *fp;
    char body[WIFI_CONFIG_MAX];
    int fd;
    fp = fopen("/data/cfg/wpa_supplicant.conf", "w");

    if (fp == NULL)
    {
        return -1;
    }

    snprintf(body, sizeof(body), WIFI_CONFIG_FORMAT, name, pwd);
    fputs(body, fp);
    fflush(fp);
    fd = fileno(fp);
    if (fd >= 0) {
        fsync(fd);
        printf("save wpa_supplicant.conf sucecees.\n");
    }
    fclose(fp);

    return 0;
}


void WifiUtil::WifiSetUp(char recv_buff[]){
    std::string jsonString = getJsonFromMessage(recv_buff);

    /* get setUp user name and password */
    rapidjson::Document document;
    if (document.Parse(jsonString.c_str()).HasParseError()) {
        log_err("parseJsonFailed \n");
        return;
    }

    std::string userName;
    std::string password;

    auto userNameIterator = document.FindMember("ssid");
    if (userNameIterator != document.MemberEnd() && userNameIterator->value.IsString()) {
       	userName = userNameIterator->value.GetString();
    }

    auto passwordIterator = document.FindMember("pwd");
    if (passwordIterator != document.MemberEnd() && passwordIterator->value.IsString()) {
        password = passwordIterator->value.GetString();
    }

    if(userName.empty()||password.empty()){
        log_err("userName or password empty. \n");
        return;
    }

    /* use wpa_cli to connect wifi by ssid and password */
    bool connectResult = wifiConnect(userName,password);

    if(connectResult){
        std::thread thread(checkWifiIsConnected);
        thread.detach();

        saveWifiConfig(userName.c_str(), password.c_str());
    }else{
        log_info("wifi connect failed.please check enviroment. \n");
        // TODO play audio: wifi connect failed.
    }
    return;
}
