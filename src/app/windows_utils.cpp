#include "app/windows_utils.hpp"

#include <array>
#include <cstring>
#include <filesystem>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace pixel_town {

std::filesystem::path system_executable_path() {
#ifdef _WIN32
    std::wstring buffer(260, L'\0');
    DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    while (length == buffer.size()) {
        buffer.resize(buffer.size() * 2);
        length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    }
    if (length == 0) {
        return {};
    }
    buffer.resize(length);
    return std::filesystem::path{buffer};
#elif defined(__APPLE__)
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    if (size == 0) {
        return {};
    }
    std::string buffer(size, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        return {};
    }
    buffer.resize(std::strlen(buffer.c_str()));
    return std::filesystem::path{buffer};
#elif defined(__linux__)
    std::array<char, 4096> buffer{};
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length <= 0) {
        return {};
    }
    return std::filesystem::path{std::string{buffer.data(), static_cast<std::size_t>(length)}};
#else
    return {};
#endif
}

}  // namespace pixel_town
