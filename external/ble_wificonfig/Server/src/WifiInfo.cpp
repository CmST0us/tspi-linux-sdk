#include "WifiInfo.h"

std::string WifiInfo::getBssid() {
    return this->m_bssid;
}
    
void WifiInfo::setBssid(std::string bssid) {
    this->m_bssid = bssid;
}
    
std::string WifiInfo::getFrequency() {
    return this->m_frequency;
}
   
void WifiInfo::setFrequency(std::string frequency) {
    this->m_frequency = frequency;
}
    
std::string WifiInfo::getSignalLevel() {
    return this->m_signalLevel;
}
    
void WifiInfo::setSignalLevel(std::string signalLevel) {
    this->m_signalLevel = signalLevel;
}
    
std::string WifiInfo::getFlags() {
    return this->m_flags;
}
    
void WifiInfo::setFlags(std::string flags) {
    this->m_flags = flags;
}
    
std::string WifiInfo::getSsid() {
    return this->m_ssid;
}
    
void WifiInfo::setSsid(std::string ssid) {
    this->m_ssid = ssid;
}

void WifiInfo::addJsonToRoot(rapidjson::Document &document,rapidjson::Value &root) {
    rapidjson::Value object(rapidjson::kObjectType);

    //object.AddMember("bssid",rapidjson::StringRef(m_bssid.c_str()),document.GetAllocator());
    //object.AddMember("frequency", rapidjson::StringRef(m_frequency.c_str()), document.GetAllocator());
    object.AddMember("level", rapidjson::StringRef(m_signalLevel.c_str()), document.GetAllocator());
    object.AddMember("flags", rapidjson::StringRef(m_flags.c_str()), document.GetAllocator());
    object.AddMember("id", rapidjson::StringRef(m_ssid.c_str()), document.GetAllocator());

    root.PushBack(object, document.GetAllocator());
}


