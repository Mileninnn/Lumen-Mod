#include "LumenOverlay.h"
#include "LumenMenu.h"
#include "../hooks/HookMacros.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <windows.h>
#include <vector>
#include <wrl/client.h>

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

using Microsoft::WRL::ComPtr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Lumen::GUI {

bool gMenuOpen = false;

namespace {

HWND gGameWindow = nullptr;
WNDPROC gOriginalWndProc = nullptr;

ID3D12Device *gDevice = nullptr;
ID3D12CommandQueue *gCommandQueue = nullptr;
ID3D12DescriptorHeap *gRtvHeap = nullptr;
ID3D12DescriptorHeap *gSrvHeap = nullptr;
ID3D12GraphicsCommandList *gCommandList = nullptr;
UINT gRtvDescriptorSize = 0;
bool gImGuiInitialized = false;

struct FrameContext {
  ComPtr<ID3D12CommandAllocator> allocator;
  ComPtr<ID3D12Resource> backBuffer;
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
};
std::vector<FrameContext> gFrames;

LRESULT CALLBACK LumenWndProc(HWND hWnd, UINT msg, WPARAM wParam,
                              LPARAM lParam) {
  if (msg == WM_KEYUP && wParam == VK_INSERT) {
    gMenuOpen = !gMenuOpen;
  }

  if (gMenuOpen && gImGuiInitialized) {
    ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
    const ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse &&
        (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST))
      return true;
    if (io.WantCaptureKeyboard &&
        (msg >= WM_KEYFIRST && msg <= WM_KEYLAST))
      return true;
  }

  return CallWindowProc(gOriginalWndProc, hWnd, msg, wParam, lParam);
}

void SetupFrames(IDXGISwapChain3 *swapChain, UINT bufferCount) {
  D3D12_DESCRIPTOR_HEAP_DESC rtvDesc{};
  rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvDesc.NumDescriptors = bufferCount;
  rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  gDevice->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&gRtvHeap));
  gRtvDescriptorSize =
      gDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  gFrames.resize(bufferCount);
  D3D12_CPU_DESCRIPTOR_HANDLE rtvStart =
      gRtvHeap->GetCPUDescriptorHandleForHeapStart();

  for (UINT i = 0; i < bufferCount; ++i) {
    gDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                    IID_PPV_ARGS(&gFrames[i].allocator));
    swapChain->GetBuffer(i, IID_PPV_ARGS(&gFrames[i].backBuffer));
    gFrames[i].rtvHandle = rtvStart;
    gFrames[i].rtvHandle.ptr += static_cast<SIZE_T>(i) * gRtvDescriptorSize;
    gDevice->CreateRenderTargetView(gFrames[i].backBuffer.Get(), nullptr,
                                    gFrames[i].rtvHandle);
  }

  D3D12_DESCRIPTOR_HEAP_DESC srvDesc{};
  srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  srvDesc.NumDescriptors = 1;
  srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  gDevice->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&gSrvHeap));

  gDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                             gFrames[0].allocator.Get(), nullptr,
                             IID_PPV_ARGS(&gCommandList));
  gCommandList->Close();
}

void EnsureInitialized(IDXGISwapChain3 *swapChain) {
  if (gImGuiInitialized) return;

  if (FAILED(swapChain->GetDevice(IID_PPV_ARGS(&gDevice)))) return;
  if (!gCommandQueue) return; // wait until ExecuteCommandLists hook sees one

  DXGI_SWAP_CHAIN_DESC desc{};
  swapChain->GetDesc(&desc);
  gGameWindow = desc.OutputWindow;

  SetupFrames(swapChain, desc.BufferCount);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGui_ImplWin32_Init(gGameWindow);
  ImGui_ImplDX12_Init(gDevice, static_cast<int>(desc.BufferCount),
                      DXGI_FORMAT_R8G8B8A8_UNORM, gSrvHeap,
                      gSrvHeap->GetCPUDescriptorHandleForHeapStart(),
                      gSrvHeap->GetGPUDescriptorHandleForHeapStart());

  gOriginalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(
      gGameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(LumenWndProc)));

  gImGuiInitialized = true;
}

DeclareHook(ExecuteCommandLists, void, ID3D12CommandQueue *This,
            UINT NumCommandLists, ID3D12CommandList *const *ppCommandLists) {
  if (!gCommandQueue) gCommandQueue = This;
  ExecuteCommandLists_Original(This, NumCommandLists, ppCommandLists);
}

DeclareHook(Present, HRESULT, IDXGISwapChain3 *This, UINT SyncInterval,
            UINT Flags) {
  EnsureInitialized(This);

  if (gImGuiInitialized) {
    UINT frameIndex = This->GetCurrentBackBufferIndex();
    FrameContext &frame = gFrames[frameIndex];

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (gMenuOpen) {
      Lumen::GUI::DrawLumenMenu(&gMenuOpen);
    }

    ImGui::Render();

    frame.allocator->Reset();
    gCommandList->Reset(frame.allocator.Get(), nullptr);

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = frame.backBuffer.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    gCommandList->ResourceBarrier(1, &barrier);

    gCommandList->OMSetRenderTargets(1, &frame.rtvHandle, FALSE, nullptr);
    ID3D12DescriptorHeap *heaps[] = {gSrvHeap};
    gCommandList->SetDescriptorHeaps(1, heaps);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gCommandList);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    gCommandList->ResourceBarrier(1, &barrier);

    gCommandList->Close();
    ID3D12CommandList *lists[] = {gCommandList};
    gCommandQueue->ExecuteCommandLists(1, lists);
  }

  return Present_Original(This, SyncInterval, Flags);
}

// Creates a throwaway device + swapchain purely to read real vtable
// pointers for Present/ExecuteCommandLists out of them, then tears them
// down. Standard technique for hooking DX12 without catching the game's
// own device creation call.
bool GetRealVTableTargets(void **outPresent, void **outExecute) {
  WNDCLASSEX wc{sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc,
               0,       0,         GetModuleHandle(nullptr),
               nullptr, nullptr,   nullptr,
               nullptr, L"LumenDummy", nullptr};
  RegisterClassEx(&wc);
  HWND hwnd = CreateWindow(wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW, 0, 0,
                           100, 100, nullptr, nullptr, wc.hInstance, nullptr);

  ComPtr<ID3D12Device> device;
  D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

  D3D12_COMMAND_QUEUE_DESC qDesc{};
  ComPtr<ID3D12CommandQueue> queue;
  device->CreateCommandQueue(&qDesc, IID_PPV_ARGS(&queue));

  ComPtr<IDXGIFactory4> factory;
  CreateDXGIFactory1(IID_PPV_ARGS(&factory));

  DXGI_SWAP_CHAIN_DESC scDesc{};
  scDesc.BufferCount = 2;
  scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  scDesc.OutputWindow = hwnd;
  scDesc.SampleDesc.Count = 1;
  scDesc.Windowed = TRUE;
  scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

  ComPtr<IDXGISwapChain> swapChain;
  if (FAILED(factory->CreateSwapChain(queue.Get(), &scDesc, &swapChain))) {
    DestroyWindow(hwnd);
    return false;
  }

  void **swapChainVTable = *reinterpret_cast<void ***>(swapChain.Get());
  void **queueVTable = *reinterpret_cast<void ***>(queue.Get());

  *outPresent = swapChainVTable[8];       // IDXGISwapChain::Present
  *outExecute = queueVTable[10];          // ID3D12CommandQueue::ExecuteCommandLists

  DestroyWindow(hwnd);
  return true;
}

} // namespace

void InstallOverlayHook() {
  void *presentAddr = nullptr;
  void *executeAddr = nullptr;
  if (!GetRealVTableTargets(&presentAddr, &executeAddr)) {
    Lumen::Hooks::Log("[Lumen] failed to resolve DX12 vtable targets");
    return;
  }

  MH_CreateHook(presentAddr, reinterpret_cast<void *>(&Present_Hook),
               reinterpret_cast<void **>(&Present_Original));
  MH_EnableHook(presentAddr);

  MH_CreateHook(executeAddr,
               reinterpret_cast<void *>(&ExecuteCommandLists_Hook),
               reinterpret_cast<void **>(&ExecuteCommandLists_Original));
  MH_EnableHook(executeAddr);

  Lumen::Hooks::Log("[Lumen] DX12 overlay hooks installed");
}

} // namespace Lumen::GUI
