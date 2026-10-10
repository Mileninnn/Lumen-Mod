#pragma once
// ============================================================================
// AOB signatures for Minecraft.Windows.exe, one list per hook.
//
// STATUS: these are placeholders. Nothing here has been found against your
// current build (26.52.03) yet — that requires a disassembler pass
// (Ghidra/IDA/x64dbg) on the real executable, which isn't something I can do
// from this sandbox (no Windows, no game binary, no disassembler available
// to me here). Until a real pattern is filled in, TrySigHook() for that
// function will just log "signature not found" and skip the hook — Lumen
// will still load and the overlay will still work, the affected feature
// just won't be live yet.
//
// How to fill these in once Ghidra is set up (happy to walk through this
// live when you're ready):
//   1. Open Minecraft.Windows.exe for your installed version in Ghidra.
//   2. For DEFERRED_SHADING_CTOR: search for xrefs to the
//      DeferredShadingParameters struct — it's constructed alongside the
//      ray tracing / Vibrant Visuals feature setup during world load.
//   3. For MATERIAL_READ_FILE: search for the function that resolves
//      "data/renderer/materials/<name>.material.bin" paths — it's called
//      from the resource/material loading path during shader pack load.
//   4. Grab ~16-40 bytes of the function prologue, convert to space-
//      separated hex, replace wildcard bytes (addresses/offsets that will
//      differ per-build, e.g. operands of CALL/LEA) with "?".
// ============================================================================

namespace Lumen::Signatures {

// dragon::framerenderer::DeferredShadingParameters constructor (or any
// function that receives a pointer into the live settings struct) —
// capturing this gives Lumen a stable pointer to the struct whose fields
// drive shadow mode, texel reflections, etc.
inline constexpr const char *DEFERRED_SHADING_CTOR[] = {
    // "26.52.03" -- Found via LBIAS_SHADOW_C lead
    "48 C7 85 ? ? ? ? 48 C7 85 ? ? ? ? 48 C7 85 ? ? ? ? 48 8B 05",
};

// The function that resolves "data/renderer/materials/<name>.material.bin"
// resource paths. Hooking this lets Lumen redirect specific material.bin
// reads (clouds / sky) to files Lumen supplies instead of the engine's
// built-in ones.
inline constexpr const char *MATERIAL_READ_FILE[] = {
    // "26.52.03" -- TODO: find and paste pattern here
    nullptr,
};

} // namespace Lumen::Signatures
