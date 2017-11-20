#include "Common/Common.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"

#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/RenderTarget/RenderTarget.hpp"
#include "Graphics/Texture/Texture.hpp"

#include <wincodec.h>
#include <wincodecsdk.h>
#pragma comment(lib, "WindowsCodecs.lib")
#include <locale.h>

namespace Lobelia::Graphics {
	Texture::Texture(const Math::Vector2& size, DXGI_FORMAT format, UINT bind_flags, const DXGI_SAMPLE_DESC& sample) :size(size) {
		HRESULT hr = S_OK;
		CreateTexture(format, bind_flags, sample);
		D3D11_TEXTURE2D_DESC desc = {};
		texture->GetDesc(&desc);
		if (desc.BindFlags&D3D11_BIND_SHADER_RESOURCE)	CreateShaderResourceView(desc.Format);
	}
	Texture::Texture(const ComPtr<ID3D11Texture2D>& texture) : texture(texture) {
		D3D11_TEXTURE2D_DESC desc = {};
		this->texture->GetDesc(&desc);
		size.x = static_cast<float>(desc.Width); size.y = static_cast<float>(desc.Height);
		if (desc.BindFlags&D3D11_BIND_SHADER_RESOURCE)	CreateShaderResourceView(desc.Format);
	}
	Texture::~Texture() = default;
	ComPtr<ID3D11Texture2D>&Texture::Get() { return texture; }
	void Texture::CreateTexture(DXGI_FORMAT format, UINT bind_flags, const DXGI_SAMPLE_DESC& sample) {
		HRESULT hr = S_OK;
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = size.Get().x;
		desc.Height = size.Get().y;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc = sample;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.BindFlags = bind_flags;
		hr = Device::Get()->CreateTexture2D(&desc, nullptr, texture.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("テクスチャの作成に失敗しました");
	}
	void Texture::CreateShaderResourceView(DXGI_FORMAT format) {
		D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc = {};
		srvdesc.Format = format;
		srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvdesc.Texture2D.MostDetailedMip = 0;
		srvdesc.Texture2D.MipLevels = 1;
		HRESULT hr = Device::Get()->CreateShaderResourceView(texture.Get(), &srvdesc, view.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("シェーダーリソースビューの作成に失敗");
	}

	const Math::Vector2& Texture::GetSize() { return size; }
	void Texture::Set(int num_slot, ShaderStageList list) {
		switch (list) {
		case ShaderStageList::VS:	Device::GetContext()->VSSetShaderResources(num_slot, 1, view.GetAddressOf());	break;
		case ShaderStageList::PS:	Device::GetContext()->PSSetShaderResources(num_slot, 1, view.GetAddressOf());	break;
		case ShaderStageList::HS:	Device::GetContext()->HSSetShaderResources(num_slot, 1, view.GetAddressOf());	break;
		case ShaderStageList::GS:	Device::GetContext()->GSSetShaderResources(num_slot, 1, view.GetAddressOf());	break;
		case ShaderStageList::DS:	Device::GetContext()->DSSetShaderResources(num_slot, 1, view.GetAddressOf());	break;
		case ShaderStageList::CS:	Device::GetContext()->CSSetShaderResources(num_slot, 1, view.GetAddressOf());	break;
		default:	STRICT_THROW("範囲外の値です");
		}
	}
	void Texture::Clean(int num_slot, ShaderStageList list) {
		ID3D11ShaderResourceView* renderTransform = nullptr;
		switch (list) {
		case ShaderStageList::VS:	Device::GetContext()->VSSetShaderResources(num_slot, 1, &renderTransform);	break;
		case ShaderStageList::PS:	Device::GetContext()->PSSetShaderResources(num_slot, 1, &renderTransform);	break;
		case ShaderStageList::HS:	Device::GetContext()->HSSetShaderResources(num_slot, 1, &renderTransform);	break;
		case ShaderStageList::GS:	Device::GetContext()->GSSetShaderResources(num_slot, 1, &renderTransform);	break;
		case ShaderStageList::DS:	Device::GetContext()->DSSetShaderResources(num_slot, 1, &renderTransform);	break;
		case ShaderStageList::CS:	Device::GetContext()->CSSetShaderResources(num_slot, 1, &renderTransform);	break;
		default:	STRICT_THROW("範囲外の値です");
		}
	}
	TextureFileAccessor::Extension TextureFileAccessor::JudgeExtension(const std::string& file_path) {
		std::string extension = Utility::FilePathControl::GetExtension(file_path);
		std::locale locale;
		std::transform(extension.cbegin(), extension.cend(), extension.begin(), tolower);
		if (extension == ".png")return Extension::PNG;
		else if (extension == ".jpg")return Extension::JPG;
		else if (extension == ".tga")return Extension::TGA;
		else if (extension == ".bmp")return Extension::BMP;
		return Extension::NO_SUPPORT;
	}
	void TextureFileAccessor::LoadFile(const wchar_t* file_path, Extension extension, DirectX::TexMetadata* meta, DirectX::ScratchImage& image) {
		HRESULT hr = S_OK;
		switch (extension) {
		case Extension::PNG:
		case Extension::BMP:
		case Extension::JPG:
			hr = DirectX::LoadFromWICFile(file_path, DirectX::WIC_FLAGS_ALL_FRAMES, meta, image);
			break;
		case Extension::TGA:
			hr = DirectX::LoadFromTGAFile(file_path, meta, image);
			break;
		case Extension::NO_SUPPORT:
		default:	STRICT_THROW("対応していないテクスチャフォーマットです");		break;
		}
		if (FAILED(hr))STRICT_THROW("テクスチャが読み込めませんでした");
	}

	void TextureFileAccessor::Load(const char* file_path, Texture** texture) {
		if (Utility::FilePathControl::GetFilename(file_path) == ".")return;
		if (ResourceBank<Texture>::IsExist(file_path)) {
			*texture = ResourceBank<Texture>::Get(file_path);
			return;
		}
		if (strcmp(file_path, "") == 0) {
			DXGI_SAMPLE_DESC msaa = { 1,0 };
			*texture = ResourceBank<Texture>::Factory(file_path, Math::Vector2(1, 1), DXGI_FORMAT_R32G32B32A32_FLOAT, D3D11_BIND_SHADER_RESOURCE, msaa);
			return;
		}
		Extension extension = JudgeExtension(file_path);
		DirectX::TexMetadata meta = {};
		DirectX::ScratchImage image = {};
		std::wstring filePath = Utility::ConverteWString(file_path);
		LoadFile(filePath.c_str(), extension, &meta, image);
		ComPtr<ID3D11Texture2D> renderTransform;
		HRESULT hr = DirectX::CreateTexture(Device::Get().Get(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), reinterpret_cast<ID3D11Resource**>(renderTransform.GetAddressOf()));
		if (FAILED(hr))STRICT_THROW("テクスチャの作成に失敗しました");
		*texture = ResourceBank<Texture>::Factory(file_path, renderTransform);
	}
	void TextureFileAccessor::Save(const char* file_path, Texture* texture) {
		HRESULT 	hr = S_OK;
		DirectX::ScratchImage image = {};
		hr = DirectX::CaptureTexture(Device::Get().Get(), Device::GetContext().Get(), texture->texture.Get(), image);
		if (FAILED(hr))STRICT_THROW("テクスチャを取得できませんでした");
		Extension extension = JudgeExtension(file_path);
		GUID guid = {};
		switch (extension) {
		case Extension::PNG:	guid = GUID_ContainerFormatPng;		break;
		case Extension::BMP:	guid = GUID_ContainerFormatBmp;		break;
		case Extension::JPG:	guid = GUID_ContainerFormatJpeg;		break;
			/*default:	HostConsole::GetInstance()->SetLog("対応していない拡張子です"); return;*/
		}
		hr = DirectX::SaveToWICFile(*image.GetImages(), 0, guid, Utility::ConverteWString(file_path).c_str());
		if (FAILED(hr))STRICT_THROW("保存に失敗しました");
	}
	void TextureFileAccessor::CreateNormalMap(Texture* src, Texture** normal, float amplitude) {
		HRESULT 	hr = S_OK;
		DirectX::ScratchImage image = {};
		hr = DirectX::CaptureTexture(Device::Get().Get(), Device::GetContext().Get(), src->texture.Get(), image);
		if (FAILED(hr))STRICT_THROW("テクスチャを取得できませんでした");
		DirectX::ScratchImage srcImage = {};
		//ちょっと調べる
		//CNMAP_DEFAULT  DXGI_FORMAT_R8G8B8A8_UNORM
		hr = DirectX::ComputeNormalMap(*image.GetImages(), DirectX::CNMAP_DEFAULT, amplitude, DXGI_FORMAT_R8G8B8A8_UNORM, srcImage);
		if (FAILED(hr))STRICT_THROW("ノーマルマップの計算に失敗");
		DXGI_SAMPLE_DESC msaa = { 1,0 };
		//ここ何とかしような
		*normal = new Texture(src->GetSize(), DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, msaa);
		hr = DirectX::CreateTexture(Device::Get().Get(), srcImage.GetImages(), srcImage.GetImageCount(), srcImage.GetMetadata(), reinterpret_cast<ID3D11Resource**>((*normal)->Get().GetAddressOf()));
		if (FAILED(hr))STRICT_THROW("テクスチャの作成に失敗しました");
	}

}