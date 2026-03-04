#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#include "WICTextureLoader.h"


// Link required libraries
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;

// --- Shaders ---
const char* shaderSource = R"(
struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
}

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(float4(input.Pos, 1.0f), World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Tex = input.Tex;
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    return txDiffuse.Sample(samLinear, input.Tex);
}
)";

// --- Structures ---
struct SimpleVertex
{
  XMFLOAT3 Pos;
  XMFLOAT2 Tex;
};

struct ConstantBuffer
{
  XMMATRIX mWorld;
  XMMATRIX mView;
  XMMATRIX mProjection;
};

// --- Global Variables ---
HINSTANCE                 g_hInst = nullptr;
HWND                      g_hWnd = nullptr;
D3D_DRIVER_TYPE           g_driverType = D3D_DRIVER_TYPE_HARDWARE;
D3D_FEATURE_LEVEL         g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*             g_pd3dDevice = nullptr;
ID3D11DeviceContext*      g_pImmediateContext = nullptr;
IDXGISwapChain*           g_pSwapChain = nullptr;
ID3D11RenderTargetView*   g_pRenderTargetView = nullptr;
ID3D11Texture2D*          g_pDepthStencil = nullptr;
ID3D11DepthStencilView*   g_pDepthStencilView = nullptr;
ID3D11VertexShader*       g_pVertexShader = nullptr;
ID3D11PixelShader*        g_pPixelShader = nullptr;
ID3D11InputLayout*        g_pVertexLayout = nullptr;
ID3D11Buffer*             g_pVertexBuffer = nullptr;
ID3D11Buffer*             g_pIndexBuffer = nullptr;
ID3D11Buffer*             g_pConstantBuffer = nullptr;
ID3D11ShaderResourceView* g_pTextureRV = nullptr;
ID3D11SamplerState*       g_pSamplerLinear = nullptr;
XMMATRIX                  g_World;
XMMATRIX                  g_View;
XMMATRIX                  g_Projection;

// --- Function Prototypes ---
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  if (FAILED(InitWindow(hInstance, nCmdShow))) return 0;
  if (FAILED(InitDevice()))
  {
    CleanupDevice();
    return 0;
  }

  MSG msg = { 0 };
  while (WM_QUIT != msg.message) {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      Render();
    }
  }

  CleanupDevice();
  return (int)msg.wParam;
}

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = nullptr;
  wcex.lpszClassName = L"DX11CubeWindowClass";
  wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
  if (!RegisterClassEx(&wcex)) return E_FAIL;

  g_hInst = hInstance;
  RECT rc = { 0, 0, 800, 600 };
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
  g_hWnd = CreateWindow(L"DX11CubeWindowClass", L"DirectX 11 Textured Rotating Cube",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
    rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
  if (!g_hWnd) return E_FAIL;

  ShowWindow(g_hWnd, nCmdShow);
  return S_OK;
}

HRESULT CompileShaderFromSource(const char* szSource, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
  HRESULT hr = S_OK;
  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
  ID3DBlob* pErrorBlob = nullptr;
  hr = D3DCompile(szSource, strlen(szSource), nullptr, nullptr, nullptr, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
  if (FAILED(hr))
  {
    if (pErrorBlob) OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
    if (pErrorBlob) pErrorBlob->Release();
    return hr;
  }
  if (pErrorBlob) pErrorBlob->Release();
  return S_OK;
}

HRESULT InitDevice()
{
  HRESULT hr = S_OK;
  RECT rc;
  GetClientRect(g_hWnd, &rc);
  UINT width = rc.right - rc.left;
  UINT height = rc.bottom - rc.top;

  UINT createDeviceFlags = 0;
  D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
  UINT numFeatureLevels = ARRAYSIZE(featureLevels);

  DXGI_SWAP_CHAIN_DESC sd = {};
  sd.BufferCount = 1;
  sd.BufferDesc.Width = width;
  sd.BufferDesc.Height = height;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = g_hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;

  hr = D3D11CreateDeviceAndSwapChain(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels,
    numFeatureLevels, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
  if (FAILED(hr)) return hr;

  ID3D11Texture2D* pBackBuffer = nullptr;
  hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
  if (FAILED(hr)) return hr;
  hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
  pBackBuffer->Release();
  if (FAILED(hr)) return hr;

  D3D11_TEXTURE2D_DESC descDepth = {};
  descDepth.Width = width;
  descDepth.Height = height;
  descDepth.MipLevels = 1;
  descDepth.ArraySize = 1;
  descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  descDepth.SampleDesc.Count = 1;
  descDepth.SampleDesc.Quality = 0;
  descDepth.Usage = D3D11_USAGE_DEFAULT;
  descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil);
  if (FAILED(hr)) return hr;

  D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
  descDSV.Format = descDepth.Format;
  descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
  if (FAILED(hr)) return hr;

  g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

  D3D11_VIEWPORT vp;
  vp.Width = (FLOAT)width;
  vp.Height = (FLOAT)height;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  g_pImmediateContext->RSSetViewports(1, &vp);

  ID3DBlob* pVSBlob = nullptr;
  hr = CompileShaderFromSource(shaderSource, "VS", "vs_4_0", &pVSBlob);
  if (FAILED(hr)) return hr;
  hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
  if (FAILED(hr)) return hr;

  D3D11_INPUT_ELEMENT_DESC layout[] =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  hr = g_pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pVertexLayout);
  pVSBlob->Release();
  if (FAILED(hr)) return hr;
  g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

  ID3DBlob* pPSBlob = nullptr;
  hr = CompileShaderFromSource(shaderSource, "PS", "ps_4_0", &pPSBlob);
  if (FAILED(hr)) return hr;
  hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
  pPSBlob->Release();
  if (FAILED(hr)) return hr;

  // Cube Vertices with UV mapping (duplicated vertices for correct texture alignment per face)
  SimpleVertex vertices[] =
  {
    // Front Face
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
    // Back Face
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
    // Top Face
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
    // Bottom Face
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
    // Left Face
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },
    // Right Face
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
  };

  D3D11_BUFFER_DESC bd = {};
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(vertices);
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  D3D11_SUBRESOURCE_DATA InitData = { vertices };
  hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
  if (FAILED(hr)) return hr;

  WORD indices[] =
  {
      0,1,2, 0,2,3,       // Front
      4,6,5, 4,7,6,       // Back
      8,9,10, 8,10,11,    // Top
      12,14,13, 12,15,14, // Bottom
      16,17,18, 16,18,19, // Left
      20,22,21, 20,23,22  // Right
  };

  bd.ByteWidth = sizeof(indices);
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  InitData.pSysMem = indices;
  hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
  if (FAILED(hr)) return hr;

  CreateWICTextureFromFile(g_pd3dDevice, L"dx.jpg", nullptr, &g_pTextureRV);

  // Create Sampler State
  D3D11_SAMPLER_DESC sampDesc = {};
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
  if (FAILED(hr)) return hr;

  bd.ByteWidth = sizeof(ConstantBuffer);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);
  if (FAILED(hr)) return hr;

  g_World = XMMatrixIdentity();
  XMVECTOR Eye = XMVectorSet(0.0f, 1.5f, -5.0f, 0.0f);
  XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
  XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  g_View = XMMatrixLookAtLH(Eye, At, Up);
  g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f);

  return S_OK;
}

void CleanupDevice()
{
  if (g_pImmediateContext) g_pImmediateContext->ClearState();
  if (g_pSamplerLinear) g_pSamplerLinear->Release();
  if (g_pTextureRV) g_pTextureRV->Release();
  if (g_pConstantBuffer) g_pConstantBuffer->Release();
  if (g_pVertexBuffer) g_pVertexBuffer->Release();
  if (g_pIndexBuffer) g_pIndexBuffer->Release();
  if (g_pVertexLayout) g_pVertexLayout->Release();
  if (g_pVertexShader) g_pVertexShader->Release();
  if (g_pPixelShader) g_pPixelShader->Release();
  if (g_pDepthStencil) g_pDepthStencil->Release();
  if (g_pDepthStencilView) g_pDepthStencilView->Release();
  if (g_pRenderTargetView) g_pRenderTargetView->Release();
  if (g_pSwapChain) g_pSwapChain->Release();
  if (g_pImmediateContext) g_pImmediateContext->Release();
  if (g_pd3dDevice) g_pd3dDevice->Release();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

void Render() 
{
  static float t = 0.0f;
  static ULONGLONG timeStart = 0;
  ULONGLONG timeCur = GetTickCount64();
  if (timeStart == 0) timeStart = timeCur;
  t = (timeCur - timeStart) / 1000.0f;

  // Rotate the cube around X, Y, and Z axes
  g_World = XMMatrixRotationX(t) * XMMatrixRotationY(t * 0.7f) * XMMatrixRotationZ(t * 0.3f);

  float ClearColor[4] = { 0.15f, 0.15f, 0.15f, 1.0f };
  g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
  g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

  ConstantBuffer cb;
  cb.mWorld = XMMatrixTranspose(g_World);
  cb.mView = XMMatrixTranspose(g_View);
  cb.mProjection = XMMatrixTranspose(g_Projection);
  g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);

  UINT stride = sizeof(SimpleVertex);
  UINT offset = 0;
  g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
  g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
  g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
  g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
  g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
  g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
  g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
  g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);

  g_pImmediateContext->DrawIndexed(36, 0, 0);
  g_pSwapChain->Present(0, 0);
}
