cmake_minimum_required(VERSION 3.15)
project(Lumen)

set(CMAKE_CXX_STANDARD 17)

# Force MinHook to download
include(FetchContent)
FetchContent_Declare(
  minhook
  GIT_REPOSITORY https://github.com/TsudaKageyasu/minhook.git
  GIT_TAG main
)
FetchContent_MakeAvailable(minhook)

# Force ImGui to download
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.90.4
)
FetchContent_MakeAvailable(imgui)

add_library(Lumen SHARED
    src/dllmain.cpp
    src/Config.cpp
    src/gui/LumenMenu.cpp
    src/gui/LumenOverlay.cpp
    src/hooks/DeferredShadingHook.cpp
    src/hooks/MaterialRedirectHook.cpp
    src/hooks/Memory.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
)

target_include_directories(Lumen PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hooks
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${minhook_SOURCE_DIR}
    ${minhook_SOURCE_DIR}/include
)

target_link_libraries(Lumen PRIVATE minhook)

3. Update the Build Script one last time
Edit your .github/workflows/build.yml and paste this. I've simplified it so it doesn't search—it just builds from the root:

name: Build Lumen DLL

on:
  push:
    branches: [ "main", "master" ]
  workflow_dispatch:

jobs:
  build:
    runs-on: windows-latest
    steps:
    - uses: actions/checkout@v4
    - name: Configure
      run: cmake -B build -A x64 .
    - name: Build
      run: cmake --build build --config Release
    - name: Upload
      uses: actions/upload-artifact@v4
      with:
        name: Lumen-DLL
        path: build/bin/Release/Lumen.dll
