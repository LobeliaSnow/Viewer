#include "Common/Common.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/Texture/Texture.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Graphics/Material/Material.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/ShaderBank.hpp"


namespace Lobelia::Graphics {
	Material::Material(const char* name, const char* texture_path) :name(name), texture(nullptr), visible(true), constantBuffer(std::make_unique<ConstantBuffer<Data>>(2, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS)) {
		try {
			TextureFileAccessor::Load(texture_path, texture);
			SetTexColor(0xFFFFFFFF);
		}
		catch (...) {
			throw;
		}
	}
	Material::~Material() = default;
	const std::string& Material::GetName() { return name; }
	Texture* Material::GetTexture() { return texture.get(); }
	void Material::SetDiffuseColor(const Math::Vector4& diffuse) { data.diffuse = diffuse; }
	void Material::SetAmbientColor(const Math::Vector4& ambient) { data.ambient = ambient; }
	void Material::SetSpecularColor(const Math::Vector4& specular) { data.specular = specular; }
	void Material::SetTexColor(Utility::Color tex_color) {
		data.texColor.x = tex_color.GetNormalizedA();
		data.texColor.y = tex_color.GetNormalizedR();
		data.texColor.z = tex_color.GetNormalizedG();
		data.texColor.w = tex_color.GetNormalizedB();
	}
	void Material::ChangeVisible(bool visible) { this->visible = visible; }
	bool Material::IsVisible() { return visible; }

	//いつでもテクスチャを切り替えれるように。
	void Material::ChangeTexture(std::shared_ptr<Texture>& texture) { this->texture = texture; }
	void Material::Set(bool texture, bool color) {
		//if (IsSet())return;
		if (this->texture&&texture)this->texture->Set(0, ShaderStageList::PS);
		if (color)constantBuffer->Activate(data);
	}
}