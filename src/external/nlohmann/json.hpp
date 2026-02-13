#pragma once

// Dummy nlohmann/json.hpp for compilation on macOS
// In a real project, this would be the actual nlohmann/json library header

namespace nlohmann {
    class json {};
}
