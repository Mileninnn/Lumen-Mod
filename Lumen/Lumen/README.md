# Lumen

A shader-loader mod for Minecraft Bedrock Edition — real RenderDragon/Vibrant
Visuals shadows and reflections, editable at runtime, plus a cloud/sky
material swap-in layer (built to drop in NewB X's `clouds.material.bin` /
`sky.material.bin`). Loads via LeviLauncher's mod manager like a normal DLL
mod — **no LeviLamina required**.

## Status — what's real right now vs. what's still pending

This compiles into a working DLL with a real, working piece and a stubbed
piece:

**Real and working as soon as you build this:**
- DLL loads via LeviLauncher's mod manager (standard `DllMain` injection,
  same model as your performance mod)
- DX12 `Present` hook + Dear ImGui overlay (the standard overlay-injection
  technique — this doesn't depend on any Minecraft-specific reverse
  engineering)
- **Insert** toggles the Lumen menu (separate hotkey from the performance
  mod's backslash menu, since these are two separate DLLs on purpose)
- The menu itself: Shadows / Reflections / Clouds sections only, nothing
  more detailed, as asked
- Cloud/sky material.bin redirect plumbing (reads a file you drop in
  `%APPDATA%\Lumen\materials\`) and settings persistence

**Stubbed, pending reverse engineering against your 26.52.03 build:**
- The actual pointer into RenderDragon's live `DeferredShadingParameters`
  struct (drives the Shadow/Reflections sliders) — needs one AOB signature
- The actual material.bin read-redirect hook (drives the Clouds section) —
  needs one AOB signature

Both of these are wired up and will "just work" the moment a signature is
filled into `src/hooks/Signatures.h` — until then, the sliders render but
don't affect anything in-game, and Lumen logs that plainly rather than
pretending. I don't have Ghidra/IDA or your game binary available to me
here to find these myself, so finding them is the next real step — happy to
walk through it with you once you've got Ghidra set up, same as we talked
about for the performance mod.

## Why this scope is realistic

I looked at **BetterRenderDragon** (github.com/QYCottage/BetterRenderDragon,
AGPL-3.0) — an existing open-source mod doing almost exactly this: real
texel-space shadow/reflection control and material.bin redirection, loaded
via LeviLauncher, no LeviLamina. Studying its architecture (not copying its
code) is what told me:
- RenderDragon's shadows/reflections for Vibrant Visuals live in one
  in-memory struct (`DeferredShadingParameters`) that's cheap to expose
  once you have a pointer to it — this is "find one struct pointer," not
  "write a shader compiler," which is why the real-shadows/reflections half
  of this is much more tractable than a true Iris-style shader pipeline
  would be
- Clouds/sky aren't in that struct — they're separate material.bin assets,
  which is exactly why your plan to redirect those from NewB X specifically
  (rather than trying to make them "live-editable" too) is the right call
- The project targets Minecraft 1.26.2x; your 26.52.03 is newer, so its
  exact byte signatures won't match your binary even if borrowed directly

**License note:** BetterRenderDragon is AGPL-3.0. The struct layout in
`DeferredShadingHook.h` is modeled on what it documents (struct layouts
aren't really copyrightable expression, just a factual memory layout) — but
if Lumen ends up leaning on more of its actual hook code later, AGPL's
copyleft would mean Lumen would need to ship under AGPL-3.0 too (source
available on request). Worth keeping in mind if this goes beyond personal
use.

## Build

Requires Visual Studio (MSVC, x64) + CMake. Internet access during
configure (CMake pulls Dear ImGui and MinHook via `FetchContent`).

```
cmake -B build -A x64
cmake --build build --config Release
```

Output: `build\bin\Release\Lumen.dll`

## Install (LeviLauncher, no LeviLamina)

1. Open LeviLauncher → your "main" instance → Mods page
2. Import `Lumen.dll`
3. Launch, press **Insert** in-game to open the menu

## Clouds / sky (NewB X material swap)

1. From your NewB X install, locate `clouds.material.bin` and
   `sky.material.bin`
2. Drop them in `%APPDATA%\Lumen\materials\`
3. Toggle "Use NewB X clouds material" / "Use NewB X sky material" in the
   Clouds section of the menu

## Next steps

1. Get Ghidra set up (can walk through this together)
2. Find the two signatures in `src/hooks/Signatures.h` for 26.52.03
3. Confirm the `DeferredShadingParameters` struct layout still matches —
   test by watching `mSkyIntensity`/`mAmbientLightIlluminance` through the
   menu against known default lighting behavior
4. Confirm the exact `.material.bin` filenames Vibrant Visuals uses for
   clouds/sky on your build (the redirect hook currently matches on
   `clouds`/`sky` appearing anywhere in the filename, so close variants
   should still catch — but worth confirming against real log output)
