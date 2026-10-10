#pragma once

namespace Lumen::Engine {

// Watches engine reads of specific "data/renderer/materials/<name>.material.bin"
// files and substitutes bytes from a Lumen-supplied file on disk when the
// name matches an entry in the redirect table (clouds / sky, by default —
// where you'd drop the NewB X-derived material.bin files). Everything else
// passes through untouched.
void InstallMaterialRedirectHook();

// Enable/disable a specific redirect by material file name, e.g. "clouds".
// Looks for <RoamingPath>\materials\<name>.material.bin on disk.
void SetMaterialRedirectEnabled(const char *name, bool enabled);
bool IsMaterialRedirectEnabled(const char *name);

} // namespace Lumen::Engine
