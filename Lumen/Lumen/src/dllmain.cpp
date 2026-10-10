#include <windows.h>
#include <MinHook.h>
#include <thread>
#include <chrono>

#include "Global.h"
#include "Config.h"
#include "hooks/HookMacros.h"
#include "hooks/DeferredShadingHook.h"
#include "hooks/MaterialRedirectHook.h"
#include "gui/LumenOverlay.h"

namespace {

void LumenInit() {
  Lumen::Hooks::Log("[Lumen] starting up");

  // Give the game a moment to finish its own init before we start poking
  // at it — same reasoning BetterRenderDragon uses this delay for.
  std::this_thread::sleep_for(std::chrono::seconds(1));

  MH_Initialize();

  Lumen::Engine::InstallDeferredShadingHook();
  Lumen::Engine::InstallMaterialRedirectHook();
  Lumen::Config::Load();

  // Overlay hook last: once this is in, the menu can render even if the
  // engine-specific hooks above are still waiting on signatures.
  Lumen::GUI::InstallOverlayHook();

  Lumen::Hooks::Log("[Lumen] ready — press Insert to open the menu");
}

} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
  switch (reason) {
  case DLL_PROCESS_ATTACH:
    Global::hModule = hModule;
    DisableThreadLibraryCalls(hModule);
    CreateThread(nullptr, 0,
                 reinterpret_cast<LPTHREAD_START_ROUTINE>(LumenInit), nullptr,
                 0, nullptr);
    break;
  case DLL_PROCESS_DETACH:
    Lumen::Config::Save();
    break;
  }
  return TRUE;
}
