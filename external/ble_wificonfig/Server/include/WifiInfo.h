#ifndef __WIFI_INFO_H__
#define __WIFI_INFO_H__

#include <string>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

class WifiInfo {

public:
    WifiInfo(){}

    std::string getBssid();
    std::string getFrequency();
    std::string getSignalLevel();
    std::string getFlags();
    std::string getSsid();

    void setBssid(std::string bssid);
    void setFrequency(std::string frequency);
    void setSignalLevel(std::string signalLevel);
    void setFlags(std::string flags);
    void setSsid(std::string ssid);
    void addJsonToRoot(rapidjson::Document &document,rapidjson::Value &root);
private:
    std::string m_bssid;
    std::string m_frequency;
    std::string m_signalLevel;
    std::string m_flags;
    std::string m_ssid;

};

#endif // __WIFI_INFO_H__
