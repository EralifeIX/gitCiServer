#include "gitCIServer.h"


gitCIServer::gitCIServer(int port, const std::string& repoUrl, const std::string& cloneDir)
    : port_(port), repoUrl_(repoUrl), cloneDir_(cloneDir), daemon_(nullptr) {}

gitCIServer::~gitCIServer() {
    stop();
}

bool gitCIServer::start() {
    daemon_ = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, port_, nullptr, nullptr,
                               reinterpret_cast<MHD_AccessHandlerCallback>(&gitCIServer::handleRequest), this, MHD_OPTION_END);
    return daemon_ != nullptr;
}

void gitCIServer::stop() {
    if (daemon_) {
        MHD_stop_daemon(daemon_);
        daemon_ = nullptr;
    }
}

int gitCIServer::handleRequest(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls) {
    auto *server = static_cast<gitCIServer*>(cls);

    if (std::string(method) != "POST") return MHD_NO;

    static std::string* jsonData = new std::string;
    if (*upload_data_size != 0) {
        jsonData->append(upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }

    // Обработка push-события
    if (jsonData->find("\"ref\"") == std::string::npos) {
        std::string error = "Not a push event\n";
        auto* response = MHD_create_response_from_buffer(
            error.size(), (void *)error.c_str(), MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        delete jsonData;
        return ret;
    }

    // Клонирование/обновление и запуск тестов
    server->jsonData_ = *jsonData;
    server->processPush();
    std::string result = server->readTestResult();

    auto* response = MHD_create_response_from_buffer(
        result.size(), (void*)result.c_str(), MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    delete jsonData;
    return ret;
}

void gitCIServer::processPush() const {
    namespace fs = std::filesystem;

    // Создаём родительскую папку, если её нет
    fs::path clonePath(cloneDir_);
    if (!fs::exists(clonePath.parent_path())) {
        fs::create_directories(clonePath.parent_path());
    }

    // Если репозиторий не клонирован — клонируем
    if (!fs::exists(cloneDir_)) {
        std::string cmd = "git clone " + repoUrl_ + " " + cloneDir_;
        std::system(cmd.c_str());
    } else {
        std::string cmd = "cd " + cloneDir_ + " && git pull";
        std::system(cmd.c_str());
    }

    // Запуск тестов
    std::string testCmd = "cd " + cloneDir_ + " && ./run_tests.sh > result.txt 2>&1";
    std::system(testCmd.c_str());
}

std::string gitCIServer::readTestResult() {
    std::ifstream resultFile(cloneDir_ + "/result.txt");
    return std::string((std::istreambuf_iterator<char>(resultFile)),
                        std::istreambuf_iterator<char>());
}
