#include "Common/Common.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/Reflection/Reflection.hpp"
#include "Graphics/InputLayout/InputLayout.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Graphics/Texture/Texture.hpp"
#include "Graphics/Material/Material.hpp"
#include "Graphics/Mesh/Mesh.hpp"
#include "Graphics/Transform/Transform.hpp"
#include "Graphics/Sprite/Sprite.hpp"
#include "Graphics/Renderer/Renderer.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Graphics {
	void Renderere::RegistSprite(Sprite* sprite, const Transform2D& transform) {
		sprites.push_back(std::pair<Sprite*, Transform2D>(sprite, transform));
	}

}