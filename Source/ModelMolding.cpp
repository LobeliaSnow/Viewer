#include "Tool.h"
//設計ゴミ 主にマテリアル回り
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
		//頂点バッファ作成
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
		//描画
		Graphics::Device::GetContext()->Draw(indexCount, 0);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////
	//  ModelMolding::Material
	//////////////////////////////////////////////////////////////////////////////////////////////
	ModelMolding::Material::Material(const char* name, const char* texture_path) {
		//マテリアル構成
		textureName = texture_path;
		material = std::make_unique<Graphics::Material>(name, texture_path);
		_findfirst(textureName.c_str(), &fileDate);
	}
	ModelMolding::Material::~Material() = default;
	void ModelMolding::Material::Update() {
		_finddata_t now;
		_findfirst(textureName.c_str(), &now);
		if (now.time_access > fileDate.time_access) {
			//アクセス権が取得できるまで待機
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
		//アニメーションの最大値を取得
		int animationMax = (frameCount - 1)*(1000 / framePerCount);
		while (this->time >= animationMax)this->time -= animationMax;
	}
	void ModelMolding::Animation::Update(int mesh_index) {
		//補間等ごちゃごちゃしないといけない
		for (int i = 0; i < clusterCount[mesh_index]; i++) {
			DirectX::XMMATRIX renderTransform = DirectX::XMLoadFloat4x4(&keyFrames[mesh_index][i][static_cast<int>(time / (1000 / framePerCount))]);
			renderTransform = DirectX::XMMatrixTranspose(renderTransform);
			//本当はここで補間
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
			//マテリアル構成
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
				//重要、頂点バッファの作成等を行う。
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
		//デフォルトの頂点シェーダー取得
		Graphics::VertexShader* vs = Graphics::ShaderBank::Get<Graphics::VertexShader>(DEFAULT_VERTEX_SHADER_STATIC_MODEL);
		//リフレクション開始
		std::unique_ptr<Graphics::Reflection> reflector = std::make_unique<Graphics::Reflection>(vs);
		//入力レイアウト作成
		inputLayout = std::make_unique<Graphics::InputLayout>(vs, reflector.get());
		//コンスタントバッファ作成
		constantBuffer = std::make_unique<Graphics::ConstantBuffer<DirectX::XMMATRIX>>(1, Graphics::ShaderStageList::VS);
		//ワールド変換行列(単位行列)
		world = DirectX::XMMatrixIdentity();
		selectMesh = -1;
	}
	void ModelMolding::ConfigureStaticMesh(FL::Mesh* mesh) {
		std::shared_ptr<Mesh> meshTemp = std::make_shared<Mesh>();
		//基本情報取得
		meshTemp->indexCount = mesh->GetIndexCount();
		meshTemp->uvCount = mesh->GetUVCount();
		if (meshTemp->indexCount != meshTemp->uvCount)STRICT_THROW("初めて見るファイルフォーマットです、この例外メッセージを制作者に連絡してください");//とかいいつつ、humanoid.fbxはこっちに来るんじゃないかなと予測
		//頂点用バッファを確保
		meshTemp->vertices.resize(meshTemp->indexCount);
		//頂点情報構築
		for (int i = 0; i < meshTemp->indexCount; i++) {
			int index = mesh->GetIndexBuffer(i);
			memcpy_s(&meshTemp->vertices[i].pos, sizeof(Vector4), &mesh->GetVertex(index), sizeof(Vector3));
			meshTemp->vertices[i].pos.w = 1.0f;
			memcpy_s(&meshTemp->vertices[i].normal, sizeof(Vector4), &mesh->GetNormal(index), sizeof(Vector3));
			memcpy_s(&meshTemp->vertices[i].tex, sizeof(Vector2), &mesh->GetUV(i), sizeof(Vector2));
		}
		//対応マテリアル名取得
		meshTemp->materialName = mesh->GetMaterialName();
		//データを追加
		meshes.push_back(meshTemp);
	}
	void ModelMolding::ConfigureBoneInfo(int mesh_count, FL::Skin* skin) {
		struct BoneInfo {
			int clusterIndex;
			float weight;
		};
		meshes[mesh_count]->clusterCount = skin->GetClusterCount();
		//頂点数分
		std::vector<std::vector<BoneInfo>> boneInfos(meshes[mesh_count]->indexCount);
		//バッファ確保
		meshes[mesh_count]->initPoseMatrices.resize(meshes[mesh_count]->clusterCount);
		meshes[mesh_count]->impactSize.resize(meshes[mesh_count]->indexCount);
		//データパース
		for (int clusterIndex = 0; clusterIndex < meshes[mesh_count]->clusterCount; clusterIndex++) {
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
				meshes[mesh_count]->impactSize[vertexPointIndex]++;
				//追加
				boneInfos[vertexPointIndex].push_back(info);
			}
			//初期姿勢行列取得 いったん放置
			meshes[mesh_count]->initPoseMatrices[clusterIndex] = cluster->GetInitPoseMatrix();
		}
		//構築
		for (int i = 0; i < meshes[mesh_count]->indexCount; i++) {
			for (int j = 0; j < boneInfos[i].size(); j++) {
				meshes[mesh_count]->vertices[i].clusterIndex[j] = boneInfos[i][j].clusterIndex;
				meshes[mesh_count]->vertices[i].weights[j] = boneInfos[i][j].weight;
			}
		}
	}
	void ModelMolding::ConfigureAnimation(FbxImporter* importer) {
		if (!importer)STRICT_THROW("ファイルが読み込まれていない可能性があります");
		FL::Model* model = importer->GetModel();
		if (meshes[0]->clusterCount == 0)return;
		int animationCount = model->GetMesh(0)->GetSkin(0)->GetCluster(0)->GetAnimationCount();
		animations.resize(animationCount);
		//アニメーションの数
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
	//プロトタイプ宣言
	void LoadMatrix(DirectX::XMFLOAT4X4* mat0, const Matrix& mat1);
	void ModelMolding::ConfigureKeyFrames(FL::Cluster* cluster, int mesh_index, int cluster_index, int animation_index, std::shared_ptr<Animation>& animation) {
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
		animation->keyFrames[mesh_index][cluster_index].resize(animation->frameCount);
		for (int i = 0; i < animation->frameCount; i++) {
			Matrix ctemp = take->GetCurrentPoseMatrix(i);
			DirectX::XMFLOAT4X4 currentPose = {};
			//行列をDirectXMathの型に入れ込む
			LoadMatrix(&currentPose, ctemp);
			DirectX::XMMATRIX current;
			//計算用の型に移し替え
			current = DirectX::XMLoadFloat4x4(&currentPose);
			//バインドポーズ行列の逆行列と、カレントポーズ行列をかけて事前計算
			DirectX::XMMATRIX frame = inverseInit * current;
			//保存用の型に変換、受け取りも保存用変数
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
			//メッシュが選択されているとき
			if (index == selectMesh) world = DirectX::XMMatrixScaling(0.4f, 0.4f, 0.4f);
			else world = DirectX::XMMatrixScaling(0.3f, 0.3f, 0.3f);
			constantBuffer->Activate(DirectX::XMMatrixTranspose(world));
			meshes[index]->Render(this);
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
}