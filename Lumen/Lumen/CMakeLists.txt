cmake_minimum_required(VERSION 3.20)
project(Lumen LANGUAGES CXX)

if(NOT WIN32)
  message(FATAL_ERROR "Lumen targets Windows (Bedrock Edition / RenderDragon on DX12) only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ---- Dear ImGui (docking branch has the DX12 backend we need) ----
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.91.1
)
FetchContent_MakeAvailable(imgui)

# ---- MinHook (x64 trampoline hooking) ----
FetchContent_Declare(
  minhook
  GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
  GIT_TAG        v1.3.3
)
FetchContent_MakeAvailable(minhook)

add_library(Lumen SHARED
  src/dllmain.cpp
  src/Global.h
  src/Config.h
  src/Config.cpp

  src/hooks/Memory.h
  src/hooks/Memory.cpp
  src/hooks/HookMacros.h
  src/hooks/Signatures.h
  src/hooks/DeferredShadingHook.h
  src/hooks/DeferredShadingHook.cpp
  src/hooks/MaterialRedirectHook.h
  src/hooks/MaterialRedirectHook.cpp

  src/gui/LumenOverlay.h
  src/gui/LumenOverlay.cpp
  src/gui/LumenMenu.h
  src/gui/LumenMenu.cpp

  # Dear ImGui core + Win32/DX12 backends
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
)

target_include_directories(Lumen PRIVATE
  src
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(Lumen PRIVATE
  minhook
  d3d12
  dxgi
  d3dcompiler
)

set_target_properties(Lumen PROPERTIES
  OUTPUT_NAME "Lumen"
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_compile_definitions(Lumen PRIVATE UNICODE _UNICODE)
