#pragma once

namespace Lumen::Config {

// Loads %APPDATA%\Lumen\config.txt (if present) and applies it to the
// material redirect toggles. Call once at startup, after
// InstallMaterialRedirectHook().
void Load();

// Writes current toggle state back to disk. Call on menu close / DLL unload.
void Save();

} // namespace Lumen::Config
