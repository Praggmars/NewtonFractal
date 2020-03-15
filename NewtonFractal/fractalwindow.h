#pragma once

#include "common.h"
#define MOVE_ZEROS false

namespace fractal
{
	struct float2
	{
		float x, y;
	};

	struct alignas(alignof(float)) FractalParams
	{
		float2 center;
		float2 steepness;
		float screenAspect;
		float zoom;
		float iterationCount;
		unsigned rootCount;
		float2 roots[32];	// FIXME: index 1 root could not be read in pixel shader
	};

	class FractalWindow
	{
		HWND m_window;

		AutoReleasePtr<ID3D11Device> m_device3D;
		AutoReleasePtr<ID3D11DeviceContext> m_context3D;
		AutoReleasePtr<IDXGISwapChain> m_swapChain;
		AutoReleasePtr<ID3D11RenderTargetView> m_renderTargetView;
		AutoReleasePtr<ID3D11RasterizerState> m_rasterizerState;
		AutoReleasePtr<ID3D11Texture2D> m_depthStencilBuffer;
		AutoReleasePtr<ID3D11DepthStencilView> m_depthStencilView;
		AutoReleasePtr<ID3D11DepthStencilState> m_depthStencilState;
		D3D11_VIEWPORT m_viewport;

		AutoReleasePtr<ID3D11VertexShader> m_vertexShader;
		AutoReleasePtr<ID3D11PixelShader> m_pixelShader;
		AutoReleasePtr<ID3D11Buffer> m_vertexBuffer;
		AutoReleasePtr<ID3D11Buffer> m_indexBuffer;
		AutoReleasePtr<ID3D11InputLayout> m_inputLayout;
		AutoReleasePtr<ID3D11Buffer> m_cbFractalParams;

		FractalParams m_fractalParams;

#if MOVE_ZEROS
		HCURSOR m_arrowCursor;
		HCURSOR m_handCursor;
		
		int m_movingRootIndex;
		float2 m_rootGrabOffset;
#endif

		float2 m_windowSize;
		float2 m_prevCursor;

	private:
		void InitGraphics(int width, int height);
		void CreateWindowsizeDependentResources(int width, int height);
		void CreateResources();
		void CreateVertexShader();
		void CreatePixelShader();

		void MouseMove(WPARAM wparam, LPARAM lparam);
		void MouseWheel(WPARAM wparam, LPARAM lparam);
#if MOVE_ZEROS
		void MiddleButtonDown(LPARAM lparam);
#endif
		void KeyDown(WPARAM wparam);
		void Resize();
		void Paint();
		void Redraw();

		int RootUnderCursor(float2 cursor);

		void DefaultRoots();
		void DefaultFractalParams();
		float2 ScreenToCoords(float2 screen);
		float2 CoordsToScreen(float2 coords);

	public:
		FractalWindow();
		~FractalWindow();
		void Init(const wchar_t* windowName, int x, int y, int width, int height);

		LRESULT MessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	};
}