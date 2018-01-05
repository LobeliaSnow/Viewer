#include "Tool.h"

#define EXCEPTION_FC(fc)		if (!fc)STRICT_THROW("�t�@�C������Ɏ��s���܂���");

namespace Lobelia {
	void LoadMatrix(DirectX::XMFLOAT4X4* mat0, const Matrix& mat1) {
		for (int row = 0; row < 4; row++) {
			for (int column = 0; column < 4; column++) {
				mat0->m[row][column] = mat1.mat[row][column];
			}
		}
	}
	namespace Future {
		//Animation�̃G�N�X�|�[�^�[���t���Ȃ��ƕ`�揇�ύX�����ۂɋl��
		DxdExporter::DxdExporter() = default;
		DxdExporter::~DxdExporter() = default;
		void DxdExporter::MeshSave(std::weak_ptr<Utility::FileController> file, ModelMolding::Mesh* mesh) {
			std::shared_ptr<Utility::FileController> fc = file.lock();
			EXCEPTION_FC(fc);
			//�C���f�b�N�X���擾
			int indexCount = mesh->indexCount;
			//UV���擾
			int uvCount = mesh->uvCount;
			//�C���f�b�N�X���o��
			fc->Write(&indexCount, 1);
			//UV���o��
			fc->Write(&uvCount, 1);
			if (indexCount != uvCount)STRICT_THROW("���߂Č���t�@�C���t�H�[�}�b�g�ł��A���̗�O���b�Z�[�W�𐧍�҂ɘA�����Ă�������");//�Ƃ������Ahumanoid.fbx�͂������ɗ���񂶂�Ȃ����ȂƗ\��
			std::vector<WriteVertex> vertices(indexCount);
			for (int i = 0; i < indexCount; i++) {
				memcpy_s(&vertices[i].pos, sizeof(Vector4), &mesh->vertices[i].pos, sizeof(Vector3));
				vertices[i].pos.w = 1.0f;
				memcpy_s(&vertices[i].normal, sizeof(Vector4), &mesh->vertices[i].normal, sizeof(Vector3));
				memcpy_s(&vertices[i].tex, sizeof(Vector2), &mesh->vertices[i].tex, sizeof(Vector2));
			}
			//���_�ʒu�A�@���AUV�ۑ�
			fc->Write(vertices.data(), indexCount);
			int materialNameSize = mesh->materialName.size() + 1;
			//�}�e���A�����̒������o��
			fc->Write(&materialNameSize, 1);
			//�}�e���A�����o��
			fc->Write(mesh->materialName.c_str(), materialNameSize);
		}
		void DxdExporter::ClusterSave(std::weak_ptr<Utility::FileController> file, ModelMolding::Mesh* mesh) {
			std::shared_ptr<Utility::FileController> fc = file.lock();
			EXCEPTION_FC(fc);
			//���_���擾
			int indexCount = mesh->indexCount;
			//���z
			//���_��->�e����
			//���_�����o�b�t�@�m��
			int clusterCount = mesh->clusterCount;
			//�o�͕�
			//���_���o��
			fc->Write(&indexCount, 1);
			for (int i = 0; i < indexCount; i++) {
				int impactSize = mesh->impactSize[i];
				//�e�����ۑ�
				fc->Write(&impactSize, 1);
				std::vector<BoneInfo> boneInfo(impactSize);
				for (int j = 0; j < impactSize; j++) {
					boneInfo[j].clusterIndex = mesh->vertices[i].clusterIndex[j];
					boneInfo[j].weight = mesh->vertices[i].weights[j];
				}
				//�e���x�ۑ�
				fc->Write(boneInfo.data(), impactSize);
			}
			//�N���X�^�[���o��
			fc->Write(&clusterCount, 1);
			//�����p���s��o��
			fc->Write(mesh->initPoseMatrices.data(), clusterCount);
		}
		void DxdExporter::Save(ModelMolding* model, const char* file_path) {
			if (!model)STRICT_THROW("�t�@�C�����ǂݍ��܂�Ă��Ȃ��\��������܂�");
			std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
			fc->Open(file_path, Utility::FileController::OpenMode::WriteBinary);
			if (!fc->IsOpen())STRICT_THROW("�ۑ��p�t�@�C���I�[�v���Ɏ��s");
			//���b�V�����擾
			int meshCount = model->meshCount;
			if (meshCount <= 0)STRICT_THROW("���b�V����������܂���ł����B���f�����s�K�i�ȉ\��������܂�");
			//���b�V�����o��
			fc->Write(&meshCount, 1);
			//�X�L���̐����擾 ����1���b�V���ɂ�1�X�L���������Ă�Ȃ����߂���ȊO������ƕs�K�i
			for (int i = 0; i < meshCount; i++) {
				//���b�V���ۑ�
				MeshSave(fc, model->meshes[model->renderMeshIndex[i]].get());
			}
			for (int i = 0; i < meshCount; i++) {
				//�����A�j���[�V�������Ȃ������ꍇ�̓N���X�^�[���Ȃ��̂ŏ����o���I��
				bool haveSkin = (model->skinCount);
				//�X�L�������݂��邩�ǂ����̃t���O���o��
				fc->Write(&haveSkin, 1);
				if (!haveSkin)continue;
				//�N���X�^�[�ۑ�
				ClusterSave(fc, model->meshes[model->renderMeshIndex[i]].get());
			}
			//�I��
			fc->Close();
		}
		MaterialExporter::MaterialExporter() = default;
		MaterialExporter::~MaterialExporter() = default;
		void MaterialExporter::Save(ModelMolding* model, const char* file_path) {
			if (!model)STRICT_THROW("�t�@�C�����ǂݍ��܂�Ă��Ȃ��\��������܂�");
			std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
			fc->Open(file_path, Utility::FileController::OpenMode::WriteBinary);
			if (!fc->IsOpen())STRICT_THROW("�ۑ��p�t�@�C���I�[�v���Ɏ��s");
			//�}�e���A�����擾
			int materialCount = model->materialCount;
			if (materialCount <= 0)STRICT_THROW("�}�e���A�������݂��܂���");
			//�}�e���A�����o��
			fc->Write(&materialCount, 1);
			for (int i = 0; i < materialCount; i++) {
				Graphics::Material* material = model->mwidgets[i]->parameter.material.get();
				//�}�e���A�����T�C�Y�擾
				int materialNameLength = material->GetName().size() + 1;
				//�}�e���A�����T�C�Y��������
				fc->Write(&materialNameLength, 1);
				//�}�e���A�����o��
				fc->Write(material->GetName().c_str(), materialNameLength);
				//�����ŕύX���ꂽ�e�N�X�`�������擾�B
				std::string textureName = Utility::FilePathControl::GetFilename(*model->mwidgets[i]->parameter.texturePath);
				//�e�N�X�`�����̃T�C�Y�擾
				int textureNameSize = textureName.size() + 1;
				//�T�C�Y��������
				fc->Write(&textureNameSize, 1);
				//�e�N�X�`���t�@�C�����o��
				fc->Write(textureName.c_str(), textureNameSize);
			}
			//�I��
			fc->Close();
		}
	}
	namespace Ancient {
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		//		dxd
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		DxdExporter::DxdExporter() = default;
		DxdExporter::~DxdExporter() = default;

		void DxdExporter::MeshSave(std::weak_ptr<Utility::FileController> file, FL::Mesh* mesh) {
			std::shared_ptr<Utility::FileController> fc = file.lock();
			EXCEPTION_FC(fc);
			//�C���f�b�N�X���擾
			int indexCount = mesh->GetIndexCount();
			//UV���擾
			int uvCount = mesh->GetUVCount();
			//�C���f�b�N�X���o��
			fc->Write(&indexCount, 1);
			//UV���o��
			fc->Write(&uvCount, 1);
			if (indexCount != uvCount)STRICT_THROW("���߂Č���t�@�C���t�H�[�}�b�g�ł��A���̗�O���b�Z�[�W�𐧍�҂ɘA�����Ă�������");//�Ƃ������Ahumanoid.fbx�͂������ɗ���񂶂�Ȃ����ȂƗ\��
			std::vector<WriteVertex> vertices(indexCount);
			for (int i = 0; i < indexCount; i++) {
				int index = mesh->GetIndexBuffer(i);
				memcpy_s(&vertices[i].pos, sizeof(Vector4), &mesh->GetVertex(index), sizeof(Vector3));
				vertices[i].pos.w = 1.0f;
				memcpy_s(&vertices[i].normal, sizeof(Vector4), &mesh->GetNormal(index), sizeof(Vector3));
				memcpy_s(&vertices[i].tex, sizeof(Vector2), &mesh->GetUV(i), sizeof(Vector2));
			}
			//���_�ʒu�A�@���AUV�ۑ�
			fc->Write(vertices.data(), indexCount);
			int materialNameSize = mesh->GetMaterialName().size() + 1;
			//�}�e���A�����̒������o��
			fc->Write(&materialNameSize, 1);
			//�}�e���A�����o��
			fc->Write(mesh->GetMaterialName().c_str(), materialNameSize);
		}
		//���b�V���P�ʂŌĂ΂��
		void DxdExporter::ClusterSave(std::weak_ptr<Utility::FileController> file, FL::Mesh* mesh) {
			std::shared_ptr<Utility::FileController> fc = file.lock();
			EXCEPTION_FC(fc);
			//���_���擾
			int indexCount = mesh->GetIndexCount();
			//���z
			//���_��->�e����
			//���_�����o�b�t�@�m��
			std::vector<std::vector<BoneInfo>> boneInfo(indexCount);
			FL::Skin* skin = mesh->GetSkin(0);
			int clusterCount = skin->GetClusterCount();
			std::vector<Matrix> initPoseMatrices(clusterCount);
			//�f�[�^�p�[�X
			for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
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
					boneInfo[vertexPointIndex].push_back(info);
				}
				//�����p���s��擾
				initPoseMatrices[clusterIndex] = cluster->GetInitPoseMatrix();
			}
			//�o�͕�
			//���_���o��
			fc->Write(&indexCount, 1);
			for (int i = 0; i < indexCount; i++) {
				int impactSize = boneInfo[i].size();
				//�e�����ۑ�
				fc->Write(&impactSize, 1);
				//�e���x�ۑ�
				fc->Write(boneInfo[i].data(), impactSize);
			}
			//�N���X�^�[���o��
			fc->Write(&clusterCount, 1);
			//�����p���s��o��
			fc->Write(initPoseMatrices.data(), clusterCount);

		}
		void DxdExporter::Save(FbxImporter* importer, const char* file_path) {
			if (!importer)STRICT_THROW("�t�@�C�����ǂݍ��܂�Ă��Ȃ��\��������܂�");
			std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
			fc->Open(file_path, Utility::FileController::OpenMode::WriteBinary);
			if (!fc->IsOpen())STRICT_THROW("�ۑ��p�t�@�C���I�[�v���Ɏ��s");
			FL::Model* model = importer->GetModel();
			//���b�V�����擾
			int meshCount = model->GetMeshCount();
			if (meshCount <= 0)STRICT_THROW("���b�V����������܂���ł����B���f�����s�K�i�ȉ\��������܂�");
			//���b�V�����o��
			fc->Write(&meshCount, 1);
			//�X�L���̐����擾 ����1���b�V���ɂ�1�X�L���������Ă�Ȃ����߂���ȊO������ƕs�K�i
			for (int i = 0; i < meshCount; i++) {
				//���b�V���ۑ�
				MeshSave(fc, model->GetMesh(i));
			}
			for (int i = 0; i < meshCount; i++) {
				//�����X�L�����Ȃ������ꍇ�̓N���X�^�[���Ȃ��̂ŏ����o���I��
				bool haveSkin = (model->GetMesh(i)->GetSkinCount() > 0);
				//�X�L�������݂��邩�ǂ����̃t���O���o��
				fc->Write(&haveSkin, 1);
				if (!haveSkin)continue;
				FL::Mesh* mesh = model->GetMesh(i);
				//�N���X�^�[�ۑ�
				ClusterSave(fc, mesh);
			}
			//�I��
			fc->Close();
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	material
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		MaterialExporter::MaterialExporter() = default;
		MaterialExporter::~MaterialExporter() = default;
		void MaterialExporter::Save(FbxImporter* importer, const char* file_path) {
			if (!importer)STRICT_THROW("�t�@�C�����ǂݍ��܂�Ă��Ȃ��\��������܂�");
			std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
			fc->Open(file_path, Utility::FileController::OpenMode::WriteBinary);
			if (!fc->IsOpen())STRICT_THROW("�ۑ��p�t�@�C���I�[�v���Ɏ��s");
			FL::Model* model = importer->GetModel();
			//�}�e���A�����擾
			int materialCount = model->GetMaterialCount();
			if (materialCount <= 0)STRICT_THROW("�}�e���A�������݂��܂���");
			//�}�e���A�����o��
			fc->Write(&materialCount, 1);
			for (int i = 0; i < materialCount; i++) {
				//�}�e���A���擾
				FL::Material* material = model->GetMaterial(i);
				//�}�e���A�����T�C�Y�擾
				int materialNameLength = material->GetName().size() + 1;
				//�}�e���A�����T�C�Y��������
				fc->Write(&materialNameLength, 1);
				//�}�e���A�����o��
				fc->Write(material->GetName().c_str(), materialNameLength);
				//�e�N�X�`�����̃T�C�Y�擾
				int textureNameSize = material->GetTexture(0).size() + 1;
				//�T�C�Y��������
				fc->Write(&textureNameSize, 1);
				//�e�N�X�`���t�@�C�����o��
				fc->Write(material->GetTexture(0).c_str(), textureNameSize);
			}
			//�I��
			fc->Close();
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		// animation
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		AnimationExporter::AnimationExporter() = default;
		AnimationExporter::~AnimationExporter() = default;
		void AnimationExporter::KeyFrameSave(std::weak_ptr<Utility::FileController> file, FL::Cluster* cluster, int animation_index) {
			std::shared_ptr<Utility::FileController> fc = file.lock();
			EXCEPTION_FC(fc);
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
			//�L�[�t���[�����擾
			int keyFrameCount = take->GetMatrixSize();
			std::vector<DirectX::XMFLOAT4X4> matrix(keyFrameCount);
			//���̂�����Ƃ��͏�̊֐�������������Ă��Ďg���܂킹��
			for (int i = 0; i < keyFrameCount; i++) {
				Matrix ctemp = take->GetCurrentPoseMatrix(i);
				DirectX::XMFLOAT4X4 currentPose = {};
				//�s���DirectXMath�̌^�ɓ��ꍞ��
				LoadMatrix(&currentPose, ctemp);
				DirectX::XMMATRIX current;
				//�v�Z�p�̌^�Ɉڂ��ւ�
				current = DirectX::XMLoadFloat4x4(&currentPose);
				//�o�C���h�|�[�Y�s��̋t�s��ƁA�J�����g�|�[�Y�s��������Ď��O�v�Z
				DirectX::XMMATRIX frame = inverseInit*current;
				//�ۑ��p�̌^�ɕϊ��A�󂯎����ۑ��p�ϐ�
				DirectX::XMStoreFloat4x4(&matrix[i], frame);
			}
			//�L�[�t���[���o��
			fc->Write(matrix.data(), keyFrameCount);
		}
		//���������������Ă��ꂢ�ɂ��悤
		void AnimationExporter::Save(FbxImporter* importer, const char* directroy) {
			if (!importer)STRICT_THROW("�t�@�C�����ǂݍ��܂�Ă��Ȃ��\��������܂�");
			FL::Model* model = importer->GetModel();
			//���b�V�����擾
			int meshCount = model->GetMeshCount();
			//�ꉞ���b�V�����`�F�b�N
			if (meshCount <= 0)STRICT_THROW("���b�V��������܂���B���f�����s�K�i�ȉ\��������܂�");
			int animationCount = model->GetMesh(0)->GetSkin(0)->GetCluster(0)->GetAnimationCount();
			//�A�j���[�V�����̐�
			for (int animationIndex = 0; animationIndex < animationCount; animationIndex++) {
				std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
				std::string takeName = model->GetMesh(0)->GetSkin(0)->GetCluster(0)->GetAnimationTake(animationIndex)->GetTakeName();
				fc->Open((directroy + takeName + ".anm").c_str(), Utility::FileController::OpenMode::WriteBinary);
				if (!fc->IsOpen())STRICT_THROW("�ۑ��p�t�@�C���I�[�v���Ɏ��s");
				//�A�j���[�V�����e�C�N���̒����擾
				int nameSize = takeName.size() + 1;
				//�A�j���[�V�����e�C�N���̒����o��
				fc->Write(&nameSize, 1);
				//�A�j���[�V�����e�C�N���o��
				fc->Write(takeName.c_str(), nameSize);
				//���̃��C�u������1�b�����艽�t���[���T���v�������̂��擾
				int framePerCount = FL::System::GetInstance()->sampleFramePerCount;
				//���̃��C�u������1�b�����艽�t���[���T���v�������̂��o��
				fc->Write(&framePerCount, 1);
				FL::AnimationTake* take = model->GetMesh(0)->GetSkin(0)->GetCluster(0)->GetAnimationTake(animationIndex);
				//�L�[�t���[�����擾
				int keyFrameCount = take->GetMatrixSize();
				//�L�[�t���[�����o��
				fc->Write(&keyFrameCount, 1);
				//���b�V�����o��
				fc->Write(&meshCount, 1);
				//���b�V���̐�
				for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
					if (model->GetMesh(meshIndex)->GetSkinCount() == 0) {
						int clusterCount = 0;
						//�N���X�^�[���ۑ�
						fc->Write(&clusterCount, 1);
						continue;
					}
					FL::Skin* skin = model->GetMesh(meshIndex)->GetSkin(0);
					int clusterCount = skin->GetClusterCount();
					//�N���X�^�[���ۑ�
					fc->Write(&clusterCount, 1);
					//�N���X�^�[�̐�
					for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
						KeyFrameSave(fc, skin->GetCluster(clusterIndex), animationIndex);
					}
				}
				fc->Close();
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
	}
}