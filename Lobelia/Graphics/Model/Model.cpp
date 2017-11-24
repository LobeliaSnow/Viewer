#include "Common/Common.hpp"
#include "Graphics/Transform/Transform.hpp"
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

namespace Lobelia::Graphics {
#define EXCEPTION_FC(fc)		if (!fc)STRICT_THROW("ファイル操作に失敗しました");
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Dxd
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	DxdImporter::DxdImporter(const char* file_path) {
		std::shared_ptr<Utility::FileController> fc = std::make_unique<Utility::FileController>();
		//ファイル開く
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		//開かれているかどうか？
		if (!fc->IsOpen())STRICT_THROW("ファイルが開けませんでした");
		//メッシュ読み込み
		MeshLoad(fc);
		//スキン読み込み
		SkinLoad(fc);
		//終了
		fc->Close();
	}
	DxdImporter::~DxdImporter() = default;
	void DxdImporter::MeshLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//メッシュ数取得
		fc->Read(&meshCount, sizeof(int), sizeof(int), 1);
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			VertexLoad(file);
		}
	}
	void DxdImporter::VertexLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		Mesh mesh = {};
		//インデックス数取得
		fc->Read(&mesh.indexCount, sizeof(int), sizeof(int), 1);
		//uv数取得
		fc->Read(&mesh.uvCount, sizeof(int), sizeof(int), 1);
		mesh.vertices.resize(mesh.indexCount);
		//Vertex取得
		fc->Read(mesh.vertices.data(), sizeof(Vertex)*mesh.indexCount, sizeof(Vertex), mesh.indexCount);
		//マテリアル名取得
		fc->Read(&mesh.materialNameLength, sizeof(int), sizeof(int), 1);
		//マテリアル名
		char* temp = new char[mesh.materialNameLength];
		fc->Read(temp, sizeof(char)*mesh.materialNameLength, sizeof(char), mesh.materialNameLength);
		mesh.materialName = temp;
		delete[] temp;
		//メッシュ追加
		meshes.push_back(mesh);
	}
	void DxdImporter::SkinLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		clusterCount.resize(meshCount);
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			bool isEntity = false;
			//ボーンを持っているか否か
			fc->Read(&isEntity, sizeof(bool), sizeof(bool), 1);
			//ボーン持っていなければ終了
			if (!isEntity) {
				meshsBoneInfo.push_back({});
				continue;
			}
			ClusterLoad(file, meshIndex);
		}
	}
	void DxdImporter::ClusterLoad(std::weak_ptr<Utility::FileController> file, int mesh_index) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		Bone bone = {};
		bone.isEntity = true;
		int indexCount = 0;
		//頂点数取得
		fc->Read(&indexCount, sizeof(int), sizeof(int), 1);
		//頂点数分だけバッファ確保
		bone.infos.resize(indexCount);
		for (int i = 0; i < indexCount; i++) {
			int impactSize = 0;
			//影響数保存
			fc->Read(&impactSize, sizeof(int), sizeof(int), 1);
			bone.infos[i].resize(impactSize);
			//影響度保存
			fc->Read(bone.infos[i].data(), sizeof(Bone::Info)*impactSize, sizeof(Bone::Info), impactSize);
		}
		//クラスター数取得
		fc->Read(&clusterCount[mesh_index], sizeof(int), sizeof(int), 1);
		//クラスター数分だけバッファ確保
		bone.initPoseMatrices.resize(clusterCount[mesh_index]);
		std::vector<DirectX::XMFLOAT4X4> matrices(clusterCount[mesh_index]);
		//初期姿勢行列取得
		fc->Read(matrices.data(), sizeof(DirectX::XMFLOAT4X4)*clusterCount[mesh_index], sizeof(DirectX::XMFLOAT4X4), clusterCount[mesh_index]);
		for (int i = 0; i < clusterCount[mesh_index]; i++) {
			bone.initPoseMatrices[i] = DirectX::XMLoadFloat4x4(&matrices[i]);
		}
		//ボーン追加
		meshsBoneInfo.push_back(bone);
	}
	int DxdImporter::GetMeshCount() { return meshCount; }
	const std::vector<DxdImporter::Mesh>& DxdImporter::GetMeshes() { return meshes; }
	DxdImporter::Mesh& DxdImporter::GetMesh(int index) { return meshes[index]; }
	int DxdImporter::GetBoneCount(int mesh_index) { return clusterCount[mesh_index]; }
	const std::vector<DxdImporter::Bone>& DxdImporter::GetMeshsBoneInfos() { return meshsBoneInfo; }
	const DxdImporter::Bone& DxdImporter::GetMeshBoneInfo(int index) { return meshsBoneInfo[index]; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// material
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	MaterialImporter::MaterialImporter(const char* file_path) {
		std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		if (!fc->IsOpen())STRICT_THROW("ファイルを開けませんでした");
		Load(fc);
		fc->Close();
	}
	MaterialImporter::~MaterialImporter() = default;
	void MaterialImporter::Load(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//マテリアル数取得
		fc->Read(&materialCount, sizeof(int), sizeof(int), 1);
		materials.resize(materialCount);
		auto StringLoad = [&](int* count, std::string* str) {
			fc->Read(count, sizeof(int), sizeof(int), 1);
			//バッファ確保
			char* temp = new char[*count];
			//マテリアル名取得
			fc->Read(temp, sizeof(char)*(*count), sizeof(char), *count);
			*str = temp;
			//バッファ解放
			delete[] temp;
		};
		for (int i = 0; i < materialCount; i++) {
			Material material = {};
			//マテリアル名取得
			StringLoad(&material.nameLength, &material.name);
			//テクスチャ名取得
			StringLoad(&material.textureNameLength, &material.textureName);
			//マテリアル追加
			materials[i] = material;
		}
	}
	int MaterialImporter::GetMaterialCount() { return materialCount; }
	const std::vector<MaterialImporter::Material>&  MaterialImporter::GetMaterials() { return materials; }
	const MaterialImporter::Material& MaterialImporter::GetMaterial(int index) { return materials[index]; }
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// anm
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AnimationImporter::AnimationImporter(const char* file_path) {
		std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		if (!fc->IsOpen())STRICT_THROW("アニメーションファイルが開けませんでした");
		//名前取得
		LoadName(fc);
		//基本情報取得
		SettingLoad(fc);
		//キーフレーム取得
		KeyFramesLoad(fc);
		fc->Close();
	}
	AnimationImporter::~AnimationImporter() = default;
	void AnimationImporter::LoadName(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//マテリアル名取得
		fc->Read(&nameLength, sizeof(int), sizeof(int), 1);
		//バッファ確保
		char* temp = new char[nameLength];
		//マテリアル名
		fc->Read(temp, sizeof(char)*nameLength, sizeof(char), nameLength);
		name = temp;
		delete[] temp;
	}
	void AnimationImporter::SettingLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//1秒あたりのフレーム数取得
		fc->Read(&framePerSecond, sizeof(int), sizeof(int), 1);
		//総フレーム数取得
		fc->Read(&keyFrameCount, sizeof(int), sizeof(int), 1);
		//メッシュ数取得
		fc->Read(&meshCount, sizeof(int), sizeof(int), 1);
	}
	void AnimationImporter::KeyFramesLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		infos.resize(meshCount);
		for (int i = 0; i < meshCount; i++) {
			Info info = {};
			//クラスター数取得
			fc->Read(&info.clusetCount, sizeof(int), sizeof(int), 1);
			//バッファ確保
			info.clusterFrames.resize(info.clusetCount);
			for (int clusterIndex = 0; clusterIndex < info.clusetCount; clusterIndex++) {
				info.clusterFrames[clusterIndex].keyFrames.resize(keyFrameCount);
				//キーフレーム取得
				fc->Read(info.clusterFrames[clusterIndex].keyFrames.data(), sizeof(DirectX::XMFLOAT4X4)*keyFrameCount, sizeof(DirectX::XMFLOAT4X4), keyFrameCount);
			}
			//キーフレーム情報取得
			infos[i] = info;
		}
	}
	const std::string& AnimationImporter::GetName() { return name; }
	int AnimationImporter::GetSampleFramePerSecond() { return framePerSecond; }
	int AnimationImporter::GetKeyFrameCount() { return keyFrameCount; }
	int AnimationImporter::GetMeshCount() { return meshCount; }
	const std::vector<AnimationImporter::Info>& AnimationImporter::GetInfos() { return infos; }
	const AnimationImporter::Info& AnimationImporter::GetInfo(int index) { return infos[index]; }
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Animation
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Animation::Animation(const char* file_path) :constantBuffer(std::make_unique<ConstantBuffer<Constant>>(3, ShaderStageList::VS)), time(0.0f) {
		std::shared_ptr<Lobelia::Graphics::AnimationImporter> importer = std::make_unique<Lobelia::Graphics::AnimationImporter>(file_path);
		//アニメーション名取得
		name = importer->GetName();
		//メッシュ数取得
		meshCount = importer->GetMeshCount();
		//1秒あたりのサンプルフレーム数取得
		framePerCount = importer->GetSampleFramePerSecond();
		//フレーム数取得
		frameCount = importer->GetKeyFrameCount();
		//バッファ確保
		clusterCount.resize(meshCount);
		keyFrames.resize(meshCount);
		//キーフレーム取得開始
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			//メッシュごとのクラスター数取得
			clusterCount[meshIndex] = importer->GetInfo(meshIndex).clusetCount;
			//バッファ確保
			keyFrames[meshIndex].resize(clusterCount[meshIndex]);
			for (int clusterIndex = 0; clusterIndex < clusterCount[meshIndex]; clusterIndex++) {
				//バッファ確保
				keyFrames[meshIndex][clusterIndex].resize(frameCount);
				for (int frameIndex = 0; frameIndex < frameCount; frameIndex++) {
					//各メッシュの各クラスターにあるキーフレームを取得
					keyFrames[meshIndex][clusterIndex][frameIndex] = importer->GetInfo(meshIndex).clusterFrames[clusterIndex].keyFrames[frameIndex];
				}
			}
		}
	}
	Animation::~Animation() = default;
	void Animation::AddElapsedTime(float time) {
		this->time += time;
		//アニメーションの最大値を取得
		int animationMax = (frameCount - 1)*(1000 / framePerCount);
		if (this->time >= animationMax)this->time -= animationMax;
	}
	void Animation::Update(int meshIndex) {
		//補間等ごちゃごちゃしないといけない
		for (int i = 0; i < clusterCount[meshIndex]; i++) {
			DirectX::XMMATRIX renderTransform = DirectX::XMLoadFloat4x4(&keyFrames[meshIndex][i][static_cast<int>(time / (1000 / framePerCount))]);
			renderTransform = DirectX::XMMatrixTranspose(renderTransform);
			//本当はここで補間
			DirectX::XMStoreFloat4x4(&buffer.keyFrame[i], renderTransform);
		}
		constantBuffer->Activate(buffer);
	}
	const std::string& Animation::GetName() { return name; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Model
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Model::Model(const char* dxd_path, const char* mt_path) :allMeshVertexCountSum(0), world() {
		StateInitialize();
		std::shared_ptr<DxdImporter> dxd = std::make_unique<DxdImporter>(dxd_path);
		std::shared_ptr<MaterialImporter> mt = std::make_unique<MaterialImporter>(mt_path);
		//総点数算出
		for (int i = 0; i < dxd->GetMeshCount(); i++) {
			//各メッシュの頂点数を足し合わせる
			allMeshVertexCountSum += dxd->GetMesh(i).indexCount;
		}
		//メッシュバッファ確保
		mesh = std::make_unique<Mesh<Vertex>>(allMeshVertexCountSum);
		//頂点構成
		ConfigureVertex(dxd);
		//マテリアル構成
		std::string directory = Utility::FilePathControl::GetParentDirectory(mt_path);
		if (!directory.empty())directory += "/";
		ConfigureMaterial(mt, directory);
		//バッファ確保
		renderIndexMaterial.resize(dxd->GetMeshCount());
		//描画順にマテリアルへのポインタ整列
		for (int i = 0; i < dxd->GetMeshCount(); i++) {
			renderIndexMaterial[i] = materials[dxd->GetMesh(i).materialName].get();
		}
		SetTransformAndCalcMatrix(transform);
		CalculationWorldMatrix();
	}
	Model::~Model() = default;
	void Model::StateInitialize() {
		//アクティブなアニメーションは無し(-1)
		activeAnimation = -1;
		//デフォルトの頂点シェーダー取得
		VertexShader* vs = ShaderBank::Get<VertexShader>(DEFAULT_VERTEX_SHADER_STATIC_MODEL);
		//リフレクション開始
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs);
		//入力レイアウト作成
		inputLayout = std::make_unique<InputLayout>(vs, reflector.get());
		//コンスタントバッファ作成
		constantBuffer = std::make_unique<ConstantBuffer<DirectX::XMMATRIX>>(1, Graphics::ShaderStageList::VS);
		//親は設定されていない
		parent = nullptr;
		transform = {};
		transform.scale = Math::Vector3(1.0f, 1.0f, 1.0f);
		animationCount = 0;
	}
	void Model::ConfigureVertex(std::weak_ptr<DxdImporter> dxd) {
		std::shared_ptr<DxdImporter> importer = dxd.lock();
		if (!importer)STRICT_THROW("インポーターが取得できませんでした");
		int log = 0;
		for (int meshIndex = 0, index = 0, boneIndex = 0; meshIndex < importer->GetMeshCount(); meshIndex++) {
			auto& dxdMesh = importer->GetMesh(meshIndex);
			//頂点数取得
			int vertexCount = dxdMesh.indexCount;
			//頂点情報(一部)取得
			for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++, index++) {
				mesh->GetBuffer()[index].pos = dxdMesh.vertices[vertexIndex].pos;
				mesh->GetBuffer()[index].normal = dxdMesh.vertices[vertexIndex].normal;
				mesh->GetBuffer()[index].tex = dxdMesh.vertices[vertexIndex].tex;
			}
			//ボーン情報構成
			ConfigureBones(dxd, meshIndex, &boneIndex);
			//サブセット構築
			Subset subset = { meshIndex, log, importer->GetMesh(meshIndex).indexCount };
			//メッシュ開始地点を次へずらす
			log += importer->GetMesh(meshIndex).indexCount;
			//サブセット追加
			subsets.push_back(subset);
		}
	}
	void Model::ConfigureBones(std::weak_ptr<DxdImporter> dxd, int mesh_index, int* vertex_index) {
		std::shared_ptr<DxdImporter> importer = dxd.lock();
		if (!importer)STRICT_THROW("インポーターが取得できませんでした");
		//メッシュごとのボーン群取得
		auto& meshBones = importer->GetMeshBoneInfo(mesh_index);
		int indexCount = importer->GetMesh(mesh_index).indexCount;
		//この時ボーンは生成されないので、少し注意。何か思わぬ誤作動があるかも？
		if (!meshBones.isEntity) {
			(*vertex_index) += indexCount;
			return;
		}
		//頂点構成(残り)
		for (int i = 0; i < indexCount; i++) {
			int impactCount = i_cast(meshBones.infos[i].size());
			for (int j = 0; j < 4; j++) {
				if (j < impactCount) {
					mesh->GetBuffer()[*vertex_index].clusteIndex[j] = meshBones.infos[i][j].clusterIndex;
					mesh->GetBuffer()[*vertex_index].weights.v[j] = meshBones.infos[i][j].weight;
				}
				else {
					mesh->GetBuffer()[*vertex_index].clusteIndex[j] = 0UL;
					mesh->GetBuffer()[*vertex_index].weights.v[j] = 0.0f;
				}
			}
			(*vertex_index)++;
		}
		Bone bone = {};
		//クラスター数取得
		bone.clusterCount = importer->GetBoneCount(mesh_index);
		//初期姿勢行列取得
		for (int i = 0; i < bone.clusterCount; i++) {
			bone.initPoseMatrices.push_back(meshBones.initPoseMatrices[i]);
		}
		bones.push_back(bone);
	}
	void Model::ConfigureMaterial(std::weak_ptr<MaterialImporter> mt, const std::string& directory) {
		std::shared_ptr<MaterialImporter> importer = mt.lock();
		int materialCount = importer->GetMaterialCount();
		for (int i = 0; i < materialCount; i++) {
			auto& material = importer->GetMaterial(i);
			materials[material.name] = std::make_shared<Material>(material.name.c_str(), (directory + material.textureName).c_str());
		}
	}
	void Model::CalcTranslateMatrix() { translate = DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z); }
	void Model::CalcScallingMatrix() { scalling = DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z); }
	void Model::SetTransformAndCalcMatrix(const Transform3D& transform) {
		this->transform = transform;
		Translation(transform.position);
		RotationRollPitchYow(transform.rotation);
		Scalling(transform.scale);
	}
	const Transform3D& Model::GetTransform() { return transform; }
	void Model::LinkParent(Model* model) { parent = model; }
	void Model::UnLinkParent() { parent = nullptr; }
	//移動
	void Model::Translation(const Math::Vector3& pos) {
		transform.position = pos;
		CalcTranslateMatrix();
	}
	void Model::Translation(float x, float y, float z) {
		transform.position.x = x; transform.position.y = y; transform.position.z = z;
		CalcTranslateMatrix();
	}
	void Model::TranslationMove(const Math::Vector3& move) {
		transform.position += move;
		CalcTranslateMatrix();
	}
	void Model::TranslationMove(float x, float y, float z) {
		transform.position.x += x; transform.position.y += y; transform.position.z += z;
		CalcTranslateMatrix();
	}
	//回転
	void Model::RotationQuaternion(const DirectX::XMVECTOR& quaternion) {
		rotation = DirectX::XMMatrixRotationQuaternion(quaternion);
		//ここでトランスフォームの回転にRPYの回転量算出
	}
	void Model::RotationAxis(const Math::Vector3& axis, float rad) {
		rotation = DirectX::XMMatrixRotationAxis(DirectX::XMVECTOR{ axis.x,axis.y,axis.z,1.0f }, rad);
		//ここでトランスフォームの回転にRPYの回転量算出
	}
	void Model::RotationRollPitchYow(const Math::Vector3& rpy) {
		transform.rotation = rpy;
		rotation = DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z);
	}
	void Model::RotationYAxis(float rad) {
		transform.rotation.x = 0.0f;	transform.rotation.y = rad; transform.rotation.z = 0.0f;
		rotation = DirectX::XMMatrixRotationY(transform.rotation.y);
	}
	//拡縮
	void Model::Scalling(const Math::Vector3& scale) {
		transform.scale = scale;
		CalcScallingMatrix();
	}
	void Model::Scalling(float x, float y, float z) {
		transform.scale.x = x; transform.scale.y = y; transform.scale.z = z;
		CalcScallingMatrix();
	}
	void Model::Scalling(float scale) {
		transform.scale.x = scale; transform.scale.y = scale; transform.scale.z = scale;
		CalcScallingMatrix();
	}
	//更新処理
	void Model::CalculationWorldMatrix() {
		//親子関係あるさいは自分のtransformは親から見たものになるが、ワールドの状態でも欲しいかな？
		world = scalling;
		world *= rotation;
		//ここは少し審議が必要
		world *= translate;
		//親子関係周り
		if (parent)world *= parent->world;
		else world.m[0][0] *= -1;
		//↑位置xを反転させる(FBX問題解消)
		//親がいるとその行列ですでに-1掛けされているのでここでは必要がない(?)
	}
	void Model::GetTranslateMatrix(DirectX::XMMATRIX* translate) {
		if (!translate)STRICT_THROW("translateがnullptrです");
		*translate = this->translate;
	}
	void Model::CalcInverseTranslateMatrix(DirectX::XMMATRIX* inv_translate) {
		if (!inv_translate)STRICT_THROW("inv_worldがnullptrです");
		DirectX::XMVECTOR arg = {};
		*inv_translate = DirectX::XMMatrixInverse(&arg, translate);
	}
	void Model::GetScallingMatrix(DirectX::XMMATRIX* scalling) {
		if (!scalling)STRICT_THROW("scallingがnullptrです");
		*scalling = this->scalling;
	}
	void Model::CalcInverseScallingMatrix(DirectX::XMMATRIX* inv_scalling) {
		if (!inv_scalling)STRICT_THROW("inv_worldがnullptrです");
		DirectX::XMVECTOR arg = {};
		*inv_scalling = DirectX::XMMatrixInverse(&arg, scalling);

	}
	void Model::GetRotationMatrix(DirectX::XMMATRIX* rotation) {
		if (!rotation)STRICT_THROW("rotationがnullptrです");
		*rotation = this->rotation;
	}
	void Model::CalcInverseRotationMatrix(DirectX::XMMATRIX* inv_rotation) {
		if (!inv_rotation)STRICT_THROW("inv_rotationがnullptrです");
		DirectX::XMVECTOR arg = {};
		*inv_rotation = DirectX::XMMatrixInverse(&arg, rotation);
	}
	void Model::GetWorldMatrix(DirectX::XMMATRIX* world) {
		if (!world)STRICT_THROW("worldがnullptrです");
		*world = this->world;
	}
	void Model::CalcInverseWorldMatrix(DirectX::XMMATRIX* inv_world) {
		if (!inv_world)STRICT_THROW("inv_worldがnullptrです");
		DirectX::XMVECTOR arg = {};
		*inv_world = DirectX::XMMatrixInverse(&arg, world);
	}
	AnimationNo Model::AnimationLoad(const char* file_path) {
		animations.push_back(std::make_unique<Animation>(file_path));
		return animationCount++;
	}
	void Model::AnimationActivate(AnimationNo index) {
		if (index >= animationCount)STRICT_THROW("存在しないアニメーションです");
		activeAnimation = index;
	}
	void Model::AnimationInActive() { activeAnimation = -1; }
	const std::string& Model::GetAnimationName(AnimationNo index) { return animations[index]->GetName(); }
	void Model::AnimationUpdate(float elapsed_time) { animations[activeAnimation]->AddElapsedTime(elapsed_time); }
	void Model::Render(bool no_set) {
		mesh->Set(); inputLayout->Set(); constantBuffer->Activate(DirectX::XMMatrixTranspose(world));
		Device::GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		if (!no_set) {
			//スキニングするか否か
			if (activeAnimation > -1) Graphics::PipelineManager::PipelineGet(DEFAULT_PIPELINE_DYNAMIC_MODEL)->Activate(true);
			else Graphics::PipelineManager::PipelineGet(DEFAULT_PIPELINE_STATIC_MODEL)->Activate(true);
		}
		int meshIndex = 0;
		for each(auto subset in subsets) {
			if (activeAnimation > -1)animations[activeAnimation]->Update(meshIndex);
			subset.Render(this);
			meshIndex++;
		}
	}
}
