#pragma once

namespace Lobelia {
	class MeshWidget;
	class MaterialWidget;
	//ここで描画やモデルの成形を担う
	//とりあえずスタティックメッシュ
	//後々ステージデータとキャラクターデータっていう感じで読み込めるようにして
	//ステージ配置＆キャラクターにコントローラーをくっつけて動かせるようにする
	class ModelMolding {
	public:
		//頂点バッファをメッシュごとに
		class Mesh {
		public:
			struct Vertex {
				Vector4 pos;
				Vector4 normal;
				Vector2 tex;
				UINT clusterIndex[4];
				float weights[4];
			};
		private:
			std::unique_ptr<Graphics::Mesh<Vertex>> mesh;
		public:
			int indexCount;
			int uvCount;
			std::vector<Vertex> vertices;
			std::string materialName;
			int clusterCount;
			std::vector<Matrix> initPoseMatrices;
			//頂点数
			std::vector<int> impactSize;
		public:
			void CreateDataToDirectX();
			void Render(ModelMolding* model);
		};
		//TODO : テクスチャ動的入れ替えの出力構想。
		//TODO : ↑MaterialWidgetと同化させたほうが楽かも。その方向で検討
		class Material {
		public:
			std::string textureName;
			std::shared_ptr<Graphics::Material> material;
			std::shared_ptr<Graphics::Texture> nowTexture;
			_finddata_t fileDate;
		public:
			Material(const char* name, const char* texture_path);
			~Material();
			void Update();
		};
		class Animation {
		private:
			struct Constant {
				enum KEY_FRAME { MAX = 256 };
				//思ったけどXMMATRIXでよくね？
				DirectX::XMFLOAT4X4 keyFrame[KEY_FRAME::MAX] = {};
			};
			//メッシュ->クラスター->フレーム
			using KeyFrames = std::vector<std::vector<std::vector<DirectX::XMFLOAT4X4>>>;
		public:
			std::string name;
			int framePerCount;
			int frameCount;
			int meshCount;
			//メッシュ数
			std::vector<int> clusterCount;
			KeyFrames keyFrames;
			Constant buffer;
			std::unique_ptr<Graphics::ConstantBuffer<Constant>> constantBuffer;
			float time;
		public:
			Animation();
			~Animation();
			void AddElapsedTime(float time);
			void Update(int mesh_index);
			//アニメーション名取得
			const std::string& GetName();
		};
	public:
		int meshCount;
		std::vector<int> renderMeshIndex;
		std::vector<std::shared_ptr<Mesh>> meshes;
		std::unique_ptr<Graphics::InputLayout> inputLayout;
		std::unique_ptr<Graphics::ConstantBuffer<DirectX::XMMATRIX>> constantBuffer;
		DirectX::XMMATRIX world;
		int materialCount;
		std::map<std::string, std::unique_ptr<Material>> materials;
		int skinCount;
		int selectMesh;
		std::vector<std::shared_ptr<MeshWidget>> widgets;
		std::vector<std::shared_ptr<MaterialWidget>> mwidgets;
		std::vector<std::shared_ptr<Animation>> animations;
		Lobelia::Timer timer;
	private:
		void CreateDirectXState();
		void ConfigureStaticMesh(FL::Mesh* mesh);
		void ConfigureBoneInfo(int mesh_count, FL::Skin* skin);
		void ConfigureAnimation(FbxImporter* importer);
		void ConfigureKeyFrames(FL::Cluster* cluster, int mesh_index, int cluster_index, int animation_index, std::shared_ptr<Animation>& animation);
	public:
		ModelMolding(FbxImporter* importer, Tool* tool);
		~ModelMolding();
		MeshWidget* GetMeshWidget(int index);
		MaterialWidget* GetMaterialWidget(int index);
		void Update(Tool* tool);
		void Render(Tool* tool);
	};
}