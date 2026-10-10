#include "Memory.h"
#include <psapi.h>
#include <sstream>

namespace Lumen::Memory {

namespace {
struct PatternByte {
  uint8_t value;
  bool wildcard;
};

std::vector<PatternByte> Parse(const char *pattern) {
  std::vector<PatternByte> out;
  std::istringstream ss(pattern);
  std::string token;
  while (ss >> token) {
    if (token == "?" || token == "??") {
      out.push_back({0, true});
    } else {
      out.push_back({static_cast<uint8_t>(strtoul(token.c_str(), nullptr, 16)),
                      false});
    }
  }
  return out;
}
} // namespace

uintptr_t GetModuleBase() {
  // Minecraft.Windows.exe is always the main module of its own process.
  return reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
}

size_t GetModuleSize() {
  MODULEINFO info{};
  HMODULE mod = GetModuleHandleW(nullptr);
  if (!mod) return 0;
  if (!GetModuleInformation(GetCurrentProcess(), mod, &info, sizeof(info)))
    return 0;
  return info.SizeOfImage;
}

uintptr_t FindPattern(const char *pattern) {
  if (!pattern) return 0;
  const auto needle = Parse(pattern);
  if (needle.empty()) return 0;

  const uintptr_t base = GetModuleBase();
  const size_t size = GetModuleSize();
  if (!base || !size) return 0;

  const uint8_t *data = reinterpret_cast<const uint8_t *>(base);
  const size_t needleLen = needle.size();

  for (size_t i = 0; i + needleLen <= size; ++i) {
    bool found = true;
    for (size_t j = 0; j < needleLen; ++j) {
      if (!needle[j].wildcard && data[i + j] != needle[j].value) {
        found = false;
        break;
      }
    }
    if (found) return base + i;
  }
  return 0;
}

uintptr_t FindFirstMatching(std::initializer_list<const char *> patterns) {
  for (const char *p : patterns) {
    if (!p) continue;
    if (uintptr_t addr = FindPattern(p)) return addr;
  }
  return 0;
}

} // namespace Lumen::Memory
