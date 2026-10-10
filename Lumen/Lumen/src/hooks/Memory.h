#pragma once
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>

namespace Lumen::Memory {

// Parses a pattern like "48 8B 89 ? ? ? ? 48 8B 01" (space-separated hex
// bytes, "?" = wildcard byte) and scans the main Minecraft.Windows.exe
// module for it. Returns the address of the first match, or 0 if not found.
uintptr_t FindPattern(const char *pattern);

// Convenience: tries each pattern in order (useful for "one pattern per
// game version") and returns the first match found, or 0 if none hit.
uintptr_t FindFirstMatching(std::initializer_list<const char *> patterns);

uintptr_t GetModuleBase();
size_t GetModuleSize();

} // namespace Lumen::Memory
