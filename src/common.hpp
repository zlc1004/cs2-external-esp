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

// Include color_t from types (this defines color_t properly for Windows)
#include "core/engine/types/Color.hpp"

using namespace al;

#include "core/logger/LogHelper.hpp"
#include "core/engine/types/Types.hpp"
#include "config/Current.hpp"

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;
