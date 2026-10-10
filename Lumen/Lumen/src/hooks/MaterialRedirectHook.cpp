#include "MaterialRedirectHook.h"
#include "HookMacros.h"
#include "Signatures.h"
#include "../Global.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

namespace Lumen::Engine {

namespace {
std::map<std::string, bool> gRedirectEnabled = {
    {"clouds", true},
    {"sky", true},
};

std::string MaterialsDir() {
  std::string dir = Global::GetLumenRoamingPath() + "\\materials";
  std::filesystem::create_directories(dir);
  return dir;
}

// Reads a whole file as bytes; empty string if it doesn't exist.
std::string ReadFileBytes(const std::string &path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return {};
  std::ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}
} // namespace

void SetMaterialRedirectEnabled(const char *name, bool enabled) {
  gRedirectEnabled[name] = enabled;
}

bool IsMaterialRedirectEnabled(const char *name) {
  auto it = gRedirectEnabled.find(name);
  return it != gRedirectEnabled.end() && it->second;
}

// --------------------------------------------------------------------------
// readFile hook. SIGNATURE SHAPE IS A BEST GUESS, NOT CONFIRMED:
// MSVC returns std::string by value via a hidden pointer (2nd arg here,
// `outStr`) — this mirrors the general pattern community tools use for this
// kind of RenderDragon file-read hook, but the exact parameter list/order
// needs verifying against 26.52.03 once it's actually hooked (easiest way:
// hook it with signature confirmed, log every `path` string that comes
// through, and see if material.bin paths show up as expected).
// --------------------------------------------------------------------------
DeclareHook(MaterialReadFile, void *, void *This, std::string *outStr,
            const char *pathUtf8) {
  void *ret = MaterialReadFile_Original(This, outStr, pathUtf8);

  if (!pathUtf8 || !outStr) return ret;
  std::string path(pathUtf8);

  static const char *kPrefix = "data/renderer/materials/";
  static const char *kSuffix = ".material.bin";
  auto prefixPos = path.find(kPrefix);
  if (prefixPos == std::string::npos) return ret;
  if (path.size() < strlen(kSuffix) ||
      path.compare(path.size() - strlen(kSuffix), strlen(kSuffix), kSuffix) != 0)
    return ret;

  // Extract "clouds" from ".../clouds.material.bin"
  std::string fileName = path.substr(prefixPos + strlen(kPrefix));
  std::string stem = fileName.substr(0, fileName.size() - strlen(kSuffix));

  // Case-insensitive-ish match against our small redirect table.
  for (auto &[name, enabled] : gRedirectEnabled) {
    if (!enabled) continue;
    if (stem.find(name) == std::string::npos) continue;

    std::string onDisk = MaterialsDir() + "\\" + name + ".material.bin";
    std::string bytes = ReadFileBytes(onDisk);
    if (!bytes.empty()) {
      *outStr = bytes;
      Lumen::Hooks::Log("[Lumen] redirected %s -> %s (%zu bytes)",
                        path.c_str(), onDisk.c_str(), bytes.size());
    } else {
      Lumen::Hooks::Log(
          "[Lumen] redirect enabled for '%s' but no file at %s yet",
          name.c_str(), onDisk.c_str());
    }
    break;
  }

  return ret;
}

void InstallMaterialRedirectHook() {
  if (!Signatures::MATERIAL_READ_FILE[0]) {
    Lumen::Hooks::Log(
        "[Lumen] no MATERIAL_READ_FILE signature for this build yet — "
        "cloud/sky redirect is wired up but inactive until it's filled in");
    return;
  }
  TrySigHook(MaterialReadFile, Signatures::MATERIAL_READ_FILE[0]);
}

} // namespace Lumen::Engine
