#pragma once

#include "../common.hpp" // For color_t
#include "../external/json/include/nlohmann/json.hpp" // For nlohmann::json

using json = nlohmann::json;

class Config {
public:
    ~Config() = default;
    Config(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(const Config&) = delete;
    Config& operator=(Config&&) = delete;

    static bool Read();
    static bool Write();
private:
    Config() {};

    static Config& GetInstance()
    {
        static Config i{};
        return i;
    }

    bool ReadImpl();
    bool WriteImpl();

    static color_t JsonToColor(const json& parent, const std::string& key, const color_t& def);
    static void ColorToJson(json& parent, const std::string& key, const color_t& color);
};