cmake_minimum_required(VERSION 3.15)
project(Lumen)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
  minhook
  GIT_REPOSITORY https://github.com/TsudaKageyasu/minhook.git
  GIT_TAG main
)
FetchContent_MakeAvailable(minhook)

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
