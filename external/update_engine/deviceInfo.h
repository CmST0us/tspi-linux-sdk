#pragma once

#define VersionPath "/etc/version"
class DeviceInfo{
public:
    virtual void setTargetConfig() = 0;
    void setCurrentConfig();
    void setTargetConfigData(std::string data);
    bool compareVersion();

    std::string getCurrentVersion();
    std::string getCurrentModel();
    std::string getTargetVersion();
    std::string getTargetModel();
    std::string getHost();

    void setHost(std::string _host);

protected:
    std::string currentModle;
    std::string host;
    std::string currentVersion;
    std::string targetModel;
    std::string targetVersion;
};


class DeviceInfoInternel: public DeviceInfo{
public:
    void setTargetConfig();
};
