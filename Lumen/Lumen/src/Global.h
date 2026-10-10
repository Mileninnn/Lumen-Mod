#pragma once
#include <windows.h>
#include <shlobj_core.h>
#include <filesystem>
#include <string>

namespace Global {
inline HMODULE hModule = nullptr;

// %APPDATA%\Lumen
inline std::string GetLumenRoamingPath() {
  PWSTR raw = nullptr;
  if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &raw) != S_OK) {
    if (raw) CoTaskMemFree(raw);
    return "";
  }
  std::filesystem::path path(raw);
  CoTaskMemFree(raw);
  path /= "Lumen";
  std::filesystem::create_directories(path);
  return path.string();
}
} // namespace Global
