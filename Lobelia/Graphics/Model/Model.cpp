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
#define EXCEPTION_FC(fc)		if (!fc)STRICT_THROW("�t�@�C������Ɏ��s���܂���");
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Dxd
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	DxdImporter::DxdImporter(const char* file_path) {
		std::shared_ptr<Utility::FileController> fc = std::make_unique<Utility::FileController>();
		//�t�@�C���J��
		fc->Open(file_path, Utility::FileController::OpenMode::ReadBinary);
		//�J����Ă��邩�ǂ����H
		if (!fc->IsOpen())STRICT_THROW("�t�@�C�����J���܂���ł���");
		//���b�V���ǂݍ���
		MeshLoad(fc);
		//�X�L���ǂݍ���
		SkinLoad(fc);
		//�I��
		fc->Close();
	}
	DxdImporter::~DxdImporter() = default;
	void DxdImporter::MeshLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//���b�V�����擾
		fc->Read(&meshCount, sizeof(int), sizeof(int), 1);
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			VertexLoad(file);
		}
	}
	void DxdImporter::VertexLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		Mesh mesh = {};
		//�C���f�b�N�X���擾
		fc->Read(&mesh.indexCount, sizeof(int), sizeof(int), 1);
		//uv���擾
		fc->Read(&mesh.uvCount, sizeof(int), sizeof(int), 1);
		mesh.vertices.resize(mesh.indexCount);
		//Vertex�擾
		fc->Read(mesh.vertices.data(), sizeof(Vertex)*mesh.indexCount, sizeof(Vertex), mesh.indexCount);
		//�}�e���A�����擾
		fc->Read(&mesh.materialNameLength, sizeof(int), sizeof(int), 1);
		//�}�e���A����
		char* temp = new char[mesh.materialNameLength];
		fc->Read(temp, sizeof(char)*mesh.materialNameLength, sizeof(char), mesh.materialNameLength);
		mesh.materialName = temp;
		delete[] temp;
		//���b�V���ǉ�
		meshes.push_back(mesh);
	}
	void DxdImporter::SkinLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		clusterCount.resize(meshCount);
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			bool isEntity = false;
			//�{�[���������Ă��邩�ۂ�
			fc->Read(&isEntity, sizeof(bool), sizeof(bool), 1);
			//�{�[�������Ă��Ȃ���ΏI��
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
		//���_���擾
		fc->Read(&indexCount, sizeof(int), sizeof(int), 1);
		//���_���������o�b�t�@�m��
		bone.infos.resize(indexCount);
		for (int i = 0; i < indexCount; i++) {
			int impactSize = 0;
			//�e�����ۑ�
			fc->Read(&impactSize, sizeof(int), sizeof(int), 1);
			bone.infos[i].resize(impactSize);
			//�e���x�ۑ�
			fc->Read(bone.infos[i].data(), sizeof(Bone::Info)*impactSize, sizeof(Bone::Info), impactSize);
		}
		//�N���X�^�[���擾
		fc->Read(&clusterCount[mesh_index], sizeof(int), sizeof(int), 1);
		//�N���X�^�[���������o�b�t�@�m��
		bone.initPoseMatrices.resize(clusterCount[mesh_index]);
		std::vector<DirectX::XMFLOAT4X4> matrices(clusterCount[mesh_index]);
		//�����p���s��擾
		fc->Read(matrices.data(), sizeof(DirectX::XMFLOAT4X4)*clusterCount[mesh_index], sizeof(DirectX::XMFLOAT4X4), clusterCount[mesh_index]);
		for (int i = 0; i < clusterCount[mesh_index]; i++) {
			bone.initPoseMatrices[i] = DirectX::XMLoadFloat4x4(&matrices[i]);
		}
		//�{�[���ǉ�
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
		if (!fc->IsOpen())STRICT_THROW("�t�@�C�����J���܂���ł���");
		Load(fc);
		fc->Close();
	}
	MaterialImporter::~MaterialImporter() = default;
	void MaterialImporter::Load(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//�}�e���A�����擾
		fc->Read(&materialCount, sizeof(int), sizeof(int), 1);
		materials.resize(materialCount);
		auto StringLoad = [&](int* count, std::string* str) {
			fc->Read(count, sizeof(int), sizeof(int), 1);
			//�o�b�t�@�m��
			char* temp = new char[*count];
			//�}�e���A�����擾
			fc->Read(temp, sizeof(char)*(*count), sizeof(char), *count);
			*str = temp;
			//�o�b�t�@���
			delete[] temp;
		};
		for (int i = 0; i < materialCount; i++) {
			Material material = {};
			//�}�e���A�����擾
			StringLoad(&material.nameLength, &material.name);
			//�e�N�X�`�����擾
			StringLoad(&material.textureNameLength, &material.textureName);
			//�}�e���A���ǉ�
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
		if (!fc->IsOpen())STRICT_THROW("�A�j���[�V�����t�@�C�����J���܂���ł���");
		//���O�擾
		LoadName(fc);
		//��{���擾
		SettingLoad(fc);
		//�L�[�t���[���擾
		KeyFramesLoad(fc);
		fc->Close();
	}
	AnimationImporter::~AnimationImporter() = default;
	void AnimationImporter::LoadName(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//�}�e���A�����擾
		fc->Read(&nameLength, sizeof(int), sizeof(int), 1);
		//�o�b�t�@�m��
		char* temp = new char[nameLength];
		//�}�e���A����
		fc->Read(temp, sizeof(char)*nameLength, sizeof(char), nameLength);
		name = temp;
		delete[] temp;
	}
	void AnimationImporter::SettingLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		//1�b������̃t���[�����擾
		fc->Read(&framePerSecond, sizeof(int), sizeof(int), 1);
		//���t���[�����擾
		fc->Read(&keyFrameCount, sizeof(int), sizeof(int), 1);
		//���b�V�����擾
		fc->Read(&meshCount, sizeof(int), sizeof(int), 1);
	}
	void AnimationImporter::KeyFramesLoad(std::weak_ptr<Utility::FileController> file) {
		std::shared_ptr<Utility::FileController> fc = file.lock();
		EXCEPTION_FC(fc);
		infos.resize(meshCount);
		for (int i = 0; i < meshCount; i++) {
			Info info = {};
			//�N���X�^�[���擾
			fc->Read(&info.clusetCount, sizeof(int), sizeof(int), 1);
			//�o�b�t�@�m��
			info.clusterFrames.resize(info.clusetCount);
			for (int clusterIndex = 0; clusterIndex < info.clusetCount; clusterIndex++) {
				info.clusterFrames[clusterIndex].keyFrames.resize(keyFrameCount);
				//�L�[�t���[���擾
				fc->Read(info.clusterFrames[clusterIndex].keyFrames.data(), sizeof(DirectX::XMFLOAT4X4)*keyFrameCount, sizeof(DirectX::XMFLOAT4X4), keyFrameCount);
			}
			//�L�[�t���[�����擾
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
		//�A�j���[�V�������擾
		name = importer->GetName();
		//���b�V�����擾
		meshCount = importer->GetMeshCount();
		//1�b������̃T���v���t���[�����擾
		framePerCount = importer->GetSampleFramePerSecond();
		//�t���[�����擾
		frameCount = importer->GetKeyFrameCount();
		//�o�b�t�@�m��
		clusterCount.resize(meshCount);
		keyFrames.resize(meshCount);
		//�L�[�t���[���擾�J�n
		for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
			//���b�V�����Ƃ̃N���X�^�[���擾
			clusterCount[meshIndex] = importer->GetInfo(meshIndex).clusetCount;
			//�o�b�t�@�m��
			keyFrames[meshIndex].resize(clusterCount[meshIndex]);
			for (int clusterIndex = 0; clusterIndex < clusterCount[meshIndex]; clusterIndex++) {
				//�o�b�t�@�m��
				keyFrames[meshIndex][clusterIndex].resize(frameCount);
				for (int frameIndex = 0; frameIndex < frameCount; frameIndex++) {
					//�e���b�V���̊e�N���X�^�[�ɂ���L�[�t���[�����擾
					keyFrames[meshIndex][clusterIndex][frameIndex] = importer->GetInfo(meshIndex).clusterFrames[clusterIndex].keyFrames[frameIndex];
				}
			}
		}
	}
	Animation::~Animation() = default;
	void Animation::AddElapsedTime(float time) {
		this->time += time;
		//�A�j���[�V�����̍ő�l���擾
		int animationMax = (frameCount - 1)*(1000 / framePerCount);
		if (this->time >= animationMax)this->time -= animationMax;
	}
	void Animation::Update(int meshIndex) {
		//��ԓ������Ⴒ���Ⴕ�Ȃ��Ƃ����Ȃ�
		for (int i = 0; i < clusterCount[meshIndex]; i++) {
			DirectX::XMMATRIX renderTransform = DirectX::XMLoadFloat4x4(&keyFrames[meshIndex][i][static_cast<int>(time / (1000 / framePerCount))]);
			renderTransform = DirectX::XMMatrixTranspose(renderTransform);
			//�{���͂����ŕ��
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
		//���_���Z�o
		for (int i = 0; i < dxd->GetMeshCount(); i++) {
			//�e���b�V���̒��_���𑫂����킹��
			allMeshVertexCountSum += dxd->GetMesh(i).indexCount;
		}
		//���b�V���o�b�t�@�m��
		mesh = std::make_unique<Mesh<Vertex>>(allMeshVertexCountSum);
		//���_�\��
		ConfigureVertex(dxd);
		//�}�e���A���\��
		std::string directory = Utility::FilePathControl::GetParentDirectory(mt_path);
		if (!directory.empty())directory += "/";
		ConfigureMaterial(mt, directory);
		//�o�b�t�@�m��
		renderIndexMaterial.resize(dxd->GetMeshCount());
		//�`�揇�Ƀ}�e���A���ւ̃|�C���^����
		for (int i = 0; i < dxd->GetMeshCount(); i++) {
			renderIndexMaterial[i] = materials[dxd->GetMesh(i).materialName].get();
		}
		SetTransformAndCalcMatrix(transform);
		CalculationWorldMatrix();
	}
	Model::~Model() = default;
	void Model::StateInitialize() {
		//�A�N�e�B�u�ȃA�j���[�V�����͖���(-1)
		activeAnimation = -1;
		//�f�t�H���g�̒��_�V�F�[�_�[�擾
		VertexShader* vs = ShaderBank::Get<VertexShader>(DEFAULT_VERTEX_SHADER_STATIC_MODEL);
		//���t���N�V�����J�n
		std::unique_ptr<Reflection> reflector = std::make_unique<Reflection>(vs);
		//���̓��C�A�E�g�쐬
		inputLayout = std::make_unique<InputLayout>(vs, reflector.get());
		//�R���X�^���g�o�b�t�@�쐬
		constantBuffer = std::make_unique<ConstantBuffer<DirectX::XMMATRIX>>(1, Graphics::ShaderStageList::VS);
		//�e�͐ݒ肳��Ă��Ȃ�
		parent = nullptr;
		transform = {};
		transform.scale = Math::Vector3(1.0f, 1.0f, 1.0f);
		animationCount = 0;
	}
	void Model::ConfigureVertex(std::weak_ptr<DxdImporter> dxd) {
		std::shared_ptr<DxdImporter> importer = dxd.lock();
		if (!importer)STRICT_THROW("�C���|�[�^�[���擾�ł��܂���ł���");
		int log = 0;
		for (int meshIndex = 0, index = 0, boneIndex = 0; meshIndex < importer->GetMeshCount(); meshIndex++) {
			auto& dxdMesh = importer->GetMesh(meshIndex);
			//���_���擾
			int vertexCount = dxdMesh.indexCount;
			//���_���(�ꕔ)�擾
			for (int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++, index++) {
				mesh->GetBuffer()[index].pos = dxdMesh.vertices[vertexIndex].pos;
				mesh->GetBuffer()[index].normal = dxdMesh.vertices[vertexIndex].normal;
				mesh->GetBuffer()[index].tex = dxdMesh.vertices[vertexIndex].tex;
			}
			//�{�[�����\��
			ConfigureBones(dxd, meshIndex, &boneIndex);
			//�T�u�Z�b�g�\�z
			Subset subset = { meshIndex, log, importer->GetMesh(meshIndex).indexCount };
			//���b�V���J�n�n�_�����ւ��炷
			log += importer->GetMesh(meshIndex).indexCount;
			//�T�u�Z�b�g�ǉ�
			subsets.push_back(subset);
		}
	}
	void Model::ConfigureBones(std::weak_ptr<DxdImporter> dxd, int mesh_index, int* vertex_index) {
		std::shared_ptr<DxdImporter> importer = dxd.lock();
		if (!importer)STRICT_THROW("�C���|�[�^�[���擾�ł��܂���ł���");
		//���b�V�����Ƃ̃{�[���Q�擾
		auto& meshBones = importer->GetMeshBoneInfo(mesh_index);
		int indexCount = importer->GetMesh(mesh_index).indexCount;
		//���̎��{�[���͐�������Ȃ��̂ŁA�������ӁB�����v��ʌ�쓮�����邩���H
		if (!meshBones.isEntity) {
			(*vertex_index) += indexCount;
			return;
		}
		//���_�\��(�c��)
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
		//�N���X�^�[���擾
		bone.clusterCount = importer->GetBoneCount(mesh_index);
		//�����p���s��擾
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
	//�ړ�
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
	//��]
	void Model::RotationQuaternion(const DirectX::XMVECTOR& quaternion) {
		rotation = DirectX::XMMatrixRotationQuaternion(quaternion);
		//�����Ńg�����X�t�H�[���̉�]��RPY�̉�]�ʎZ�o
	}
	void Model::RotationAxis(const Math::Vector3& axis, float rad) {
		rotation = DirectX::XMMatrixRotationAxis(DirectX::XMVECTOR{ axis.x,axis.y,axis.z,1.0f }, rad);
		//�����Ńg�����X�t�H�[���̉�]��RPY�̉�]�ʎZ�o
	}
	void Model::RotationRollPitchYow(const Math::Vector3& rpy) {
		transform.rotation = rpy;
		rotation = DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z);
	}
	void Model::RotationYAxis(float rad) {
		transform.rotation.x = 0.0f;	transform.rotation.y = rad; transform.rotation.z = 0.0f;
		rotation = DirectX::XMMatrixRotationY(transform.rotation.y);
	}
	//�g�k
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
	//�X�V����
	void Model::CalculationWorldMatrix() {
		//�e�q�֌W���邳���͎�����transform�͐e���猩�����̂ɂȂ邪�A���[���h�̏�Ԃł��~�������ȁH
		world = scalling;
		world *= rotation;
		//�����͏����R�c���K�v
		world *= translate;
		//�e�q�֌W����
		if (parent)world *= parent->world;
		else world.m[0][0] *= -1;
		//���ʒux�𔽓]������(FBX������)
		//�e������Ƃ��̍s��ł��ł�-1�|������Ă���̂ł����ł͕K�v���Ȃ�(?)
	}
	void Model::GetTranslateMatrix(DirectX::XMMATRIX* translate) {
		if (!translate)STRICT_THROW("translate��nullptr�ł�");
		*translate = this->translate;
	}
	void Model::CalcInverseTranslateMatrix(DirectX::XMMATRIX* inv_translate) {
		if (!inv_translate)STRICT_THROW("inv_world��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_translate = DirectX::XMMatrixInverse(&arg, translate);
	}
	void Model::GetScallingMatrix(DirectX::XMMATRIX* scalling) {
		if (!scalling)STRICT_THROW("scalling��nullptr�ł�");
		*scalling = this->scalling;
	}
	void Model::CalcInverseScallingMatrix(DirectX::XMMATRIX* inv_scalling) {
		if (!inv_scalling)STRICT_THROW("inv_world��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_scalling = DirectX::XMMatrixInverse(&arg, scalling);

	}
	void Model::GetRotationMatrix(DirectX::XMMATRIX* rotation) {
		if (!rotation)STRICT_THROW("rotation��nullptr�ł�");
		*rotation = this->rotation;
	}
	void Model::CalcInverseRotationMatrix(DirectX::XMMATRIX* inv_rotation) {
		if (!inv_rotation)STRICT_THROW("inv_rotation��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_rotation = DirectX::XMMatrixInverse(&arg, rotation);
	}
	void Model::GetWorldMatrix(DirectX::XMMATRIX* world) {
		if (!world)STRICT_THROW("world��nullptr�ł�");
		*world = this->world;
	}
	void Model::CalcInverseWorldMatrix(DirectX::XMMATRIX* inv_world) {
		if (!inv_world)STRICT_THROW("inv_world��nullptr�ł�");
		DirectX::XMVECTOR arg = {};
		*inv_world = DirectX::XMMatrixInverse(&arg, world);
	}
	AnimationNo Model::AnimationLoad(const char* file_path) {
		animations.push_back(std::make_unique<Animation>(file_path));
		return animationCount++;
	}
	void Model::AnimationActivate(AnimationNo index) {
		if (index >= animationCount)STRICT_THROW("���݂��Ȃ��A�j���[�V�����ł�");
		activeAnimation = index;
	}
	void Model::AnimationInActive() { activeAnimation = -1; }
	const std::string& Model::GetAnimationName(AnimationNo index) { return animations[index]->GetName(); }
	void Model::AnimationUpdate(float elapsed_time) { animations[activeAnimation]->AddElapsedTime(elapsed_time); }
	void Model::Render(bool no_set) {
		mesh->Set(); inputLayout->Set(); constantBuffer->Activate(DirectX::XMMatrixTranspose(world));
		Device::GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		if (!no_set) {
			//�X�L�j���O���邩�ۂ�
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
