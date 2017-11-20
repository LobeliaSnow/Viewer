#include "Common/Common.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Exception/Exception.hpp"

namespace Lobelia::Graphics {
	void ShaderCompiler::ReadCSO(const char* file_path, int* size, BYTE* buffer) {
		std::unique_ptr<Utility::FileController> fc = std::make_unique<Utility::FileController>();
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		*size = static_cast<int>(fc->GetSize());
		buffer = new BYTE[*size];
		fc->Rewind();
		fc->Read(buffer, *size, *size, 1);
		fc->Close();
	}
	void ShaderCompiler::CompileShaderFromFile(const char* file_path, const char* entry_point, const char* shader_model, ID3DBlob** blob) {
		try {
			HRESULT hr = S_OK;
			ComPtr<ID3DBlob> error = {};
			//これを引数でとるか否かは要調整
			DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
			shaderFlags |= D3DCOMPILE_DEBUG;
#else
			shaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
			hr = D3DCompileFromFile(Utility::ConverteWString(file_path).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point, shader_model, shaderFlags, 0, blob, &error);
			if (FAILED(hr)) {
				std::stringstream errorMes;
				if (!error)THROW;
				errorMes << "\n//////////////シェーダーコンパイルエラー！//////////////\n\n" << reinterpret_cast<const char*>(error->GetBufferPointer()) << "\n////////////////////////////////////////////////////////\n" << "シェーダーのコンパイルに失敗\n";
				STRICT_THROW(errorMes.str());
			}
		}
		catch (const Exception& exception) {
			exception.BoxMessage();
		}
	}
	ShaderLinkageInstance::ShaderLinkageInstance() = default;
	ShaderLinkageInstance::~ShaderLinkageInstance() = default;
	const std::string& ShaderLinkageInstance::GetName() { return name; }

	ShaderLinkage::ShaderLinkage() { Device::Get()->CreateClassLinkage(classLinkage.GetAddressOf()); }
	ShaderLinkage::~ShaderLinkage() = default;
	void ShaderLinkage::CreateInstance(const char* instance_name, ShaderLinkageInstance* instance) {
		instance->name = instance_name;
		if (FAILED(classLinkage->CreateClassInstance(instance_name, 0, 0, 0, 0, instance->instance.GetAddressOf())))STRICT_THROW("インスタンス生成に失敗");
	}
	void ShaderLinkage::GetInstance(const char* instance_name, int instance_index, ShaderLinkageInstance* instance) {
		instance->name = instance_name;
		classLinkage->GetClassInstance(instance_name, instance_index, instance->instance.GetAddressOf());
	}

	Shader::Shader(const char* file_path, const char* entry_point, const char* shader_model, ShaderLinkage* linkage) :linkage(linkage) {
		ShaderCompiler::CompileShaderFromFile(file_path, entry_point, shader_model, blob.GetAddressOf());
	}
	Shader::~Shader() = default;
	static void ShaderSet(std::function<void(int, ID3D11ClassInstance**)> functor, int instance_count, ShaderLinkageInstance* instances) {
		ID3D11ClassInstance** renderTransform = nullptr;
		if (instance_count > 0) {
			renderTransform = new  ID3D11ClassInstance*[instance_count];
			for (int i = 0; i < instance_count; i++) {
				renderTransform[i] = instances->instance.Get();
			}
		}
		functor(instance_count, renderTransform);
		Utility::SafeDelete(renderTransform);
	}
	ShaderLinkage* Shader::GetLinkage() { return linkage.get(); }
	VertexShader::VertexShader(const char* file_path, const char* entry_point, Model shader_model, bool use_linkage) :Shader(file_path, entry_point, ConverteShaderModelString(shader_model).c_str(), use_linkage ? new ShaderLinkage() : nullptr) {
		if (FAILED(Device::Get()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), linkage ? linkage->classLinkage.Get() : nullptr, vs.GetAddressOf())))STRICT_THROW("頂点シェーダーの作成に失敗");
	}
	VertexShader::~VertexShader() = default;
	//templateのほうが良いかも。
	std::string VertexShader::ConverteShaderModelString(Model shader_model) {
		switch (shader_model) {
		case Model::VS_2_0:		return "vs_2_0";
		case Model::VS_3_0:		return "vs_3_0";
		case Model::VS_4_0:		return "vs_4_0";
		case Model::VS_4_1:		return "vs_4_1";
		case Model::VS_5_0:		return "vs_5_0";
		case Model::VS_5_1:		return "vs_5_1";
		default:STRICT_THROW("範囲外の値です");
		}
		return "";
	}

	void VertexShader::Set(int instance_count, ShaderLinkageInstance* instances) {
		ShaderSet([this](int instance_count, ID3D11ClassInstance** renderTransform) { Device::GetContext()->VSSetShader(vs.Get(), renderTransform, instance_count); }, instance_count, instances);
	}

	PixelShader::PixelShader(const char* file_path, const char* entry_point, Model shader_model, bool use_linkage) :Shader(file_path, entry_point, ConverteShaderModelString(shader_model).c_str(), use_linkage ? new ShaderLinkage() : nullptr) {
		if (FAILED(Device::Get()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), linkage ? linkage->classLinkage.Get() : nullptr, ps.GetAddressOf())))STRICT_THROW("ピクセルシェーダーの作成に失敗");
	}
	PixelShader::~PixelShader() = default;
	std::string PixelShader::ConverteShaderModelString(Model shader_model) {
		switch (shader_model) {
		case Model::PS_2_0:		return "ps_2_0";
		case Model::PS_3_0:		return "ps_3_0";
		case Model::PS_4_0:		return "ps_4_0";
		case Model::PS_4_1:		return "ps_4_1";
		case Model::PS_5_0:		return "ps_5_0";
		case Model::PS_5_1:		return "ps_5_1";
		default:STRICT_THROW("範囲外の値です");
		}
		return "";
	}
	void PixelShader::Set(int instance_count, ShaderLinkageInstance* instances) {
		ShaderSet([this](int instance_count, ID3D11ClassInstance** renderTransform) { Device::GetContext()->PSSetShader(ps.Get(), renderTransform, instance_count); }, instance_count, instances);
	}

}