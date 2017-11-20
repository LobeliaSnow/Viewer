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
#include "Pipeline.hpp"

namespace Lobelia::Graphics {
	Pipeline::Pipeline(const std::string& blend_key, const std::string& sampler_key, const std::string& depth_stencil_key, const std::string& rasterizer_key, const std::string& vs_key, int vs_instance_count, ShaderLinkageInstance* vs_instances, const std::string& ps_key, int ps_instance_count, ShaderLinkageInstance* ps_instances) :blend(blend_key), sampler(sampler_key), depthStencil(depth_stencil_key), rasterizer(rasterizer_key), vs(vs_key), vsInstanceCount(vs_instance_count), vsInstances(vs_instances), ps(ps_key), psInstanceCount(ps_instance_count), psInstances(ps_instances) {}
	Pipeline::~Pipeline() = default;

	void Pipeline::BlendSet(const std::string& key) { isBlendChange = true; blend = key; }
	void Pipeline::SamplerSet(const std::string& key) { isSamplerChange = true; sampler = key; }
	void Pipeline::DepthStencilSet(const std::string& key) { isDepthStencilChange = true; depthStencil = key; }
	void Pipeline::RasterizerSet(const std::string& key) { isRasterizerChange = true; rasterizer = key; }
	void Pipeline::VertexShaderSet(const std::string& key, int instance_count, ShaderLinkageInstance* instances) {
		isVSChange = true;
		vs = key;
		vsInstanceCount = instance_count;
		vsInstances = instances;
	}
	void Pipeline::VertexShaderSet(int instance_count, ShaderLinkageInstance* instances) {
		isVSChange = true;
		vsInstanceCount = instance_count;
		vsInstances = instances;
	}
	void Pipeline::PixelShaderSet(const std::string& key, int instance_count, ShaderLinkageInstance* instances) {
		isPSChange = true;
		ps = key;
		psInstanceCount = instance_count;
		psInstances = instances;
	}
	void Pipeline::PixelShaderSet(int instance_count, ShaderLinkageInstance* instances) {
		isPSChange = true;
		psInstanceCount = instance_count;
		psInstances = instances;
	}

	VertexShader* Pipeline::GetVertexShader() {
		isVSChange = true;
		return ResourceBank<VertexShader>::Get(vs);
	}
	PixelShader* Pipeline::GetPixelShader() {
		isPSChange = true;
		return ResourceBank<PixelShader>::Get(ps);
	}
	void Pipeline::Activate(bool force_set) {
		if (force_set || isBlendChange)ResourceBank<BlendState>::Get(blend)->Set(force_set);
		if (force_set || isSamplerChange)ResourceBank<SamplerState>::Get(sampler)->Set(force_set);
		if (force_set || isDepthStencilChange)ResourceBank<DepthStencilState>::Get(depthStencil)->Set(force_set);
		if (force_set || isRasterizerChange)ResourceBank<RasterizerState>::Get(rasterizer)->Set(force_set);
		if (force_set || isVSChange)ResourceBank<VertexShader>::Get(vs)->Set(vsInstanceCount, vsInstances);
		if (force_set || isPSChange)ResourceBank<PixelShader>::Get(ps)->Set(psInstanceCount, psInstances);
		isBlendChange = isSamplerChange = isDepthStencilChange = isRasterizerChange = isVSChange = isPSChange = false;
		PipelineManager::pipeline = this;
	}
	Pipeline* PipelineManager::pipeline = nullptr;

	void PipelineManager::PipelineRegister(const std::string& key, Pipeline* pipeline) { ResourceBank<Pipeline>::Register(key, std::shared_ptr<Pipeline>(pipeline)); }
	Pipeline* PipelineManager::PipelineGet(const std::string& key) { return ResourceBank<Pipeline>::Get(key); }
	Pipeline* PipelineManager::PipelineGet() { return pipeline; }
	void PipelineManager::PipelineActivate(const std::string& key, bool force_set) { PipelineGet(key)->Activate(force_set); }
	void PipelineManager::PipelineActivate(bool force_set) { pipeline->Activate(force_set); }

}