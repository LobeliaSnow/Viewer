#include "Tool.h"

#define EXCEPTION_FC(fc)		if (!fc)STRICT_THROW("ファイル操作に失敗しました");

namespace Lobelia {
	void LoadMatrix(DirectX::XMFLOAT4X4* mat0, const Matrix& mat1) {
		for (int row = 0; row < 4; row++) {
			for (int column = 0; column < 4; column++) {
				mat0->m[row][column] = mat1.mat[row][column];
			}
		}
	}
	namespace Future {
		//Animationのエクスポーターも付けないと描画順変更した際に詰む
		DxdExporter::DxdExporter() = default;
		DxdExporter::~DxdExporter() = default;
		void DxdExporter::MeshSave(std::weak_ptr<Utility::FileController> file, ModelMolding::Mesh* mesh) {
			std::shared_ptr<Utility::FileController> fc = file.lock();
			EXCEPTION_FC(fc);
			//インデックス数取得
			int indexCount = mesh->indexCount;
			//UV数取得
			int uvCount = mesh->uvCount;
			//インデックス数出力
			fc->Write(&indexCount, 1);
			//UV数出力
			fc->Write(&uvCount, 1);
			if (indexCount != uvCount)STRICT_THROW("初めて見るファイルフォーマットです、この例外メッセージを制作者に連絡してください");//とかいいつつ、humanoid.fbxはこっちに来るんじゃないかなと予測
			std::vector<WriteVertex> vertices(indexCount);
			for (int i = 0; i < indexCount; i++) {
				memcpy_s(&vertices[i].pos, sizeof(Vector4), &mesh->vertices[i].pos, sizeof(Vector3));
				vertices[i].pos.w = 1.0f;
				memcpy_s(&vertices[i].normal, sizeof(Vector4), &mesh->vertices[i].normal, sizeof(Vector3));
				memcpy_s(&vertices[i].tex, sizeof(Vector2), &mesh->vertices[i].tex, sizeof(Vector2));
			}
			//頂点位置、法線、UV保存
			fc->Write(vertices.data(), indexCount);
			int materialNameSize = mesh->materialName.size() + 1;
			//マテリアル名の長さを出力
			fc->Write(&materialNameSize, 1);
			//マテリアル名出力
			fc->Write(mesh->materialName.c_str(), materialNameSize);
		}
		void DxdExporter::ClusterSave(std::weak_ptr<Utility::FileController> file, ModelMolding::Mesh* mesh) {
			std::shared_ptr<Utility::FileController> fc = file.lock();
			EXCEPTION_FC(fc);
			//頂点数取得
			int indexCount = mesh->indexCount;
			//理想
			//頂点数->影響数
			//頂点数分バッファ確保
			int clusterCount = mesh->clusterCount;
			//出力部
			//頂点数出力
			fc->Write(&indexCount, 1);
			for (int i = 0; i < indexCount; i++) {
				int impactSize = mesh->impactSize[i];
				//影響数保存
				fc->Write(&impactSize, 1);
				std::vector<BoneInfo> boneInfo(impactSize);
				for (int j = 0; j < impactSize; j++) {
					boneInfo[j].clusterIndex = mesh->vertices[i].clusterIndex[j];
					boneInfo[j].weight = mesh->vertices[i].weights[j];
				}
				//影響度保存
				fc->Write(boneInfo.data(), impactSize);
			}
			//クラスター数出力
			fc->Write(&clusterCount, 1);
			//初期姿勢行列出力
			fc->Write(mesh->initPoseMatrices.data(), clusterCount);
		}
		void DxdExporter::Save(ModelMolding* model, const char* file_path) {
			if (!model)STRICT_THROW("ファイルが読み込まれていない可能性があります");
			std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
			fc->Open(file_path, Utility::FileController::OpenMode::WriteBinary);
			if (!fc->IsOpen())STRICT_THROW("保存用ファイルオープンに失敗");
			//メッシュ数取得
			int meshCount = model->meshCount;
			if (meshCount <= 0)STRICT_THROW("メッシュが見つかりませんでした。モデルが不適格な可能性があります");
			//メッシュ数出力
			fc->Write(&meshCount, 1);
			//スキンの数を取得 現在1メッシュにつき1スキンしか当てれないためそれ以外が来ると不適格
			for (int i = 0; i < meshCount; i++) {
				//メッシュ保存
				MeshSave(fc, model->meshes[model->renderMeshIndex[i]].get());
			}
			for (int i = 0; i < meshCount; i++) {
				//もしアニメーションがなかった場合はクラスターもないので書き出し終了
				bool haveSkin = (model->skinCount);
				//スキンが存在するかどうかのフラグを出力
				fc->Write(&haveSkin, 1);
				if (!haveSkin)continue;
				//クラスター保存
				ClusterSave(fc, model->meshes[model->renderMeshIndex[i]].get());
			}
			//終了
			fc->Close();
		}
		MaterialExporter::MaterialExporter() = default;
		MaterialExporter::~MaterialExporter() = default;
		void MaterialExporter::Save(ModelMolding* model, const char* file_path) {
			if (!model)STRICT_THROW("ファイルが読み込まれていない可能性があります");
			std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
			fc->Open(file_path, Utility::FileController::OpenMode::WriteBinary);
			if (!fc->IsOpen())STRICT_THROW("保存用ファイルオープンに失敗");
			//マテリアル数取得
			int materialCount = model->materialCount;
			if (materialCount <= 0)STRICT_THROW("マテリアルが存在しません");
			//マテリアル数出力
			fc->Write(&materialCount, 1);
			for (int i = 0; i < materialCount; i++) {
				Graphics::Material* material = model->mwidgets[i]->parameter.material.get();
				//マテリアル名サイズ取得
				int materialNameLength = material->GetName().size() + 1;
				//マテリアル名サイズ書き込み
				fc->Write(&materialNameLength, 1);
				//マテリアル名出力
				fc->Write(material->GetName().c_str(), materialNameLength);
				//ここで変更されたテクスチャ名を取得。
				std::string textureName = Utility::FilePathControl::GetFilename(*model->mwidgets[i]->parameter.texturePath);
				//テクスチャ名のサイズ取得
				int textureNameSize = textureName.size() + 1;
				//サイズ書き込み
				fc->Write(&textureNameSize, 1);
				//テクスチャファイル名出力
				fc->Write(textureName.c_str(), textureNameSize);
			}
			//終了
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
			//インデックス数取得
			int indexCount = mesh->GetIndexCount();
			//UV数取得
			int uvCount = mesh->GetUVCount();
			//インデックス数出力
			fc->Write(&indexCount, 1);
			//UV数出力
			fc->Write(&uvCount, 1);
			if (indexCount != uvCount)STRICT_THROW("初めて見るファイルフォーマットです、この例外メッセージを制作者に連絡してください");//とかいいつつ、humanoid.fbxはこっちに来るんじゃないかなと予測
			std::vector<WriteVertex> vertices(indexCount);
			for (int i = 0; i < indexCount; i++) {
				int index = mesh->GetIndexBuffer(i);
				memcpy_s(&vertices[i].pos, sizeof(Vector4), &mesh->GetVertex(index), sizeof(Vector3));
				vertices[i].pos.w = 1.0f;
				memcpy_s(&vertices[i].normal, sizeof(Vector4), &mesh->GetNormal(index), sizeof(Vector3));
				memcpy_s(&vertices[i].tex, sizeof(Vector2), &mesh->GetUV(i), sizeof(Vector2));
			}
			//頂点位置、法線、UV保存
			fc->Write(vertices.data(), indexCount);
			int materialNameSize = mesh->GetMaterialName().size() + 1;
			//マテリアル名の長さを出力
			fc->Write(&materialNameSize, 1);
			//マテリアル名出力
			fc->Write(mesh->GetMaterialName().c_str(), materialNameSize);
		}
		//メッシュ単位で呼ばれる
		void DxdExporter::ClusterSave(std::weak_ptr<Utility::FileController> file, FL::Mesh* mesh) {
			std::shared_ptr<Utility::FileController> fc = file.lock();
			EXCEPTION_FC(fc);
			//頂点数取得
			int indexCount = mesh->GetIndexCount();
			//理想
			//頂点数->影響数
			//頂点数分バッファ確保
			std::vector<std::vector<BoneInfo>> boneInfo(indexCount);
			FL::Skin* skin = mesh->GetSkin(0);
			int clusterCount = skin->GetClusterCount();
			std::vector<Matrix> initPoseMatrices(clusterCount);
			//データパース
			for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
				FL::Cluster* cluster = skin->GetCluster(clusterIndex);
				//このクラスターが影響している頂点数を取得
				int impactIndexCount = cluster->GetIndexCount();
				//影響している頂点だけ回してデータの仕分け
				for (int impactIndex = 0; impactIndex < impactIndexCount; impactIndex++) {
					BoneInfo info = {};
					//自身の影響頂点調べているので影響クラスターは自身
					info.clusterIndex = clusterIndex;
					//この対象の頂点が受ける重みをセット
					info.weight = cluster->GetWeight(impactIndex);
					//エラーチェックする際はここで
					if (info.weight<0.0f || info.weight>1.0f);
					//どの頂点の影響か取得
					int vertexPointIndex = cluster->GetImpactIndex(impactIndex);
					boneInfo[vertexPointIndex].push_back(info);
				}
				//初期姿勢行列取得
				initPoseMatrices[clusterIndex] = cluster->GetInitPoseMatrix();
			}
			//出力部
			//頂点数出力
			fc->Write(&indexCount, 1);
			for (int i = 0; i < indexCount; i++) {
				int impactSize = boneInfo[i].size();
				//影響数保存
				fc->Write(&impactSize, 1);
				//影響度保存
				fc->Write(boneInfo[i].data(), impactSize);
			}
			//クラスター数出力
			fc->Write(&clusterCount, 1);
			//初期姿勢行列出力
			fc->Write(initPoseMatrices.data(), clusterCount);

		}
		void DxdExporter::Save(FbxImporter* importer, const char* file_path) {
			if (!importer)STRICT_THROW("ファイルが読み込まれていない可能性があります");
			std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
			fc->Open(file_path, Utility::FileController::OpenMode::WriteBinary);
			if (!fc->IsOpen())STRICT_THROW("保存用ファイルオープンに失敗");
			FL::Model* model = importer->GetModel();
			//メッシュ数取得
			int meshCount = model->GetMeshCount();
			if (meshCount <= 0)STRICT_THROW("メッシュが見つかりませんでした。モデルが不適格な可能性があります");
			//メッシュ数出力
			fc->Write(&meshCount, 1);
			//スキンの数を取得 現在1メッシュにつき1スキンしか当てれないためそれ以外が来ると不適格
			for (int i = 0; i < meshCount; i++) {
				//メッシュ保存
				MeshSave(fc, model->GetMesh(i));
			}
			for (int i = 0; i < meshCount; i++) {
				//もしスキンがなかった場合はクラスターもないので書き出し終了
				bool haveSkin = (model->GetMesh(i)->GetSkinCount() > 0);
				//スキンが存在するかどうかのフラグを出力
				fc->Write(&haveSkin, 1);
				if (!haveSkin)continue;
				FL::Mesh* mesh = model->GetMesh(i);
				//クラスター保存
				ClusterSave(fc, mesh);
			}
			//終了
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
			if (!importer)STRICT_THROW("ファイルが読み込まれていない可能性があります");
			std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
			fc->Open(file_path, Utility::FileController::OpenMode::WriteBinary);
			if (!fc->IsOpen())STRICT_THROW("保存用ファイルオープンに失敗");
			FL::Model* model = importer->GetModel();
			//マテリアル数取得
			int materialCount = model->GetMaterialCount();
			if (materialCount <= 0)STRICT_THROW("マテリアルが存在しません");
			//マテリアル数出力
			fc->Write(&materialCount, 1);
			for (int i = 0; i < materialCount; i++) {
				//マテリアル取得
				FL::Material* material = model->GetMaterial(i);
				//マテリアル名サイズ取得
				int materialNameLength = material->GetName().size() + 1;
				//マテリアル名サイズ書き込み
				fc->Write(&materialNameLength, 1);
				//マテリアル名出力
				fc->Write(material->GetName().c_str(), materialNameLength);
				//テクスチャ名のサイズ取得
				int textureNameSize = material->GetTexture(0).size() + 1;
				//サイズ書き込み
				fc->Write(&textureNameSize, 1);
				//テクスチャファイル名出力
				fc->Write(material->GetTexture(0).c_str(), textureNameSize);
			}
			//終了
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
			//初期姿勢行列取得
			Matrix initPoseMatrix = cluster->GetInitPoseMatrix();
			DirectX::XMFLOAT4X4 ipose = {};
			LoadMatrix(&ipose, initPoseMatrix);
			DirectX::XMMATRIX initPose = {};
			//計算用の型に流す
			initPose = DirectX::XMLoadFloat4x4(&ipose);
			DirectX::XMVECTOR arg = {};
			//初期姿勢の逆行列を算出
			DirectX::XMMATRIX inverseInit = DirectX::XMMatrixInverse(&arg, initPose);
			FL::AnimationTake* take = cluster->GetAnimationTake(animation_index);
			//キーフレーム数取得
			int keyFrameCount = take->GetMatrixSize();
			std::vector<DirectX::XMFLOAT4X4> matrix(keyFrameCount);
			//このあたりとかは上の関数から引っ張ってきて使いまわせる
			for (int i = 0; i < keyFrameCount; i++) {
				Matrix ctemp = take->GetCurrentPoseMatrix(i);
				DirectX::XMFLOAT4X4 currentPose = {};
				//行列をDirectXMathの型に入れ込む
				LoadMatrix(&currentPose, ctemp);
				DirectX::XMMATRIX current;
				//計算用の型に移し替え
				current = DirectX::XMLoadFloat4x4(&currentPose);
				//バインドポーズ行列の逆行列と、カレントポーズ行列をかけて事前計算
				DirectX::XMMATRIX frame = inverseInit*current;
				//保存用の型に変換、受け取りも保存用変数
				DirectX::XMStoreFloat4x4(&matrix[i], frame);
			}
			//キーフレーム出力
			fc->Write(matrix.data(), keyFrameCount);
		}
		//もう少し分解してきれいにしよう
		void AnimationExporter::Save(FbxImporter* importer, const char* directroy) {
			if (!importer)STRICT_THROW("ファイルが読み込まれていない可能性があります");
			FL::Model* model = importer->GetModel();
			//メッシュ数取得
			int meshCount = model->GetMeshCount();
			//一応メッシュ数チェック
			if (meshCount <= 0)STRICT_THROW("メッシュがありません。モデルが不適格な可能性があります");
			int animationCount = model->GetMesh(0)->GetSkin(0)->GetCluster(0)->GetAnimationCount();
			//アニメーションの数
			for (int animationIndex = 0; animationIndex < animationCount; animationIndex++) {
				std::shared_ptr<Utility::FileController> fc = std::make_shared<Utility::FileController>();
				std::string takeName = model->GetMesh(0)->GetSkin(0)->GetCluster(0)->GetAnimationTake(animationIndex)->GetTakeName();
				fc->Open((directroy + takeName + ".anm").c_str(), Utility::FileController::OpenMode::WriteBinary);
				if (!fc->IsOpen())STRICT_THROW("保存用ファイルオープンに失敗");
				//アニメーションテイク名の長さ取得
				int nameSize = takeName.size() + 1;
				//アニメーションテイク名の長さ出力
				fc->Write(&nameSize, 1);
				//アニメーションテイク名出力
				fc->Write(takeName.c_str(), nameSize);
				//このライブラリが1秒あたり何フレームサンプルしたのか取得
				int framePerCount = FL::System::GetInstance()->sampleFramePerCount;
				//このライブラリが1秒あたり何フレームサンプルしたのか出力
				fc->Write(&framePerCount, 1);
				FL::AnimationTake* take = model->GetMesh(0)->GetSkin(0)->GetCluster(0)->GetAnimationTake(animationIndex);
				//キーフレーム数取得
				int keyFrameCount = take->GetMatrixSize();
				//キーフレーム数出力
				fc->Write(&keyFrameCount, 1);
				//メッシュ数出力
				fc->Write(&meshCount, 1);
				//メッシュの数
				for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
					if (model->GetMesh(meshIndex)->GetSkinCount() == 0) {
						int clusterCount = 0;
						//クラスター数保存
						fc->Write(&clusterCount, 1);
						continue;
					}
					FL::Skin* skin = model->GetMesh(meshIndex)->GetSkin(0);
					int clusterCount = skin->GetClusterCount();
					//クラスター数保存
					fc->Write(&clusterCount, 1);
					//クラスターの数
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