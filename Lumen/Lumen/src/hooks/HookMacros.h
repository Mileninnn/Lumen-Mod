#pragma once
// Small convenience layer over MinHook: lets a hook be declared as a normal
// function and installed by scanning for one of several byte signatures
// (one per supported game version). If none of the signatures are found
// (e.g. a brand-new Minecraft build we haven't scanned yet), the hook is
// simply skipped and Lumen logs it instead of crashing the game.

#include <MinHook.h>
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <fstream>
#include <initializer_list>
#include "Memory.h"
#include "../Global.h"

namespace Lumen::Hooks {

inline void Log(const char *fmt, ...) {
  char buf[1024];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  OutputDebugStringA(buf);
  OutputDebugStringA("\n");

  static std::string logPath = Global::GetLumenRoamingPath() + "\\logs.txt";
  std::ofstream f(logPath, std::ios::app);
  if (f) f << buf << "\n";
}

} // namespace Lumen::Hooks

// Declares `Name_Hook` (the detour) + `Name_Original` (trampoline pointer)
// with the given return type and argument list. Call `TrySigHook(Name, ...)`
// afterwards with one or more AOB patterns to actually install it.
#define DeclareHook(Name, RetType, ...)                                      \
  using Name##_t = RetType(__fastcall *)(__VA_ARGS__);                       \
  inline Name##_t Name##_Original = nullptr;                                 \
  RetType __fastcall Name##_Hook(__VA_ARGS__)

#define TrySigHook(Name, ...)                                                \
  do {                                                                       \
    uintptr_t target = Lumen::Memory::FindFirstMatching({__VA_ARGS__});      \
    if (!target) {                                                           \
      Lumen::Hooks::Log("[Lumen] signature not found for " #Name             \
                        " (game version mismatch?)");                        \
    } else {                                                                 \
      MH_STATUS st = MH_CreateHook(reinterpret_cast<void *>(target),         \
                                   reinterpret_cast<void *>(&Name##_Hook),    \
                                   reinterpret_cast<void **>(&Name##_Original)); \
      if (st == MH_OK) {                                                     \
        MH_EnableHook(reinterpret_cast<void *>(target));                     \
        Lumen::Hooks::Log("[Lumen] hooked " #Name " at 0x%p",                \
                          (void *)target);                                   \
      } else {                                                               \
        Lumen::Hooks::Log("[Lumen] MH_CreateHook failed for " #Name          \
                          " (%d)", (int)st);                                 \
      }                                                                      \
    }                                                                        \
  } while (0)
