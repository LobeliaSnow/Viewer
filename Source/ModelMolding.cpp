#include "Tool.h"

namespace Lobelia {
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
		textureName = Utility::FilePathControl::GetFilename(texture_path);
		material = std::make_unique<Graphics::Material>(name, texture_path);
	}
	ModelMolding::Material::~Material() = default;
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////
	//  ModelMolding
	//////////////////////////////////////////////////////////////////////////////////////////////
	ModelMolding::ModelMolding(FbxImporter* importer, Tool* tool) {
		tool->GetUI().label_2->setText(importer->GetSaveAppName().c_str());
		FL::Model* model = importer->GetModel();
		meshCount = model->GetMeshCount();
		meshes.reserve(meshCount);
		renderMeshIndex.resize(meshCount);
		for (int i = 0; i < meshCount; i++) {
			ConfigureStaticMesh(model->GetMesh(i));
			tool->GetUI().listWidget->addItem(QString("Mesh ") + QString(std::to_string(i).c_str()));
		}
		materialCount = model->GetMaterialCount();
		std::string directory = Utility::FilePathControl::GetParentDirectory(importer->GetPath());
		if (!directory.empty())directory += "/";
		//マテリアル構成
		for (int i = 0; i < materialCount; i++) {
			FL::Material* material = model->GetMaterial(i);
			materials[material->GetName()] = std::make_unique<Material>(material->GetName().c_str(), (directory + material->GetTexture(0)).c_str());
			materials[material->GetName()]->material->SetTexColor(0xFFFFFFFF);
		}
		CreateDirectXState();
	}

	ModelMolding::~ModelMolding() = default;
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
		//頂点バッファ等作成
		meshTemp->CreateDataToDirectX();
		//データを追加
		meshes.push_back(meshTemp);
	}
	void ModelMolding::Render(Tool* tool) {
		inputLayout->Set();
		world = DirectX::XMMatrixScaling(0.3f, 0.3f, 0.3f);
		constantBuffer->Activate(DirectX::XMMatrixTranspose(world));
		Graphics::Device::GetContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		auto pipeline = Graphics::PipelineManager::PipelineGet(DEFAULT_PIPELINE_STATIC_MODEL);
		pipeline->Activate(true);
		//doubleClicked
		bool selected = false;
		for (int i = 0; i < meshCount; i++) {
			auto item = tool->GetUI().listWidget->item(i);
			auto text = item->text();
			if (tool->GetUI().listWidget->isItemSelected(item)) {
				selectMesh = i;
				selected = true;
			}
			char* p = strtok(text.toStdString().data(), " ");
			p = strtok(nullptr, " ");
			renderMeshIndex[i] = std::stoi(p);
		}
		if (!selected)selectMesh = -1;
		for (int i = 0; i < meshCount; i++) {
			int index = renderMeshIndex[i];
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