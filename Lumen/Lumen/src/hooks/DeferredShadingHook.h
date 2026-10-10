#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace Lumen::Engine {

// Mirrors dragon::framerenderer::DeferredShadingParameters — the live,
// in-memory settings struct RenderDragon's Vibrant Visuals deferred
// renderer reads every frame (texel-space shadows/reflections, ambient and
// emissive contributions, point light falloff, etc).
//
// Field layout matters here: this has to match byte-for-byte or every field
// after the first mistake reads/writes garbage. Layout below reflects what's
// publicly documented by the BetterRenderDragon project (AGPL-3.0,
// github.com/QYCottage/BetterRenderDragon) for Minecraft ~1.26.20-1.26.30 —
// it has NOT been re-verified against your 26.52.03 build yet. Treat the
// three fields Lumen actually uses (flagged below) as "probably right,
// needs confirming"; the rest exists only to keep offsets correct.
//
// NOTE ON LICENSING: if Lumen ships with this layout as-is and leans on
// BetterRenderDragon's other hook logic too, AGPL-3.0's copyleft means
// Lumen would need to ship under AGPL-3.0 too (source available on request,
// even for private/in-game use). Worth deciding before this goes beyond
// personal use.
struct PointLightLODingParameters {
  std::vector<float> mLightingGridHierarchyRadii;
  float mBlockLightAmbientLightIntensity_BeforeDiffuseFadeOut;
  float mBlockLightAmbientLightIntensity_AfterDiffuseFadeOut;
  float mDistanceFromCamera_StartDiffuseFadeOut;
  float mDistanceFromCamera_EndDiffuseFadeOut;
  float mDistanceFromCamera_StartSpecularFadeOut;
  float mDistanceFromCamera_EndSpecularFadeOut;
  bool mEnableFadeOutPointLightDiffuse;
  bool mEnableFadeOutPointLightSpecular;
};

struct DeferredShadingParameters {
  enum class DirectionalShadowsMode : int { OFF = 0, CASCADED = 1 };

  std::array<float, 3> mBlockBaseAmbientLightColor;
  float mBlockLightIndirectContributionMultiplier;
  float mBlockLightIndirectSpecularFallbackMultiplier;
  float mSkyLightIndirectContributionMultiplier;
  float mIBLIndirectContributionMultiplier;
  float mVolumetricFogMinAmbientValue;
  uint32_t mIBLSkyLightFadeStart;
  uint32_t mIBLSkyLightFadeEnd;
  float mCameraLightIntensityLerpSpeed;
  float mEmissiveBrightnessMultiplier;
  int mEmissiveBrightnessVLUNormalizationFactor;
  float mDirectionalLightContributionFadeEndAngle;
  float mDirectionalLightContributionFadeBeginAngle;
  float mDirectionalLightContributionFadeEndAngleAtmospherics;
  float mDirectionalLightContributionFadeBeginAngleAtmospherics;
  float mSubsurfaceScatteringContribution;
  float mSubsurfaceScatteringDiffuseWrapValue;
  float mSubsurfaceScatteringFalloffDistance;
  float mEndPortalEmissiveMultiplier;
  float mSunTextureEmissiveMultiplier;
  float mMoonTextureEmissiveMultiplier;
  float mGlowInkSignStrength;
  float mLightningBoltAdaptiveEmissiveMultiplier;

  DirectionalShadowsMode mDirectionalShadowsMode;  // <- Lumen: shadows
  bool mSpecularEnabled;
  bool mDiffuseEnabled;
  bool mEmissiveEnabled;
  bool mAmbientEnabled;
  bool mDirectionalLightContributionEnabled;
  bool mPointLightContributionEnabled;
  bool mEnableDeferredShadingSkyLightHeuristic;
  bool mEnableVolumetricSkyLightHeuristic;
  bool mPointLightManhattanDistanceAttenuationEnabled;
  bool mPointLightAttenuationWindowEnabled;
  float mPointLightAttenuationWindowSmoothstepMin;
  float mPointLightAttenuationWindowSmoothstepMax;
  float mPointLightAttenuationWindowYScale;
  float mPointLightAttenuationWindowYBias;
  PointLightLODingParameters mPointLightLODingParameters;

  float mAmbientLightColor[3];
  float mAmbientLightIlluminance;
  float mSkyIntensity;

  bool mTexelReflectionsEnabled;  // <- Lumen: reflections
  bool mTexelShadowsEnabled;      // <- Lumen: "real" (texel-space) shadows on/off
  bool mDisableShadowFiltering;   // <- Lumen: shadows (crisp vs. filtered)
  int mQuantizedTexelSize;        // <- Lumen: shadows (lower = sharper/higher-res)
  float mQuantizationSurfaceNormalPrecision;
};

// Set once DEFERRED_SHADING_CTOR fires; null until then (and null entirely
// if the signature for your build hasn't been found yet — see Signatures.h).
inline DeferredShadingParameters *gDeferredParams = nullptr;

void InstallDeferredShadingHook();

} // namespace Lumen::Engine
