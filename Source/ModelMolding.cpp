#include "Tool.h"
//�݌v�S�~ ��Ƀ}�e���A�����
namespace Lobelia {
	//////////////////////////////////////////////////////////////////////////////////////////////
	//  MeshWidget
	//////////////////////////////////////////////////////////////////////////////////////////////
	MeshWidget::MeshWidget(Tool* parent, int index, const std::string& material_name, int bone_index) :QListWidgetItem(std::string("Mesh " + std::to_string(index)).c_str()), parent(parent), index(index) {
		parameter.name = text().toStdString().data();
		parameter.materialName = material_name;
		parameter.boneIndex = bone_index;
		parameter.boneList = {};
	}
	MeshWidget::~MeshWidget() = default;
	int MeshWidget::GetIndex() { return index; }
	void MeshWidget::OpenMeshWindow() {
		parent->ShowMeshWindow(parameter);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////
	//  MaterialWidget
	//////////////////////////////////////////////////////////////////////////////////////////////
	MaterialWidget::MaterialWidget(Tool* parent, int index, std::string& texture_path, std::shared_ptr<Lobelia::Graphics::Material>& material) :QListWidgetItem(material->GetName().c_str()), parent(parent), index(index), parameter{ nullptr,material } {
		texturePath = texture_path;
		parameter.texturePath = &texturePath;
	}
	MaterialWidget::~MaterialWidget() {
	}
	int MaterialWidget::GetIndex() { return index; }
	void MaterialWidget::OpenMaterialWindow() {
		parent->ShowMaterialWindow(parameter);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////
	//  ModelMolding::Mesh
	//////////////////////////////////////////////////////////////////////////////////////////////
	void ModelMolding::Mesh::CreateDataToDirectX() {
		//���_�o�b�t�@�쐬
		mesh = std::make_unique<Graphics::Mesh<Vertex>>(indexCount);
		for (int i = 0; i < indexCount; i++) {
			mesh->GetBuffer()[i].pos = vertices[i].pos;
			mesh->GetBuffer()[i].normal = vertices[i].normal;
			mesh->GetBuffer()[i].tex = vertices[i].tex;
			for (int j = 0; j < 4; j++) {
				mesh->GetBuffer()[i].clusterIndex[j] = vertices[i].clusterIndex[j];
				mesh->GetBuffer()[i].weights[j] = vertices[i].weights[j];
			}
		}
	}
	void ModelMolding::Mesh::Render(ModelMolding* model) {
		mesh->Set();
		auto& material = model->materials[materialName];
		if (!material->material->IsVisible())return;
		material->material->Set();
		//�`��
		Graphics::Device::GetContext()->Draw(indexCount, 0);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////
	//  ModelMolding::Material
	//////////////////////////////////////////////////////////////////////////////////////////////
	ModelMolding::Material::Material(const char* name, const char* texture_path) {
		//�}�e���A���\��
		textureName = texture_path;
		material = std::make_unique<Graphics::Material>(name, texture_path);
		_findfirst(textureName.c_str(), &fileDate);
	}
	ModelMolding::Material::~Material() = default;
	void ModelMolding::Material::Update() {
		_finddata_t now;
		_findfirst(textureName.c_str(), &now);
		if (now.time_access > fileDate.time_access) {
			//�A�N�Z�X�����擾�ł���܂őҋ@
			while (access(textureName.c_str(), 2 | 4) != 0);
			Graphics::TextureFileAccessor::Load(textureName.c_str(), nowTexture);
			material->ChangeTexture(nowTexture);
			fileDate = now;
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	ModelMolding::Animation::Animation() :constantBuffer(std::make_unique<Graphics::ConstantBuffer<Constant>>(3, Graphics::ShaderStageList::VS)), time(0.0f) {
		std::string name;
		framePerCount = 0;
		frameCount = 0;
		meshCount = 0;

	}
	ModelMolding::Animation::~Animation() = default;
	void ModelMolding::Animation::AddElapsedTime(float time) {
		this->time += time;
		//�A�j���[�V�����̍ő�l���擾
		int animationMax = (frameCount - 1)*(1000 / framePerCount);
		while (this->time >= animationMax)this->time -= animationMax;
	}
	void ModelMolding::Animation::Update(int mesh_index) {
		//��ԓ������Ⴒ���Ⴕ�Ȃ��Ƃ����Ȃ�
		for (int i = 0; i < clusterCount[mesh_index]; i++) {
			DirectX::XMMATRIX renderTransform = DirectX::XMLoadFloat4x4(&keyFrames[mesh_index][i][static_cast<int>(time / (1000 / framePerCount))]);
			renderTransform = DirectX::XMMatrixTranspose(renderTransform);
			//�{���͂����ŕ��
			DirectX::XMStoreFloat4x4(&buffer.keyFrame[i], renderTransform);
		}
		constantBuffer->Activate(buffer);
	}
	const std::string& ModelMolding::Animation::GetName() { return name; }

	//////////////////////////////////////////////////////////////////////////////////////////////
	//  ModelMolding
	//////////////////////////////////////////////////////////////////////////////////////////////
	ModelMolding::ModelMolding(FbxImporter* importer, Tool* tool) {
		try {
			tool->GetUI().label_2->setText(importer->GetSaveAppName().c_str());
			FL::Model* model = importer->GetModel();
			meshCount = model->GetMeshCount();
			meshes.reserve(meshCount);
			renderMeshIndex.resize(meshCount);
			for (int i = 0; i < meshCount; i++) {
				ConfigureStaticMesh(model->GetMesh(i));
			}
			materialCount = model->GetMaterialCount();
			std::string directory = Utility::FilePathControl::GetParentDirectory(importer->GetPath());
			if (!directory.empty())directory += "/";
			//�}�e���A���\��
			mwidgets.resize(materialCount);
			for (int i = 0; i < materialCount; i++) {
				FL::Material* material = model->GetMaterial(i);
				materials[material->GetName()] = std::make_unique<Material>(material->GetName().c_str(), (directory + material->GetTexture(0)).c_str());
				materials[material->GetName()]->material->SetTexColor(0xFFFFFFFF);
				mwidgets[i] = std::make_shared<MaterialWidget>(tool, i, materials[material->GetName()]->textureName, materials[material->GetName()]->material);
				tool->GetUI().listMaterial->addItem(mwidgets[i].get());
			}
			widgets.resize(meshCount);
			for (int i = 0; i < meshCount; i++) {
				skinCount = model->GetMesh(i)->GetSkinCount();
				if (skinCount == 0) {
					widgets[i] = std::make_shared<MeshWidget>(tool, i, meshes[i]->materialName, 0);
					tool->GetUI().listWidget->addItem(widgets[i].get());
					continue;
				}
				ConfigureBoneInfo(i, model->GetMesh(i)->GetSkin(0));
				widgets[i] = std::make_shared<MeshWidget>(tool, i, meshes[i]->materialName, meshes[i]->clusterCount);
				tool->GetUI().listWidget->addItem(widgets[i].get());
			}
			for (int i = 0; i < meshCount; i++) {
				//�d�v�A���_�o�b�t�@�̍쐬�����s���B
				meshes[i]->CreateDataToDirectX();
			}
			ConfigureAnimation(importer);
			CreateDirectXState();
			timer.Begin();
		}
		catch (const Exception& e) {
			e.BoxMessage();
		}
	}

	ModelMolding::~ModelMolding() = default;
	MeshWidget* ModelMolding::GetMeshWidget(int index) { return widgets[index].get(); }
	MaterialWidget* ModelMolding::GetMaterialWidget(int index) { return mwidgets[index].get(); }
	void ModelMolding::CreateDirectXState() {
		//�f�t�H���g�̒��_�V�F�[�_�[�擾
		Graphics::VertexShader* vs = Graphics::ShaderBank::Get<Graphics::VertexShader>(DEFAULT_VERTEX_SHADER_STATIC_MODEL);
		//���t���N�V�����J�n
		std::unique_ptr<Graphics::Reflection> reflector = std::make_unique<Graphics::Reflection>(vs);
		//���̓��C�A�E�g�쐬
		inputLayout = std::make_unique<Graphics::InputLayout>(vs, reflector.get());
		//�R���X�^���g�o�b�t�@�쐬
		constantBuffer = std::make_unique<Graphics::ConstantBuffer<DirectX::XMMATRIX>>(1, Graphics::ShaderStageList::VS);
		//���[���h�ϊ��s��(�P�ʍs��)
		world = DirectX::XMMatrixIdentity();
		selectMesh = -1;
	}
	void ModelMolding::ConfigureStaticMesh(FL::Mesh* mesh) {
		std::shared_ptr<Mesh> meshTemp = std::make_shared<Mesh>();
		//��{���擾
		meshTemp->indexCount = mesh->GetIndexCount();
		meshTemp->uvCount = mesh->GetUVCount();
		if (meshTemp->indexCount != meshTemp->uvCount)STRICT_THROW("���߂Č���t�@�C���t�H�[�}�b�g�ł��A���̗�O���b�Z�[�W�𐧍�҂ɘA�����Ă�������");//�Ƃ������Ahumanoid.fbx�͂������ɗ���񂶂�Ȃ����ȂƗ\��
		//���_�p�o�b�t�@���m��
		meshTemp->vertices.resize(meshTemp->indexCount);
		//���_���\�z
		for (int i = 0; i < meshTemp->indexCount; i++) {
			int index = mesh->GetIndexBuffer(i);
			memcpy_s(&meshTemp->vertices[i].pos, sizeof(Vector4), &mesh->GetVertex(index), sizeof(Vector3));
			meshTemp->vertices[i].pos.w = 1.0f;
			memcpy_s(&meshTemp->vertices[i].normal, sizeof(Vector4), &mesh->GetNormal(index), sizeof(Vector3));
			memcpy_s(&meshTemp->vertices[i].tex, sizeof(Vector2), &mesh->GetUV(i), sizeof(Vector2));
		}
		//�Ή��}�e���A�����擾
		meshTemp->materialName = mesh->GetMaterialName();
		//�f�[�^��ǉ�
		meshes.push_back(meshTemp);
	}
	void ModelMolding::ConfigureBoneInfo(int mesh_count, FL::Skin* skin) {
		struct BoneInfo {
			int clusterIndex;
			float weight;
		};
		meshes[mesh_count]->clusterCount = skin->GetClusterCount();
		//���_����
		std::vector<std::vector<BoneInfo>> boneInfos(meshes[mesh_count]->indexCount);
		//�o�b�t�@�m��
		meshes[mesh_count]->initPoseMatrices.resize(meshes[mesh_count]->clusterCount);
		meshes[mesh_count]->impactSize.resize(meshes[mesh_count]->indexCount);
		//�f�[�^�p�[�X
		for (int clusterIndex = 0; clusterIndex < meshes[mesh_count]->clusterCount; clusterIndex++) {
			FL::Cluster* cluster = skin->GetCluster(clusterIndex);
			//���̃N���X�^�[���e�����Ă��钸�_�����擾
			int impactIndexCount = cluster->GetIndexCount();
			//�e�����Ă��钸�_�����񂵂ăf�[�^�̎d����
			for (int impactIndex = 0; impactIndex < impactIndexCount; impactIndex++) {
				BoneInfo info = {};
				//���g�̉e�����_���ׂĂ���̂ŉe���N���X�^�[�͎��g
				info.clusterIndex = clusterIndex;
				//���̑Ώۂ̒��_���󂯂�d�݂��Z�b�g
				info.weight = cluster->GetWeight(impactIndex);
				//�G���[�`�F�b�N����ۂ͂�����
				if (info.weight<0.0f || info.weight>1.0f);
				//�ǂ̒��_�̉e�����擾
				int vertexPointIndex = cluster->GetImpactIndex(impactIndex);
				meshes[mesh_count]->impactSize[vertexPointIndex]++;
				//�ǉ�
				boneInfos[vertexPointIndex].push_back(info);
			}
			//�����p���s��擾 ����������u
			meshes[mesh_count]->initPoseMatrices[clusterIndex] = cluster->GetInitPoseMatrix();
		}
		//�\�z
		for (int i = 0; i < meshes[mesh_count]->indexCount; i++) {
			for (int j = 0; j < boneInfos[i].size(); j++) {
				meshes[mesh_count]->vertices[i].clusterIndex[j] = boneInfos[i][j].clusterIndex;
				meshes[mesh_count]->vertices[i].weights[j] = boneInfos[i][j].weight;
			}
		}
	}
	void ModelMolding::ConfigureAnimation(FbxImporter* importer) {
		if (!importer)STRICT_THROW("�t�@�C�����ǂݍ��܂�Ă��Ȃ��\��������܂�");
		FL::Model* model = importer->GetModel();
		if (meshes[0]->clusterCount == 0)return;
		int animationCount = model->GetMesh(0)->GetSkin(0)->GetCluster(0)->GetAnimationCount();
		animations.resize(animationCount);
		//�A�j���[�V�����̐�
		for (int animationIndex = 0; animationIndex < animationCount; animationIndex++) {
			std::shared_ptr<Animation> animation = std::make_shared<Animation>();
			animation->name = model->GetMesh(0)->GetSkin(0)->GetCluster(0)->GetAnimationTake(animationIndex)->GetTakeName();
			animation->framePerCount = FL::System::GetInstance()->sampleFramePerCount;
			FL::AnimationTake* take = model->GetMesh(0)->GetSkin(0)->GetCluster(0)->GetAnimationTake(animationIndex);
			animation->frameCount = take->GetMatrixSize();
			animation->keyFrames.resize(meshCount);
			animation->meshCount = meshCount;
			for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
				if (model->GetMesh(meshIndex)->GetSkinCount() == 0) {
					animation->clusterCount.push_back(0);
					animation->keyFrames[meshIndex].resize(0);
					continue;
				}
				FL::Skin* skin = model->GetMesh(meshIndex)->GetSkin(0);
				int clusterCount = skin->GetClusterCount();
				animation->clusterCount.push_back(clusterCount);
				animation->keyFrames[meshIndex].resize(clusterCount);
				for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
					ConfigureKeyFrames(skin->GetCluster(clusterIndex), meshIndex, clusterIndex, animationIndex, animation);
				}
			}
			animations[animationIndex] = animation;
		}
	}
	//�v���g�^�C�v�錾
	void LoadMatrix(DirectX::XMFLOAT4X4* mat0, const Matrix& mat1);
	void ModelMolding::ConfigureKeyFrames(FL::Cluster* cluster, int mesh_index, int cluster_index, int animation_index, std::shared_ptr<Animation>& animation) {
		//�����p���s��擾
		Matrix initPoseMatrix = cluster->GetInitPoseMatrix();
		DirectX::XMFLOAT4X4 ipose = {};
		LoadMatrix(&ipose, initPoseMatrix);
		DirectX::XMMATRIX initPose = {};
		//�v�Z�p�̌^�ɗ���
		initPose = DirectX::XMLoadFloat4x4(&ipose);
		DirectX::XMVECTOR arg = {};
		//�����p���̋t�s����Z�o
		DirectX::XMMATRIX inverseInit = DirectX::XMMatrixInverse(&arg, initPose);
		FL::AnimationTake* take = cluster->GetAnimationTake(animation_index);
		animation->keyFrames[mesh_index][cluster_index].resize(animation->frameCount);
		for (int i = 0; i < animation->frameCount; i++) {
			Matrix ctemp = take->GetCurrentPoseMatrix(i);
			DirectX::XMFLOAT4X4 currentPose = {};
			//�s���DirectXMath�̌^�ɓ��ꍞ��
			LoadMatrix(&currentPose, ctemp);
			DirectX::XMMATRIX current;
			//�v�Z�p�̌^�Ɉڂ��ւ�
			current = DirectX::XMLoadFloat4x4(&currentPose);
			//�o�C���h�|�[�Y�s��̋t�s��ƁA�J�����g�|�[�Y�s��������Ď��O�v�Z
			DirectX::XMMATRIX frame = inverseInit * current;
			//�ۑ��p�̌^�ɕϊ��A�󂯎����ۑ��p�ϐ�
			DirectX::XMStoreFloat4x4(&animation->keyFrames[mesh_index][cluster_index][i], frame);
		}
	}
	void ModelMolding::Update(Tool* tool) {
		for each(auto& material in materials) {
			material.second->Update();
		}
		bool selected = false;
		for (int i = 0; i < meshCount; i++) {
			MeshWidget* item = dynamic_cast<MeshWidget*>(tool->GetUI().listWidget->item(i));
			renderMeshIndex[i] = item->GetIndex();
			if (tool->GetUI().listWidget->isItemSelected(item)) {
				selectMesh = i;
				selected = true;
			}
		}
		if (!selected)selectMesh = -1;
	}
	void ModelMolding::Render(Tool* tool) {
		inputLayout->Set();
		world = DirectX::XMMatrixScaling(0.3f, 0.3f, 0.3f);
		constantBuffer->Activate(DirectX::XMMatrixTranspose(world));
		Graphics::Device::GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//auto pipeline = Graphics::PipelineManager::PipelineGet(DEFAULT_PIPELINE_DYNAMIC_MODEL);
		Graphics::Pipeline* pipeline;
		bool hasAnimation = !animations.empty();
		if (hasAnimation)	pipeline = Graphics::PipelineManager::PipelineGet(DEFAULT_PIPELINE_DYNAMIC_MODEL);
		else pipeline = Graphics::PipelineManager::PipelineGet(DEFAULT_PIPELINE_STATIC_MODEL);
		pipeline->Activate(true);
		timer.End();
		if (hasAnimation)animations[0]->AddElapsedTime(timer.GetMilisecondResult());
		timer.Begin();
		for (int i = 0; i < meshCount; i++) {
			int index = renderMeshIndex[i];
			if (hasAnimation)animations[0]->Update(index);
			//���b�V�����I������Ă���Ƃ�
			if (index == selectMesh) world = DirectX::XMMatrixScaling(0.4f, 0.4f, 0.4f);
			else world = DirectX::XMMatrixScaling(0.3f, 0.3f, 0.3f);
			constantBuffer->Activate(DirectX::XMMatrixTranspose(world));
			meshes[index]->Render(this);
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
}