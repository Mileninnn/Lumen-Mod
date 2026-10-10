#include "DeferredShadingHook.h"
#include "HookMacros.h"
#include "Signatures.h"

namespace Lumen::Engine {

namespace {
// Offset of the DeferredShadingParameters struct inside whatever object
// DEFERRED_SHADING_CTOR receives as `this`. BetterRenderDragon's public
// code uses +176 for the MC versions it targets; unconfirmed for 26.52.03.
// TODO: verify once the signature above is found and the ctor is hooked —
// easiest way is to hook it, dump `this` + a few hundred bytes, and compare
// against known default values (e.g. mSkyIntensity, mAmbientLightIlluminance)
// to find where the struct actually starts.
constexpr ptrdiff_t kDeferredParamsOffset = 176;
} // namespace

DeclareHook(DeferredShadingCtor, void, void *_this) {
  DeferredShadingCtor_Original(_this);
  if (!gDeferredParams) {
    gDeferredParams = reinterpret_cast<DeferredShadingParameters *>(
        reinterpret_cast<uint8_t *>(_this) + kDeferredParamsOffset);
    Lumen::Hooks::Log("[Lumen] captured DeferredShadingParameters* = 0x%p",
                      (void *)gDeferredParams);
  }
}

void InstallDeferredShadingHook() {
  if (!Signatures::DEFERRED_SHADING_CTOR[0]) {
    Lumen::Hooks::Log(
        "[Lumen] no DEFERRED_SHADING_CTOR signature for this build yet — "
        "shadow/reflection sliders will show but won't do anything until "
        "it's filled in");
    return;
  }
  TrySigHook(DeferredShadingCtor, Signatures::DEFERRED_SHADING_CTOR[0]);
}

} // namespace Lumen::Engine
