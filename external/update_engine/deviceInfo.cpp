#include <iostream>
#include "log.h"
#include "deviceInfo.h"
#include <curl/curl.h>
#include <fstream>
#include <string>
#include <vector>


void DeviceInfo::setHost(std::string _host){
    host = _host;
}

void DeviceInfo::setCurrentConfig(){
    std::ifstream f(VersionPath);
    std::string strLine, strItemName, strItemValue;

    while(getline(f, strLine)){
        std::string::size_type line_size, pos;
        line_size = strLine.size();
        if(line_size == 0 || strLine[0] == '#')
            continue;
        pos = strLine.find("=");

        if(pos == std::string::npos)
            continue;
        strItemName = strLine.substr(0, pos);
        strItemValue = strLine.substr(pos+1);
        if(strItemName.compare("RK_MODEL") == 0){
            currentModle = strItemValue;
        }else if(strItemName.compare("RK_VERSION") == 0){
            currentVersion = strItemValue;
        }else if(strItemName.compare("RK_OTA_HOST") == 0){
            //setHost(strItemValue);
        }
    }
    f.close();
}

void DeviceInfo::setTargetConfigData(std::string data){
    std::string::size_type pos1, pos2;
    std::vector<std::string> v;
    pos2 = data.find("\n");
    pos1 = 0;
    while(std::string::npos != pos2){
        v.push_back(data.substr(pos1, pos2 - pos1));
        data.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = data.find("\n", pos1);
    }

    for(std::vector<std::string>::size_type i = 0; i != v.size(); i++){
        std::string strItemName, strItemValue;
        if(v[i].size() == 0 || v[i][0] == '#'){
            continue;
        }
        pos1 = v[i].find("=");
        if(pos1 == std::string::npos){
            continue;
        }
        strItemName = v[i].substr(0, pos1);
        strItemValue = v[i].substr(pos1+1);
        if(strItemName.compare("RK_MODEL") == 0){
            targetModel = strItemValue;
        }else if(strItemName.compare("RK_VERSION") == 0){
            targetVersion = strItemValue;
        }
    }
}
bool DeviceInfo::compareVersion(){
    std::cout << targetVersion << std::endl;
    std::cout << currentVersion << std::endl;
    int retval =  currentVersion.compare(targetVersion);
    if(retval < 0)
        return true;
    else
        return false;
}

std::string DeviceInfo::getCurrentVersion(){
    return currentVersion;
}

std::string DeviceInfo::getCurrentModel(){
    return currentModle;
}

std::string DeviceInfo::getTargetVersion(){
    return targetVersion;
}

std::string DeviceInfo::getTargetModel(){
    return targetModel;
}

std::string DeviceInfo::getHost(){
    return host;
}

static size_t writeConfigToString(void *ptr, size_t size, size_t count, void *stream){
    ((std::string*)stream)->append((char*)ptr, 0, size* count);
    return size* count;
}

void DeviceInfoInternel::setTargetConfig(){
    CURL *curl;
    CURLcode res;
    std::string data;

    curl = curl_easy_init();
    if(curl) {
        std::string url = host;
        std::cout << "url is " << url << std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeConfigToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        // data now holds response

        // write curl response to string variable..
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
        setTargetConfigData(data);
    }
    std::cout << targetVersion << std::endl;
    std::cout << targetModel << std::endl;
}
