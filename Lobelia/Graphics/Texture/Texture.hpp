#pragma once
#include "../DirectXTex/DirectXTex.h"

#ifdef _DEBUG
#pragma comment(lib,"DirectXTexDebug.lib")
#else
#pragma comment(lib,"DirectXTexRelease.lib")
#endif

namespace Lobelia::Graphics {
	class Texture final {
		friend class TextureFileAccessor;
		friend class RenderTarget;
	private:
		Math::Vector2 size;
		ComPtr<ID3D11ShaderResourceView> view;
		ComPtr<ID3D11Texture2D> texture;
	private:
		//引数要検討
		void CreateTexture(DXGI_FORMAT format, UINT bind_flags, const DXGI_SAMPLE_DESC& sample);
		void CreateShaderResourceView(DXGI_FORMAT format);
	public:
		Texture(const Math::Vector2& size, DXGI_FORMAT format, UINT bind_flags, const DXGI_SAMPLE_DESC& sample);
		Texture(const ComPtr<ID3D11Texture2D>& texture);
		~Texture();
		ComPtr<ID3D11Texture2D>& Get();
		const Math::Vector2& GetSize();
		//TODO : セットを用意すること、その際掃除のことも気を付けること
		//↑レンダーターゲットとシェーダーリソースビュー両方同時にセットされてしまう状況になってしまうとまずいのでそれの回避
		void Set(int num_slot, ShaderStageList list);
		void Clean(int num_slot, ShaderStageList list);
	};
	//未来感じる
	//IWICBitmapDecoder::GetFrame
	//DirectX::ComputeNormalMap
	//DirectX::IsVideo
	class TextureFileAccessor final {
	public:
		enum class Extension {
			NO_SUPPORT = -1,
			PNG,
			JPG,
			TGA,
			BMP,
		};
	private:
		static void LoadFile(const wchar_t* file_path, Extension extension, DirectX::TexMetadata* meta, DirectX::ScratchImage& image);
	public:
		static Extension JudgeExtension(const std::string& file_path);
		static void Load(const char* file_path, std::shared_ptr<Texture>& texture);
		//保存機能追加予定
		static void Save(const char* file_path, Texture* texture);
		static void CreateNormalMap(Texture* src, Texture** normal, float amplitude);
	};
}