#include "fractalwindow.h"
#include <iostream>

namespace fractal
{
	void FractalWindow::InitGraphics(int width, int height)
	{
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;
		D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, &featureLevel, 1, D3D11_SDK_VERSION, &m_device3D, nullptr, &m_context3D);

		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.AntialiasedLineEnable = FALSE;
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.ScissorEnable = FALSE;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		ThrowIfFailed(m_device3D->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState));
		m_context3D->RSSetState(m_rasterizerState);

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.StencilEnable = TRUE;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.DepthEnable = TRUE;
		ThrowIfFailed(m_device3D->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState));
	}
	void FractalWindow::CreateWindowsizeDependentResources(int width, int height)
	{
		AutoReleasePtr<IDXGIFactory> factory;
		ThrowIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));

		m_renderTargetView.Release();
		m_swapChain.Release();
		m_depthStencilBuffer.Release();
		m_depthStencilView.Release();

		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = m_window;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;
		ThrowIfFailed(factory->CreateSwapChain(m_device3D, &swapChainDesc, &m_swapChain));

		AutoReleasePtr<ID3D11Texture2D> backBuffer;
		ThrowIfFailed(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer));
		ThrowIfFailed(m_device3D->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView));
		backBuffer.Release();

		D3D11_TEXTURE2D_DESC depthBufferDesc{};
		depthBufferDesc.Width = width;
		depthBufferDesc.Height = height;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;
		ThrowIfFailed(m_device3D->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		ThrowIfFailed(m_device3D->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView));

		m_windowSize.x = static_cast<float>(width);
		m_windowSize.y = static_cast<float>(height);

		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.Width = m_windowSize.x;
		m_viewport.Height = m_windowSize.y;
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;
		m_context3D->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
		m_context3D->OMSetDepthStencilState(m_depthStencilState, 0);
		m_context3D->RSSetViewports(1, &m_viewport);

		m_fractalParams.screenAspect = m_windowSize.x / m_windowSize.y;
	}
	void FractalWindow::CreateResources()
	{
		D3D11_BUFFER_DESC bufferDesc{};
		float2 vertices[] = {
			{ -1.0f, -1.0f },
			{ -1.0f,  1.0f },
			{ 1.0f,  1.0f },
			{ 1.0f, -1.0f }
		};
		unsigned indices[] = {
		0, 1, 2, 2, 3, 0
		};
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(vertices);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices;
		ThrowIfFailed(m_device3D->CreateBuffer(&bufferDesc, &initData, &m_vertexBuffer));
		bufferDesc.ByteWidth = sizeof(indices);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		initData.pSysMem = indices;
		ThrowIfFailed(m_device3D->CreateBuffer(&bufferDesc, &initData, &m_indexBuffer));

		CreateVertexShader();
		CreatePixelShader();

		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		bufferDesc.ByteWidth = sizeof(FractalParams);
		ThrowIfFailed(m_device3D->CreateBuffer(&bufferDesc, NULL, &m_cbFractalParams));

		UINT stride = sizeof(float2);
		UINT offset = 0;
		UINT bufferNumber = 0;
		m_context3D->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
		m_context3D->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_context3D->PSSetConstantBuffers(0, 1, &m_cbFractalParams);
	}
	void FractalWindow::CreateVertexShader()
	{
		const char* vsCode = R"(
struct VertexInputType
{
	float2 position : POSITION;
};
struct PixelInputType
{
	float4 windowPosition : SV_POSITION;
	float2 position : POSITION;
};
PixelInputType main(VertexInputType input)
{
	PixelInputType output;
	output.windowPosition = float4(input.position, 1.0f, 1.0f);
	output.position = input.position;
	return output;
})";
		AutoReleasePtr<ID3DBlob> shaderByteCode;
		AutoReleasePtr<ID3DBlob> errorMessage;
		ThrowIfFailed(
			D3DCompile(
				vsCode, strlen(vsCode), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &shaderByteCode, &errorMessage),
			errorMessage ?
			reinterpret_cast<const char*>(errorMessage->GetBufferPointer()) :
			nullptr);
		ThrowIfFailed(m_device3D->CreateVertexShader(shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), NULL, &m_vertexShader));
		D3D11_INPUT_ELEMENT_DESC inputLayoutDesc = { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		ThrowIfFailed(m_device3D->CreateInputLayout(&inputLayoutDesc, 1, shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), &m_inputLayout));
		m_context3D->IASetInputLayout(m_inputLayout);
		m_context3D->VSSetShader(m_vertexShader, NULL, 0);
	}
	void FractalWindow::CreatePixelShader()
	{
		const char* psCode(R"(
cbuffer Data
{
	float2 center;
	float2 steepness;
	float screenAspect;
	float zoom;
	float iterationCount;
	uint rootCount;
	float4 roots[16];
};
struct PixelInputType
{
	float4 windowPosition : SV_POSITION;
	float2 position : POSITION;
};

inline float2 ComplexMul(float2 z1, float2 z2)
{
	return float2(z1.x * z2.x - z1.y * z2.y, z1.x * z2.y + z1.y * z2.x);
}
inline float2 ComplexDiv(float2 z1, float2 z2)
{
	return float2(z1.x * z2.x + z1.y * z2.y, z1.y * z2.x - z1.x * z2.y) / (z2.x * z2.x + z2.y * z2.y);
}

float2 Func(float2 z)
{
	return ComplexMul(z, ComplexMul(z, z)) - float2(1.0f, 0.0f);
	float2 ret = z - roots[0].xy;
	for (uint r = 1; r < rootCount; r++)
		ret = ComplexMul(ret, z - roots[r].xy);
	return ret;
}
float2 Deriv(float2 z)
{
	return 3.0f * ComplexMul(z, z);
	float2 delta = float2(1.0f, 0.0f) / zoom * 1e-3f;
	return ComplexDiv(Func(z + delta) - Func(z - delta), 2.0f * delta);
}

float4 IterationsToColor(float r)
{
	float R = abs(r * 6.0f - 3.0f) - 1.0f;
	float G = 2.0f - abs(r * 6.0f - 2.0f);
	float B = 2.0f - abs(r * 6.0f - 4.0f);
	return float4(saturate(float3(B, G, R))*(1.0f - R * 0.49f), 1.0f);
}

bool IsNear(float2 z)
{
	float tolerance = 1e-5f;
	for (uint r = 0; r < rootCount; r++)
		if (length(z - roots[r].xy) < tolerance)
			return true;
	return false;
}

float4 main(PixelInputType input) : SV_TARGET
{
	float i;
	float2 z = float2(input.position.x * screenAspect, input.position.y) / zoom + center;
	for (i = 0.0f; i < iterationCount; i += 1.0f)
	{
		z -= ComplexMul(steepness, ComplexDiv(Func(z), Deriv(z)));
		if (IsNear(z))
			break;
	}
	return IterationsToColor(i / iterationCount);
})");
		AutoReleasePtr<ID3DBlob> shaderByteCode;
		AutoReleasePtr<ID3DBlob> errorMessage;
		ThrowIfFailed(
			D3DCompile(
				psCode, strlen(psCode), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &shaderByteCode, &errorMessage),
			errorMessage ?
			reinterpret_cast<const char*>(errorMessage->GetBufferPointer()) :
			nullptr);
		ThrowIfFailed(m_device3D->CreatePixelShader(shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), NULL, &m_pixelShader));
		m_context3D->PSSetShader(m_pixelShader, NULL, 0);
	}
	void FractalWindow::MouseMove(WPARAM wparam, LPARAM lparam)
	{
		float2 cursor = { static_cast<float>(LOWORD(lparam)), static_cast<float>(HIWORD(lparam)) };

		bool needRedraw = false;
		if (wparam & MK_LBUTTON)
		{
			m_fractalParams.center.x -= 2.0f * (cursor.x - m_prevCursor.x) * m_fractalParams.screenAspect / (m_fractalParams.zoom * m_windowSize.x);
			m_fractalParams.center.y += 2.0f * (cursor.y - m_prevCursor.y) / (m_fractalParams.zoom * m_windowSize.y);
			needRedraw = true;
		}
#if MOVE_ZEROS
		if (wparam & MK_MBUTTON)
		{
			if (m_movingRootIndex >= 0)
			{
				m_fractalParams.roots[m_movingRootIndex] = ScreenToCoords(float2{ cursor.x + m_rootGrabOffset.x, cursor.y + m_rootGrabOffset.y });
			}
		}
#endif
		if (wparam & MK_RBUTTON)
		{
			m_fractalParams.steepness = ScreenToCoords(cursor);
			needRedraw = true;
		}
		if (needRedraw)
		{
			Redraw();
		}

#if MOVE_ZEROS
		SetCursor(RootUnderCursor(cursor) < 0 ? m_arrowCursor : m_handCursor);
#endif

		m_prevCursor = cursor;
	}
	void FractalWindow::MouseWheel(WPARAM wparam, LPARAM lparam)
	{
		(wparam & MK_CONTROL ? m_fractalParams.iterationCount : m_fractalParams.zoom) *=
			(GET_WHEEL_DELTA_WPARAM(wparam) < 0 ? 1.0f / 1.1f : 1.1f);
		Redraw();
	}
#if MOVE_ZEROS
	void FractalWindow::MiddleButtonDown(LPARAM lparam)
	{
		float2 cursor = { static_cast<float>(LOWORD(lparam)), static_cast<float>(HIWORD(lparam)) };
		m_movingRootIndex = RootUnderCursor(cursor);
		if (m_movingRootIndex >= 0)
		{
			float2 rootScreen = CoordsToScreen(m_fractalParams.roots[m_movingRootIndex]);
			m_rootGrabOffset = float2{ rootScreen.x - cursor.x, rootScreen.y - cursor.y };
		}
	}
#endif
	void FractalWindow::KeyDown(WPARAM wparam)
	{
		switch (wparam)
		{
		case 'R':
			DefaultFractalParams();
			break;
		}
	}
	void FractalWindow::Resize()
	{
		if (m_swapChain)
		{
			RECT rect;
			GetClientRect(m_window, &rect);
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			CreateWindowsizeDependentResources(max(1, width), max(1, height));
			m_windowSize.x = static_cast<float>(width);
			m_windowSize.y = static_cast<float>(height);
		}
	}
	void FractalWindow::Paint()
	{
		if (m_swapChain)
		{
			float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			m_context3D->ClearRenderTargetView(m_renderTargetView, color);
			D3D11_MAPPED_SUBRESOURCE resource;
			if (SUCCEEDED(m_context3D->Map(m_cbFractalParams, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource)))
			{
				memcpy(resource.pData, &m_fractalParams, sizeof(FractalParams));
				m_context3D->Unmap(m_cbFractalParams, 0);
			}
			m_context3D->DrawIndexed(6, 0, 0);
			m_swapChain->Present(1, 0);
		}
	}
	void FractalWindow::Redraw()
	{
		InvalidateRect(m_window, nullptr, false);
	}
	int FractalWindow::RootUnderCursor(float2 cursor)
	{
		for (int r = 0; r < static_cast<int>(m_fractalParams.rootCount); r++)
		{
			float2 root = CoordsToScreen(m_fractalParams.roots[r * 2]);
			if ((root.x - cursor.x) * (root.x - cursor.x) + (root.y - cursor.y) * (root.y - cursor.y) < 100)
				return r;
		}
		return -1;
	}
	void FractalWindow::DefaultRoots()
	{
		m_fractalParams.rootCount = 3;
		m_fractalParams.roots[0 * 2] = float2{ 1.0f, 0.0f };
		m_fractalParams.roots[1 * 2] = float2{ -0.5f, std::sqrt(3.0f) / 2.0f };
		m_fractalParams.roots[2 * 2] = float2{ -0.5f, -std::sqrt(3.0f) / 2.0f };
	}
	void FractalWindow::DefaultFractalParams()
	{
		m_fractalParams.center = { 0.0f, 0.0f };
		m_fractalParams.steepness = { 1.0f, 0.0f };
		m_fractalParams.zoom = 1.0f;
		m_fractalParams.iterationCount = 100.0f;
	}
	float2 FractalWindow::ScreenToCoords(float2 screen)
	{
		return float2{
			(screen.x / m_windowSize.x - 0.5f) * 2.0f * m_fractalParams.screenAspect / m_fractalParams.zoom + m_fractalParams.center.x,
			(screen.y / m_windowSize.y - 0.5f) * -2.0f / m_fractalParams.zoom + m_fractalParams.center.y
		};
	}
	float2 FractalWindow::CoordsToScreen(float2 coords)
	{
		return float2{
			((coords.x - m_fractalParams.center.x) * m_fractalParams.zoom / m_fractalParams.screenAspect * 0.5f + 0.5f) * m_windowSize.x,
			((coords.y - m_fractalParams.center.y) * m_fractalParams.zoom * -0.5f + 0.5f) * m_windowSize.y
		};
	}
	FractalWindow::FractalWindow() :
		m_window(NULL),
		m_viewport{ 0 },
#if MOVE_ZEROS
		m_movingRootIndex(-1),
#endif
		m_windowSize{ 0.0f, 0.0f },
		m_prevCursor{ 0.0f, 0.0f }
	{
		m_fractalParams.screenAspect = 1.0f;
		DefaultFractalParams();
		DefaultRoots();
	}
	FractalWindow::~FractalWindow()
	{
		DestroyWindow(m_window);
	}
	void FractalWindow::Init(const wchar_t* windowName, int x, int y, int width, int height)
	{
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(wc);
		wc.hInstance = GetModuleHandle(NULL);
#if !MOVE_ZEROS
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
#endif
		wc.lpszClassName = windowName;
		wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)->LPARAM {
			if (msg == WM_CREATE)
			{
				LRESULT(*wndProc)(HWND, UINT, WPARAM, LPARAM) = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)->LRESULT {
					return reinterpret_cast<FractalWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))->MessageHandler(hwnd, msg, wparam, lparam);
				};
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams));
				SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndProc));
				return 0;
			}
			return DefWindowProc(hwnd, msg, wparam, lparam);
		};
		RegisterClassEx(&wc);

#if MOVE_ZEROS
		m_arrowCursor = LoadCursor(NULL, IDC_ARROW);
		m_handCursor = LoadCursor(NULL, IDC_HAND);
#endif

		RECT rect = { x, y, x + width, y + height };
		DWORD style = WS_OVERLAPPEDWINDOW;
		DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_STATICEDGE;
		AdjustWindowRect(&rect, style, true);

		m_window = CreateWindowEx(exStyle, windowName, windowName, style,
			rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
			nullptr, nullptr, wc.hInstance, this);
		ShowWindow(m_window, SW_SHOWDEFAULT);
		InitGraphics(width, height);
		CreateWindowsizeDependentResources(max(1, width), max(1, height));
		CreateResources();
	}

	LRESULT FractalWindow::MessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_MOUSEMOVE:
			MouseMove(wparam, lparam);
			return 0;
		case WM_MOUSEWHEEL:
			MouseWheel(wparam, lparam);
			return 0;
#if MOVE_ZEROS
		case WM_MBUTTONDOWN:
			MiddleButtonDown(lparam);
			return 0;
#endif
		case WM_KEYDOWN:
			KeyDown(wparam);
			return 0;
		case WM_SIZE:
			Resize();
			return 0;
		case WM_PAINT:
			Paint();
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}