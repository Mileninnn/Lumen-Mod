#include "Config.h"
#include "Global.h"
#include "hooks/MaterialRedirectHook.h"
#include <fstream>
#include <sstream>
#include <string>

namespace Lumen::Config {

namespace {
std::string ConfigPath() { return Global::GetLumenRoamingPath() + "\\config.txt"; }
} // namespace

void Load() {
  std::ifstream f(ConfigPath());
  if (!f) return; // first run, defaults from MaterialRedirectHook.cpp apply

  std::string line;
  while (std::getline(f, line)) {
    auto eq = line.find('=');
    if (eq == std::string::npos) continue;
    std::string key = line.substr(0, eq);
    std::string value = line.substr(eq + 1);
    bool enabled = (value == "1");

    if (key == "redirect.clouds")
      Engine::SetMaterialRedirectEnabled("clouds", enabled);
    else if (key == "redirect.sky")
      Engine::SetMaterialRedirectEnabled("sky", enabled);
  }
}

void Save() {
  std::ofstream f(ConfigPath(), std::ios::trunc);
  if (!f) return;
  f << "redirect.clouds=" << (Engine::IsMaterialRedirectEnabled("clouds") ? 1 : 0) << "\n";
  f << "redirect.sky=" << (Engine::IsMaterialRedirectEnabled("sky") ? 1 : 0) << "\n";
}

} // namespace Lumen::Config
