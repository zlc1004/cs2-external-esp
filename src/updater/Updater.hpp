#pragma once

#include "http/HttpHelper.hpp"

struct Status {
    bool unsafe;
    std::string notice;

    int version_current;
    int version_minimum;
};

class Updater {
public:
    ~Updater() = default;
    Updater(const Updater&) = delete;
    Updater(Updater&&) = delete;
    Updater& operator=(const Updater&) = delete;
    Updater& operator=(Updater&&) = delete;

    static bool Init();
    static bool Update(); // Not Implemented Yet
    static bool Process();
    static bool FetchOffsets(); // Fetch latest offsets from GitHub

    static Status GetStatus();
private:
    Updater() {};

    static Updater& GetInstance()
    {
        static Updater i{};
        return i;
    }

    bool InitImpl();
    bool ProcessImpl();
    bool FetchOffsetsImpl();
private:
    Status status;
    bool isSetup = false;
    int current_version = 102;
    // TODO: Update to main once released
    std::string status_url = "https://github.com/IMXNOOBX/cs2-external-esp/raw/refs/heads/cs2-recode/.github/status.json";
    std::string offsets_url = "https://raw.githubusercontent.com/zlc1004/cs2-external-esp/main/.github/offsets.json";
};