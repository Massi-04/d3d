#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

// Roba finestra win32
const int WIDTH = 1280;
const int HEIGHT = 720;
const bool WINDOWED = true;
const char* TITLE = "MassiWnd";
const char* WND_CLASS_NAME = "MassiWndClass";
HWND WndHandle = nullptr;

// roba d3d11
IDXGISwapChain* swapChain = nullptr;
ID3D11Device* device = nullptr;
ID3D11DeviceContext* context = nullptr;
ID3D11RenderTargetView* renderTargetView = nullptr;

// roba d3d11 buffer e shader ecc...
ID3D11Buffer* vertexBuffer = nullptr;
ID3D11InputLayout* vertexLayout = nullptr;
ID3D11VertexShader* vertexShader = nullptr;
ID3D11PixelShader* pixelShader = nullptr;
ID3D10Blob* vertexShaderSrc = nullptr;
ID3D10Blob* pixelShaderSrc = nullptr;

struct Vec3
{
    Vec3(float x, float y, float z)
        : X(x), Y(y), Z(z)
    {}

    Vec3()
        : X(0.0f), Y(0.0f), Z(0.0f)
    {}

    float X, Y, Z;
};

struct Vertex
{
    Vertex()
    {
    }

    Vertex(Vec3 position, Vec3 color)
        : Position(position), Color(color)
    {
    }

    Vec3 Position;
    Vec3 Color;
};

#define MESSAGE_BOX_ERR(Error) MessageBoxA(NULL, Error, "Errore", MB_OK | MB_ICONERROR)

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool InitWindow(HINSTANCE hInstance, int showWnd, int width, int height, bool windowed)
{
    WNDCLASSEXA wndClass = {};
    wndClass.cbSize = sizeof(WNDCLASSEXA);
    wndClass.style = CS_VREDRAW | CS_HREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wndClass.lpszClassName = WND_CLASS_NAME;
    wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExA(&wndClass))
    {
        MESSAGE_BOX_ERR("Impossibile registrare la wndclass");
        return false;
    }
    
    WndHandle = CreateWindowExA(0, WND_CLASS_NAME, TITLE, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT, nullptr, nullptr, hInstance, nullptr);

    if (!WndHandle)
    {
        int x = GetLastError();
        MESSAGE_BOX_ERR("Impossibile creare la finestra");
        return false;
    }

    ShowWindow(WndHandle, showWnd);
    UpdateWindow(WndHandle);

    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    switch (msg)
    {
    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            if (MessageBoxA(0, "Vuoi uscire?", "Boh", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                DestroyWindow(WndHandle);
            }
        }
    }
    break;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
    break;
    
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

void UpdateScene();
void DrawScene();

void MainLoop()
{
    MSG msg = {};
    bool appShouldRun = true;

    while (appShouldRun)
    {
        // Processa gli eventi della finestra
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                appShouldRun = false;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        // Game loop
        // ...
        UpdateScene();
        DrawScene();
    }
}

bool InitD3D(HINSTANCE hInstance)
{
    DXGI_MODE_DESC screenBufferDesc = {};
    screenBufferDesc.Width = WIDTH;
    screenBufferDesc.Height = HEIGHT;
    screenBufferDesc.RefreshRate = { 60, 1 }; // 60hz
    screenBufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM; // rgb 8 bit per colore anche alpha
    screenBufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // ordine della 
    // rasterizzazione, per esempio dal basso verso lalto o vicevesa o da destra a sinistra ecc... Nel nostro caso non ci importa
    // avendo il doppio buffering, scriveremo nel back buffer e quando sara presentato sara finito, quindi non ci interessa da che parte inizia
    screenBufferDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
    // possiamo scegliere se stretchare l'immagine o tagliarla, in questo caso la dimensione del buffer è la stessa della finestra,
    // quindi a noi non interessa, il risultato è lo stesso
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferDesc = screenBufferDesc;
    swapChainDesc.SampleDesc = { 1, 0 }; // no antialiasing ( 1 = numero di sample per pixel, 0 = qualita del sampling) 
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // serve a indicare se la cpu può accedere al back buffer, 
    // lo flagghiamo come render_target perchè lo useremo per presentare la scena
    swapChainDesc.BufferCount = 1; // 1 = doppio buffering, 0 = single buffering, 2 = triplo buffering ecc...
    // noi usiamo 1 perchè vogliamo il doppio buffering, 1 back buffer sul quale renderizzare e un front buffer sul quale presentare
    swapChainDesc.OutputWindow = WndHandle;
    swapChainDesc.Windowed = WINDOWED;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD; // il driver sceglie il modo migliore per scambiare
    // il back buffer con il front buffer quando presentiamo
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // permette di passare da fullscreen a finestra

    HRESULT result = D3D11CreateDeviceAndSwapChain
    (
        0, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, 0 /* qui si puo mettere l'instance di una dll per rasterizzazione custom*/,
        0, 0, 0, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, 0, &context
    );

    if (result != S_OK)
    {
        MESSAGE_BOX_ERR("Impossibile creare la swapchain");
        return false;
    }

    ID3D11Texture2D* backBuffer;
    swapChain->GetBuffer(0 /*first buffer (back buffer)*/, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

    device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);

    backBuffer->Release();

    context->OMSetRenderTargets(1, &renderTargetView, nullptr);

    return true;
}

void CleanUp()
{
    swapChain->Release();
    device->Release();
    context->Release();
    renderTargetView->Release();
    vertexBuffer->Release();
    vertexLayout->Release();
    vertexShader->Release();
    pixelShader->Release();
    vertexShaderSrc->Release();
    pixelShaderSrc->Release();
}

bool InitScene()
{   
    HRESULT res = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShaderSrc, nullptr);
    if (res != S_OK)
    {
        MESSAGE_BOX_ERR("Impossibile compilare la vertex shader");
        return false;
    }
    res = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShaderSrc, nullptr);
    if (res != S_OK)
    {
        MESSAGE_BOX_ERR("Impossibile compilare la pixel shader");
        return false;
    }

    res = device->CreateVertexShader(vertexShaderSrc->GetBufferPointer(), vertexShaderSrc->GetBufferSize(), nullptr, &vertexShader);
    if (res != S_OK)
    {
        MESSAGE_BOX_ERR("Impossibile creare la vertex shader");
        return false;
    }
    res = device->CreatePixelShader(pixelShaderSrc->GetBufferPointer(), pixelShaderSrc->GetBufferSize(), nullptr, &pixelShader);
    if (res != S_OK)
    {
        MESSAGE_BOX_ERR("Impossibile creare la pixel shader");
        return false;
    }

    context->VSSetShader(vertexShader, nullptr, 0);
    context->PSSetShader(pixelShader, nullptr, 0);

    Vertex vertexBufferData[] =
    {
        Vertex({ 0.0f, 0.5f, 0.5f }, { 0.196f, 0.658f, 0.321f }), // verde
        Vertex({ 0.5f, -0.5f, 0.5f }, { 0.627f, 0.196f, 0.658f }), // viola
        Vertex({ -0.5f, -0.5f, 0.5f }, { 0.658f, 0.392f, 0.196f }) // marrone
    };

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(vertexBufferData);
    vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexBufferDataDesc = {};
    vertexBufferDataDesc.pSysMem = vertexBufferData;
    
    res = device->CreateBuffer(&vertexBufferDesc, &vertexBufferDataDesc, &vertexBuffer);
    if (res != S_OK)
    {
        MESSAGE_BOX_ERR("Impossibile creare il buffer");
        return false;
    }

    unsigned int stride = sizeof(Vertex);
    unsigned int offset = 0; 

    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    
    D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLORE", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    res = device->CreateInputLayout
    (
        vertexLayoutDesc, 2, vertexShaderSrc->GetBufferPointer(),
        vertexShaderSrc->GetBufferSize(), &vertexLayout
    );
    if (res != S_OK)
    {
        MESSAGE_BOX_ERR("Impossibile creare l'input layout");
        return false;
    }

    context->IASetInputLayout(vertexLayout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = WIDTH;
    viewport.Height = HEIGHT;
    
    context->RSSetViewports(1, &viewport);

    return true;
}

float background[4] = { 0.259f, 0.529f, 0.961f, 1.0f };

void UpdateScene()
{
    /*background[0] += 0.005f;
    if (background[0] > 1)
    {
        background[0] = 0.0f;
    }*/
}

void DrawScene()
{
    context->ClearRenderTargetView(renderTargetView, background);

    context->Draw(3, 0);

    swapChain->Present(1, 0);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    if (!InitWindow(hInstance, nCmdShow, WIDTH, HEIGHT, WINDOWED))
    {
        MESSAGE_BOX_ERR("Errore InitWindow()");
        return -1;
    }
    
    if (!InitD3D(hInstance))
    {
        MESSAGE_BOX_ERR("Errore InitD3D()");
        return -1;
    }

    if (!InitScene())
    {
        MESSAGE_BOX_ERR("Errore InitScene()");
        return -1;
    }

    MainLoop();
    CleanUp();
    return 0;
}