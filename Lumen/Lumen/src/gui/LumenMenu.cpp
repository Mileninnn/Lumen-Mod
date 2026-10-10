#include "LumenMenu.h"
#include "imgui.h"
#include "../hooks/DeferredShadingHook.h"
#include "../hooks/MaterialRedirectHook.h"

namespace Lumen::GUI {

void DrawLumenMenu(bool *open) {
  ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Lumen Shader Control", open, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }

  auto *params = Lumen::Engine::gDeferredParams;

  // --- Status Section ---
  ImGui::Text("Mod Status: ");
  ImGui::SameLine();
  if (params) {
    ImGui::TextColored(ImVec2(0, 1, 0, 1), "ACTIVE");
  } else {
    ImGui::TextColored(ImVec2(1, 0, 0, 1), "INACTIVE (Check Signatures)");
  }
  ImGui::Separator();

  if (ImGui::CollapsingHeader("Shadows", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool realShadows = params && params->mTexelShadowsEnabled;
    if (ImGui::Checkbox("Enable Real Shadows", &realShadows) && params)
      params->mTexelShadowsEnabled = realShadows;

    int shadowRes = params ? params->mQuantizedTexelSize : 8;
    ImGui::BeginDisabled(!params);
    if (ImGui::SliderInt("Shadow Sharpness", &shadowRes, 1, 32) && params)
      params->mQuantizedTexelSize = shadowRes;
    ImGui::EndDisabled();

    bool crisp = params && params->mDisableShadowFiltering;
    ImGui::BeginDisabled(!params);
    if (ImGui::Checkbox("High Contrast (No Filtering)", &crisp) && params)
      params->mDisableShadowFiltering = crisp;
    ImGui::EndDisabled();
  }

  if (ImGui::CollapsingHeader("Reflections", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool reflections = params && params->mTexelReflectionsEnabled;
    ImGui::BeginDisabled(!params);
    if (ImGui::Checkbox("Enable Texel Reflections", &reflections) && params)
      params->mTexelReflectionsEnabled = reflections;
    ImGui::EndDisabled();
  }

  if (ImGui::CollapsingHeader("Custom Materials", ImGuiTreeNodeFlags_DefaultOpen)) {
    bool cloudsOn = Lumen::Engine::IsMaterialRedirectEnabled("clouds");
    if (ImGui::Checkbox("Use Custom Clouds", &cloudsOn))
      Lumen::Engine::SetMaterialRedirectEnabled("clouds", cloudsOn);

    bool skyOn = Lumen::Engine::IsMaterialRedirectEnabled("sky");
    if (ImGui::Checkbox("Use Custom Sky", &skyOn))
      Lumen::Engine::SetMaterialRedirectEnabled("sky", skyOn);

    ImGui::Spacing();
    ImGui::TextDisabled("Path: %APPDATA%\\Lumen\\materials\\");
  }

  ImGui::Separator();
  ImGui::TextDisabled("Press INSERT to hide menu");

  ImGui::End();
}

} // namespace Lumen::GUI
