#pragma once

namespace Lumen::GUI {

// Hooks IDXGISwapChain::Present (via a short-lived dummy device+swapchain,
// the standard way to get real vtable addresses without having to catch
// the game's own device creation). Once hooked, every real Present call
// renders the Lumen ImGui overlay on top of the game.
//
// This part doesn't depend on any Minecraft-specific signature — it's the
// same technique used by essentially every DX12 overlay tool — so it
// should work as soon as this compiles, independent of whether the
// RenderDragon-specific hooks above have real signatures yet.
void InstallOverlayHook();

// Toggled by the Lumen hotkey (default: Insert). When false, the game
// renders normally with no overlay drawn and input passes through.
extern bool gMenuOpen;

} // namespace Lumen::GUI
