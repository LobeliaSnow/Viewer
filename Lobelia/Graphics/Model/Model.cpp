#include "Common/Common.hpp"
#include "Graphics/Origin/Origin.hpp"
#include "Exception/Exception.hpp"
#include "Graphics/ConstantBuffer/ShaderStageList.hpp"
#include "Graphics/BufferCreator/BufferCreator.h"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Graphics/GraphicDriverInfo/GraphicDriverInfo.hpp"
#include "Graphics/Device/Device.hpp"
#include "Graphics/Mesh/Mesh.hpp"
#include "Graphics/Shader/Shader.hpp"
#include "Graphics/Shader/ShaderBank.hpp"
#include "Graphics/InputLayout/InputLayout.hpp"
#include "Graphics/Texture/Texture.hpp"
#include "Graphics/Material/Material.hpp"
#include "Graphics/Shader/Reflection/Reflection.hpp"
#include "Graphics/Model/Model.hpp"

#include "Graphics/RenderState/RenderState.hpp"
#include "Graphics/Pipeline/Pipeline.hpp"
//結局ソースが汚くなってしまった。。。
//アニメーションは悪くない
//サブセットの管理をきれいにする
namespace Lobelia::Graphics {
	Animation::Animation(const char* file_path) :constantBuffer(std::make_unique<ConstantBuffer<Constant>>(3, ShaderStageList::VS)), time(0.0f) {
		std::unique_ptr<Utility::FileController> fc = std::make_unique<Utility::FileController>();
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		fc->Read(animationName, sizeof(char) * 32, sizeof(char) * 32, 1);
		fc->Read(&framePerCount, sizeof(int), sizeof(int), 1);
		fc->Read(&frameCount, sizeof(int), sizeof(int), 1);
		fc->Read(&meshCount, sizeof(int), sizeof(int), 1);
		keyFrames.resize(meshCount);
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			fc->Read(&clusterCount, sizeof(int), sizeof(int), 1);
			keyFrames[meshIndex].resize(clusterCount);
			for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
				keyFrames[meshIndex][clusterIndex].resize(frameCount);
				fc->Read(keyFrames[meshIndex][clusterIndex].data(), sizeof(DirectX::XMFLOAT4X4)* frameCount, sizeof(DirectX::XMFLOAT4X4), frameCount);
			}
		}
		fc->Close();
	}
	Animation::~Animation() = default;
	void Animation::AddElapsedTime(float time) {
		this->time += time;
		//よく考えること
		int animationMax = (frameCount - 1)*(1000 / framePerCount);
		if (this->time >= animationMax)this->time -= animationMax;
	}
	void Animation::Activate(int meshIndex) {
		//補間等ごちゃごちゃしないといけない
		for (int i = 0; i < clusterCount; i++) {
			DirectX::XMMATRIX renderTransform = DirectX::XMLoadFloat4x4(&keyFrames[meshIndex][i][static_cast<int>(time / 16)]);
			renderTransform = DirectX::XMMatrixTranspose(renderTransform);
			//本当はここで補間
			DirectX::XMStoreFloat4x4(&buffer.keyFrame[i], renderTransform);
		}
		constantBuffer->Activate(buffer);
	}
	Model::Model() : animationIndex(-1), pos(), scale(1.0f, 1.0f, 1.0f), euler(), world(DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity())) {
		VertexShader* vs = ShaderBank::Get<VertexShader>(DEFAULT_VERTEX_SHADER_STATIC_MODEL);
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs);
		inputLayout = std::make_unique<InputLayout>(vs, reflector.get());
		//std::unique_ptr<ConstantBuffer<DirectX::XMMATRIX>> world;
		constantBuffer = std::make_unique<ConstantBuffer<DirectX::XMMATRIX>>(1, Graphics::ShaderStageList::VS | Graphics::ShaderStageList::PS);
	}

	Model::Model(const Desc& desc) :Model() {
		Construction(desc);
	}
	Model::Model(const char* file_path) : Model() {
		std::unique_ptr<Utility::FileController> fc = std::make_unique<Utility::FileController>();
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		int meshCount = 0;
		fc->Read(&meshCount, sizeof(int), sizeof(int), 1);
		int vertexCount = 0;
		std::vector<Model::ReadVertex> vertices;
		std::vector<std::string> verticesMaterialNames;
		std::vector<int> delimiters(meshCount);
		//Mesh Read
		for (int i = 0; i < meshCount; i++) {
			int vertexCountLocal = -1;
			fc->Read(&vertexCountLocal, sizeof(int), sizeof(int), 1);
			vertexCount += vertexCountLocal;
			delimiters[i] = vertexCountLocal;
			std::vector<Model::ReadVertex> verticesLocal(vertexCountLocal);
			int a = static_cast<int>(fc->Read(verticesLocal.data(), sizeof(Model::ReadVertex)*vertexCountLocal, sizeof(Model::ReadVertex), vertexCountLocal));
			vertices.insert(vertices.end(), verticesLocal.begin(), verticesLocal.end());
			char tempMaterial[64] = {};
			fc->Read(tempMaterial, 64, 64, 1);
			verticesMaterialNames.push_back(tempMaterial);
		}
		int materialCount = -1;
		fc->Read(&materialCount, sizeof(int), sizeof(int), 1);
		std::vector<std::string> materialNames(materialCount);
		std::vector<std::string> textureNames(materialCount);
		std::string parentPath = Utility::FilePathControl::GetParentDirectory(file_path);
		std::vector<Math::Vector3> diffuses;
		std::vector<Math::Vector3> ambients;
		std::vector<Math::Vector3> speculars;

		//Material Read
		for (int i = 0; i < materialCount; i++) {
			char tempMaterial[64] = {};
			char tempTexture[256] = {};
			fc->Read(tempMaterial, 64, 64, 1);
			fc->Read(tempTexture, 256, 256, 1);
			materialNames[i] = tempMaterial;
			textureNames[i] = tempTexture;
			if (!parentPath.empty())	textureNames[i] = parentPath + "\\" + textureNames[i];
			Math::Vector3 diffuse, ambient, specular;
			fc->Read(&diffuse, sizeof(Math::Vector3), sizeof(Math::Vector3), 1);
			fc->Read(&ambient, sizeof(Math::Vector3), sizeof(Math::Vector3), 1);
			fc->Read(&specular, sizeof(Math::Vector3), sizeof(Math::Vector3), 1);
			diffuses.push_back(diffuse);	ambients.push_back(ambient);	speculars.push_back(specular);
		}
		//メッシュ数分の配列
		std::vector<int> clusterCounts(meshCount);
		std::vector<int> indexCounts(meshCount);
		//メッシュ数->頂点数->影響ボーン数
		std::vector<std::vector<std::vector<BoneInfo>>> boneInfos(meshCount);
		//メッシュ数->クラスター数
		std::vector<std::vector<DirectX::XMFLOAT4X4>> initPoseMatrices(meshCount);
		//メッシュ数->頂点数
		std::vector<std::vector<int>> boneImpactSize(meshCount);

		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			fc->Read(&clusterCounts[meshIndex], sizeof(int), sizeof(int), 1);
			//一応
			fc->Read(&indexCounts[meshIndex], sizeof(int), sizeof(int), 1);
			boneInfos[meshIndex].resize(indexCounts[meshIndex]);
			boneImpactSize[meshIndex].resize(indexCounts[meshIndex]);
			for (int vertexIndex = 0; vertexIndex < indexCounts[meshIndex]; vertexIndex++) {
				int renderTransform = -1;
				fc->Read(&renderTransform, sizeof(int), sizeof(int), 1);
				boneImpactSize[meshIndex][vertexIndex] = renderTransform;
				boneInfos[meshIndex][vertexIndex].resize(renderTransform);
				fc->Read(boneInfos[meshIndex][vertexIndex].data(), sizeof(BoneInfo)*renderTransform, sizeof(BoneInfo), renderTransform);
			}
			initPoseMatrices[meshIndex].resize(clusterCounts[meshIndex]);
			fc->Read(initPoseMatrices[meshIndex].data(), sizeof(DirectX::XMFLOAT4X4)*clusterCounts[meshIndex], sizeof(DirectX::XMFLOAT4X4), clusterCounts[meshIndex]);
		}
		fc->Close();
		Model::Desc desc = {};
		desc.vertex_count = vertexCount;
		desc.vertex = vertices.data();
		desc.delimiter_count = delimiters.size();
		desc.delimiters = delimiters.data();
		desc.vertex_material_name = verticesMaterialNames.data();
		desc.material_count = materialCount;
		desc.names = materialNames.data();
		desc.texture_names = textureNames.data();
		desc.diffuses = diffuses.data();
		desc.ambients = ambients.data();
		desc.speculars = speculars.data();
		//メッシュ数分の配列
		desc.clusterCounts = &clusterCounts;
		desc.indexCounts = &indexCounts;
		//メッシュ数->頂点数->影響ボーン数
		desc.boneInfos = &boneInfos;
		//メッシュ数->クラスター数
		desc.initPoseMatrices = &initPoseMatrices;
		desc.boneImpactSize = &boneImpactSize;
		Construction(desc);
	}
	Model::~Model() = default;
	void Model::Construction(const Desc& desc) {
		try {
			//メッシュ数分の配列
			std::vector<int> clusterCounts;
			std::vector<int> indexCounts;
			//メッシュ数->頂点数->影響ボーン数
			std::vector<std::vector<std::vector<BoneInfo>>> boneInfos;
			//メッシュ数->クラスター数
			std::vector<std::vector<DirectX::XMFLOAT4X4>> initPoseMatrices;
			//メッシュ数->頂点数
			std::vector<std::vector<int>> boneImpactSize;

			std::vector<std::pair<std::string, int>> localDelimiters;
			mesh = std::make_unique<Mesh<Vertex>>(static_cast<int>(desc.vertex_count));
			for (int i = 0; i < desc.vertex_count; i++) {
				mesh->GetBuffer()[i].pos = desc.vertex[i].pos;
				mesh->GetBuffer()[i].pos.w = 1.0f;
				mesh->GetBuffer()[i].normal = desc.vertex[i].normal;
				mesh->GetBuffer()[i].tex = desc.vertex[i].tex;
			}
			for (int i = 0; i < desc.material_count; i++) {
				materials[desc.names[i]] = std::make_shared<Material>(desc.names[i].c_str(), desc.texture_names[i].c_str());
				Math::Vector4 diffuse = Math::Vector4(desc.diffuses[i].x, desc.diffuses[i].y, desc.diffuses[i].z, 1.0f);
				materials[desc.names[i]]->SetDiffuseColor(diffuse);
				Math::Vector4 ambient = Math::Vector4(desc.ambients[i].x, desc.ambients[i].y, desc.ambients[i].z, 1.0f);
				materials[desc.names[i]]->SetAmbientColor(ambient);
				Math::Vector4 specular = Math::Vector4(desc.speculars[i].x, desc.speculars[i].y, desc.speculars[i].z, 1.0f);
				materials[desc.names[i]]->SetSpecularColor(specular);
				materials[desc.names[i]]->SetTexColor(0xFFFFFFFF);
			}
			localDelimiters.resize(desc.delimiter_count);
			renderIndexMaterial.resize(desc.delimiter_count);
			for (int i = 0; i < desc.delimiter_count; i++) {
				renderIndexMaterial[i] = (materials[desc.vertex_material_name[i]].get());
				localDelimiters[i].first = desc.vertex_material_name[i];
				localDelimiters[i].second = desc.delimiters[i];
			}
			mesh->SetDelimiters(localDelimiters);
			clusterCounts = *desc.clusterCounts;
			indexCounts = *desc.indexCounts;
			//メッシュ数->頂点数->影響ボーン数
			boneInfos = *desc.boneInfos;
			//メッシュ数->クラスター数
			initPoseMatrices = *desc.initPoseMatrices;
			boneImpactSize = *desc.boneImpactSize;
			int currentPos = 0;
			for (int i = 0; i < desc.delimiter_count; i++) {
				for (int j = 0; j < desc.delimiters[i]; j++) {
					for (int k = 0; k < 4; k++) {
						if (k >= boneImpactSize[i][j]) {
							mesh->GetBuffer()[currentPos].clusteIndex[k] = 0;
							mesh->GetBuffer()[currentPos].weights.v[k] = 0;
						}
						else {
							mesh->GetBuffer()[currentPos].clusteIndex[k] = boneInfos[i][j][k].clusterIndex;
							mesh->GetBuffer()[currentPos].weights.v[k] = boneInfos[i][j][k].weight;
						}
					}
					currentPos++;
				}
			}
		}
		catch (...) {
			throw;
		}
	}
	void Model::LoadAnimation(const char* animation_path) { animations.push_back(std::make_unique<Animation>(animation_path)); }
	//名前でもアクセスできるようにすること
	void Model::AnimationUpdate(float time) { if (animationIndex > -1)animations[animationIndex]->AddElapsedTime(time); }

	std::map<std::string, std::shared_ptr<Material>>& Model::GetMaterials() { return materials; }
	Material* Model::GetMaterial(const std::string& material_name) { return materials[material_name].get(); }
	std::string Model::GetMaterialName(int polygon_index) { return nullptr; }
	void Model::ActivateAnimation(int index) { animationIndex = index; }
	void Model::SetPos(const Math::Vector3& pos) { this->pos = pos; }
	void Model::SetScale(const Math::Vector3& scale) { this->scale = scale; }
	void Model::SetEuler(const Math::Vector3& euler) { this->euler = euler; }
	void Model::Update() {
		world = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
		world *= DirectX::XMMatrixRotationRollPitchYaw(euler.x, euler.y, euler.z);
		world *= DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
		world = DirectX::XMMatrixTranspose(world);
	}
	//TODO : 後は読み込みの段階でマテリアルが共通のものを1つにまとめるようにしたほうが良い
	//その際スキンメッシュに気を付けること、あくまでも最初にスキンメッシュをしてから取り掛かる
	//TODO : ワールド変換行列を取得できるようにする。
	void Model::Render(bool set_default_pipeline) {
		mesh->Set(); inputLayout->Set(); constantBuffer->Activate(world);
		Device::GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		int delimiterSize = static_cast<int>(mesh->GetDelimiterCount());
		if (set_default_pipeline) {
			if (animationIndex > -1)Graphics::PipelineManager::PipelineGet("Default3DSkin")->Activate(true);
			else Graphics::PipelineManager::PipelineGet("Default3D")->Activate(true);
		}
		int log = 0;
		//このあたり考えて
		for (int i = 0; i < delimiterSize; i++) {
			int vertexCount = mesh->GetDelimiter(i);
			if (animationIndex > -1)	animations[animationIndex]->Activate(i);
			if (!renderIndexMaterial[i]->IsVisible()) {
				log += vertexCount;
				continue;
			}
			renderIndexMaterial[i]->Set(true, true);
			Device::GetContext()->Draw(vertexCount, log);
			log += vertexCount;
		}
	}


}

