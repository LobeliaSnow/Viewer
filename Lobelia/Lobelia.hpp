#pragma once
#include "Common/Common.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Exception/Exception.hpp"
#include "Graphics/Transform/Transform.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Graphics/DisplayInfo/DisplayInfo.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/RenderState/RenderState.hpp"
#include "Graphics/View/View.hpp"
#include "Graphics/RenderTarget/RenderTarget.hpp"
#include "Graphics/Texture/Texture.hpp"
//#include "Movie/Movie.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/ShaderBank.hpp"
#include "Graphics/Shader/Reflection/Reflection.hpp"
#include "Graphics/InputLayout/InputLayout.hpp"
#include "Graphics/Material/Material.hpp"
#include "Graphics/Mesh/Mesh.hpp"
#include "Graphics/Transform/Transform.hpp"
#include "Graphics/Sprite/Sprite.hpp"
#include "Graphics/Model/Model.hpp"
#include "Graphics/Environment/Environment.hpp"
#include "Graphics/SwapChain/SwapChain.hpp"
#include "Graphics/SwapChain/ToneCurve/ToneCurve.hpp"
#include "Graphics/Renderer/Renderer.hpp"
#include "Graphics/Pipeline/Pipeline.hpp"

namespace Lobelia {
	inline void CreateStencilStatePreset() {
		Graphics::StencilDesc desc0 = {};
		desc0.readMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc0.writeMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		desc0.front.depthFail = Graphics::StencilOperation::KEEP;
		desc0.front.faile = Graphics::StencilOperation::KEEP;
		desc0.front.pass = Graphics::StencilOperation::INCR_CLAMP;
		desc0.front.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		desc0.back.depthFail = Graphics::StencilOperation::KEEP;
		desc0.back.faile = Graphics::StencilOperation::KEEP;
		desc0.back.pass = Graphics::StencilOperation::DECR_CLAMP;
		desc0.back.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		Graphics::RenderStateBank::DepthStencilFactory(STENCIL_STATE_WRITE_Z_OFF, Graphics::DepthPreset::ALWAYS, false, desc0, true);
		Graphics::RenderStateBank::DepthStencilFactory(STENCIL_STATE_WRITE_Z_ON, Graphics::DepthPreset::ALWAYS, true, desc0, true);
		Graphics::StencilDesc desc1 = {};
		desc1.readMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc1.writeMask = 0x00;
		desc1.front.depthFail = Graphics::StencilOperation::KEEP;
		desc1.front.faile = Graphics::StencilOperation::ZERO;
		desc1.front.pass = Graphics::StencilOperation::KEEP;
		desc1.front.testFunc = Graphics::StencilFunction::LESS_EQUAL;
		desc1.back.depthFail = Graphics::StencilOperation::KEEP;
		desc1.back.faile = Graphics::StencilOperation::ZERO;
		desc1.back.pass = Graphics::StencilOperation::KEEP;
		desc1.back.testFunc = Graphics::StencilFunction::LESS_EQUAL;
		Graphics::RenderStateBank::DepthStencilFactory(STENCIL_STATE_READ_Z_OFF, Graphics::DepthPreset::ALWAYS, false, desc1, true);
		Graphics::RenderStateBank::DepthStencilFactory(STENCIL_STATE_READ_Z_ON, Graphics::DepthPreset::ALWAYS, true, desc1, true);
		//User
		Graphics::StencilDesc desc2 = {};
		desc2.readMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc2.writeMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		desc2.front.depthFail = Graphics::StencilOperation::KEEP;
		desc2.front.faile = Graphics::StencilOperation::KEEP;
		desc2.front.pass = Graphics::StencilOperation::INVERT;
		desc2.front.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		desc2.back.depthFail = Graphics::StencilOperation::KEEP;
		desc2.back.faile = Graphics::StencilOperation::KEEP;
		desc2.back.pass = Graphics::StencilOperation::INVERT;
		desc2.back.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		Graphics::RenderStateBank::DepthStencilFactory("StencilWrite", Graphics::DepthPreset::ALWAYS, false, desc2, true);
		Graphics::StencilDesc desc3 = {};
		desc3.readMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc3.writeMask = 0x00;
		desc3.front.depthFail = Graphics::StencilOperation::KEEP;
		desc3.front.faile = Graphics::StencilOperation::ZERO;
		desc3.front.pass = Graphics::StencilOperation::KEEP;
		desc3.front.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		desc3.back.depthFail = Graphics::StencilOperation::KEEP;
		desc3.back.faile = Graphics::StencilOperation::ZERO;
		desc3.back.pass = Graphics::StencilOperation::KEEP;
		desc3.back.testFunc = Graphics::StencilFunction::GREATER_EQUAL;
		Graphics::RenderStateBank::DepthStencilFactory("StencilRead", Graphics::DepthPreset::ALWAYS, false, desc3, true);

	}
	inline void CreateDefaultPipeline() {
		//共通部
		Graphics::RenderStateBank::BlendFactory("Copy", Graphics::BlendPreset::COPY, true, true);
		Graphics::RenderStateBank::BlendFactory("Add", Graphics::BlendPreset::ADD, true, true);
		Graphics::RenderStateBank::BlendFactory("Copy NoAlphaCoverage", Graphics::BlendPreset::COPY, true, false);
		Graphics::RenderStateBank::BlendFactory("Add NoAlphaCoverage", Graphics::BlendPreset::ADD, true, false);
		Graphics::RenderStateBank::SamplerFactory("Point", Graphics::SamplerPreset::POINT);
		//2D用パイプライン構築
		//Zバッファ、ステンシルテストともにオフ
		Graphics::RenderStateBank::DepthStencilFactory(STENCIL_STATE_OFF_Z_OFF, Graphics::DepthPreset::ALWAYS, false, {}, false);
		Graphics::RenderStateBank::RasterizerFactory("Cull Back", Graphics::RasterizerPreset::BACK);
		Graphics::PixelShader* ps = ResourceBank<Graphics::PixelShader>::Factory(DEFAULT_PIXEL_SHADER_2D, "Data/ShaderFile/2D/PS.hlsl", "Main2D", Graphics::PixelShader::Model::PS_5_0, true);
		//リソースバンクいらない。
		ResourceBank<Graphics::ShaderLinkageInstance>::Factory(PIXEL_INSTANCE_NAME_TEXTURE);
		ResourceBank<Graphics::ShaderLinkageInstance>::Factory(PIXEL_INSTANCE_NAME_VERTEX);
		ResourceBank<Graphics::ShaderLinkageInstance>::Factory(PIXEL_INSTANCE_NAME_INVERT_TEXTURE);
		ResourceBank<Graphics::ShaderLinkageInstance>::Factory(PIXEL_INSTANCE_NAME_GRAYSCALE_TEXTURE);
		ResourceBank<Graphics::ShaderLinkageInstance>::Factory(PIXEL_INSTANCE_NAME_SEPIA_TEXTURE);
		ps->GetLinkage()->CreateInstance(PIXEL_INSTANCE_NAME_TEXTURE, ResourceBank<Graphics::ShaderLinkageInstance>::Get(PIXEL_INSTANCE_NAME_TEXTURE));
		ps->GetLinkage()->CreateInstance(PIXEL_INSTANCE_NAME_VERTEX, ResourceBank<Graphics::ShaderLinkageInstance>::Get(PIXEL_INSTANCE_NAME_VERTEX));
		ps->GetLinkage()->CreateInstance(PIXEL_INSTANCE_NAME_INVERT_TEXTURE, ResourceBank<Graphics::ShaderLinkageInstance>::Get(PIXEL_INSTANCE_NAME_INVERT_TEXTURE));
		ps->GetLinkage()->CreateInstance(PIXEL_INSTANCE_NAME_GRAYSCALE_TEXTURE, ResourceBank<Graphics::ShaderLinkageInstance>::Get(PIXEL_INSTANCE_NAME_GRAYSCALE_TEXTURE));
		ps->GetLinkage()->CreateInstance(PIXEL_INSTANCE_NAME_SEPIA_TEXTURE, ResourceBank<Graphics::ShaderLinkageInstance>::Get(PIXEL_INSTANCE_NAME_SEPIA_TEXTURE));
		//2DSprite用パイプライン構築
		ResourceBank<Graphics::VertexShader>::Factory(DEFAULT_VERTEX_SHADER_SPRITE, "Data/ShaderFile/2D/VS.hlsl", "Main2D", Graphics::VertexShader::Model::VS_5_0);
		//2Dインスタンシング用
		ResourceBank<Graphics::VertexShader>::Factory(DEFAULT_VERTEX_SHADER_BATCH, "Data/ShaderFile/2D/VS.hlsl", "Main2DInst", Graphics::VertexShader::Model::VS_5_0);

		//スプライト用パイプライン構築
		Graphics::PipelineManager::PipelineRegister(DEFAULT_PIPELINE_SPRITE, new Graphics::Pipeline("Copy", "Point", STENCIL_STATE_OFF_Z_OFF, "Cull Back", DEFAULT_VERTEX_SHADER_SPRITE, 0, nullptr, DEFAULT_PIXEL_SHADER_2D, 1, PIXEL_INSTANCE_TEXTURE));
		//バッチ用パイプライン構築
		Graphics::PipelineManager::PipelineRegister(DEFAULT_PIPELINE_BATCH, new Graphics::Pipeline("Copy", "Point", STENCIL_STATE_OFF_Z_OFF, "Cull Back", DEFAULT_VERTEX_SHADER_BATCH, 0, nullptr, DEFAULT_PIXEL_SHADER_2D, 1, PIXEL_INSTANCE_TEXTURE));
		//3D用パイプライン構築
		Graphics::RenderStateBank::SamplerFactory("Anisotropic", Graphics::SamplerPreset::ANISOTROPIC);
		//Zバッファオン ステンシルオフ
		Graphics::RenderStateBank::DepthStencilFactory(STENCIL_STATE_OFF_Z_ON, Graphics::DepthPreset::ALWAYS, true, {}, false);
		//FBX用
		Graphics::RenderStateBank::RasterizerFactory("Cull Front", Graphics::RasterizerPreset::FRONT);
		//スタティックメッシュ用
		Graphics::ShaderBank::Register<Graphics::VertexShader>(DEFAULT_VERTEX_SHADER_STATIC_MODEL, "Data/Shaderfile/3D/VS.hlsl", "Main3DNoSkin", Graphics::VertexShader::Model::VS_5_0);
		//スキニング用
		Graphics::ShaderBank::Register<Graphics::VertexShader>(DEFAULT_VERTEX_SHADER_DYNAIMC_MODEL, "Data/Shaderfile/3D/VS.hlsl", "Main3D", Graphics::VertexShader::Model::VS_5_0);
		Graphics::ShaderBank::Register<Graphics::PixelShader>(DEFAULT_PIXEL_SHADER_3D, "Data/Shaderfile/3D/PS.hlsl", "Main3D", Graphics::PixelShader::Model::PS_5_0);

		Graphics::PipelineManager::PipelineRegister(DEFAULT_PIPELINE_STATIC_MODEL, new Graphics::Pipeline("Copy NoAlphaCoverage", "Anisotropic", STENCIL_STATE_OFF_Z_ON, "Cull Front", DEFAULT_VERTEX_SHADER_STATIC_MODEL, 0, nullptr, DEFAULT_PIXEL_SHADER_3D, 0, nullptr));
		Graphics::PipelineManager::PipelineRegister(DEFAULT_PIPELINE_DYNAMIC_MODEL, new Graphics::Pipeline("Copy NoAlphaCoverage", "Anisotropic", STENCIL_STATE_OFF_Z_ON, "Cull Front", DEFAULT_VERTEX_SHADER_DYNAIMC_MODEL, 0, nullptr, DEFAULT_PIXEL_SHADER_3D, 0, nullptr));
	}
	inline void Bootup() {
		srand(s_cast<unsigned>(time(NULL)));
		//if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))STRICT_THROW("COMの初期化に失敗しました");
		//グラフィックドライバ周り
		Graphics::GraphicDriverInfoList::Bootup();
		int driverCount = Graphics::GraphicDriverInfoList::GetGraphicDriverCount();
		Graphics::GraphicDriverInfo* driver = nullptr;
		for (int i = 0; i < driverCount; i++) {
			Graphics::GraphicDriverInfo* info = Graphics::GraphicDriverInfoList::GetDriver(i).get();
			if (!driver)driver = info;
			else if (info->GetVideoMemory() > driver->GetVideoMemory())driver = info;
		}
		//デバイス作成
		Graphics::Device::Create(D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT, driver);
		Graphics::GraphicDriverInfoList::Shutdown();
		CreateDefaultPipeline();
		CreateStencilStatePreset();

	}

	inline void Shutdown() {
		//Movie::MovieSystem::Shutdown();
		//CoUninitialize();
	}
}
