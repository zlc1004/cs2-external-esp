#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "external/AsyncLogger/include/AsyncLogger/Logger.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <imgui.h>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include "external/nlohmann/json.hpp"
#include <string_view>
#include <thread>
#include <vector>

// Dummy color_t for compilation on macOS. Will be properly defined by GUI library on Windows.
struct color_t {
    float r, g, b, a;
};

using namespace al;

#include "core/logger/LogHelper.hpp"
#include "core/engine/types/Types.hpp"
#include "config/Current.hpp"

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;
