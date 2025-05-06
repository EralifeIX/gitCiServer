#pragma once
#include <microhttpd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#ifndef GITCISERVER_H
#define GITCISERVER_H

class gitCIServer {
public:
    gitCIServer(int port, const std::string& repoUrl, const std::string& cloneDir);
    ~gitCIServer();

    bool start();
    void stop();

private:
    static int handleRequest(void *cls, struct MHD_Connection *connection,
                             const char *url, const char *method,
                             const char *version, const char *upload_data,
                             size_t *upload_data_size, void **con_cls);

    int port_;
    std::string repoUrl_;
    std::string cloneDir_;
    struct MHD_Daemon* daemon_;
    std::string jsonData_;

    void processPush() const;
    std::string readTestResult();
};



#endif //GITCISERVER_H
