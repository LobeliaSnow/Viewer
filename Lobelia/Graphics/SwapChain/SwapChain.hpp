#pragma once
namespace Lobelia::Graphics {
	class SwapChain {
		friend class GammaCorrection;
	private:
		ComPtr<IDXGISwapChain> swapChain;
		std::shared_ptr<RenderTarget> renderTarget;
	private:
		void CreateSwapChain(Window* window, DXGI_SAMPLE_DESC sample_desc, int refresh_rate);
		void CreateSwapChain(HWND hwnd, const Math::Vector2& size, DXGI_SAMPLE_DESC sample_desc, int refresh_rate);
	public:
		SwapChain(Window* window, DXGI_SAMPLE_DESC sample_desc, DisplayInfo* info = nullptr);
		SwapChain(HWND hwnd, const Math::Vector2& size, DXGI_SAMPLE_DESC sample_desc, DisplayInfo* info = nullptr);
		~SwapChain();
		static DXGI_SAMPLE_DESC SearchLevelMSAA();
		RenderTarget* GetRenderTarget();
		void Clear(Utility::Color color);
		void Present();
	};
}