#pragma once
namespace Lobelia::Graphics {
	struct BoneInfo {
		int clusterIndex;
		float weight;
	};
	//vectorが続くところはtypedefなりusingなりしたらよい。
	class Animation {
		struct Constant {
			DirectX::XMFLOAT4X4 keyFrame[256] = {};
		};
	private:
		char animationName[32] = {};
		int framePerCount = -1;
		int frameCount = -1;
		int clusterCount = -1;
		int meshCount = -1;
		//メッシュ->クラスター->フレーム
		std::vector<std::vector<std::vector<DirectX::XMFLOAT4X4>>> keyFrames;
		Constant buffer;
		std::unique_ptr<ConstantBuffer<Constant>> constantBuffer;
		float time;
	public:
		Animation(const char* file_path);
		~Animation();
		void AddElapsedTime(float time);
		void Activate(int meshIndex);
	};
	class Model {
	public:
		struct Vertex {
			Math::Vector4 pos;
			Math::Vector4 normal;
			Math::Vector2 tex;
			UINT clusteIndex[4];
			Math::Vector4 weights;
		};
		struct ReadVertex {
			Math::Vector4 pos;
			Math::Vector4 normal;
			Math::Vector2 tex;
		};
		struct Desc {
			size_t vertex_count;
			ReadVertex* vertex;
			size_t delimiter_count;
			int* delimiters;
			std::string* vertex_material_name;
			size_t material_count;
			std::string* names;
			std::string* texture_names;
			Math::Vector3* diffuses;
			Math::Vector3* ambients;
			Math::Vector3* speculars;
			std::vector<int>* clusterCounts;
			std::vector<int>* indexCounts;
			//メッシュ数->頂点数->影響ボーン数
			std::vector<std::vector<std::vector<BoneInfo>>>* boneInfos;
			//メッシュ数->クラスター数
			std::vector<std::vector<DirectX::XMFLOAT4X4>>* initPoseMatrices;
			//メッシュ数->頂点数
			std::vector<std::vector<int>>* boneImpactSize;
		};
	private:
		std::unique_ptr<Mesh<Vertex>> mesh;
		std::map<std::string, std::shared_ptr<Material>> materials;
		std::vector<Material*> renderIndexMaterial;
		std::unique_ptr<InputLayout> inputLayout;
		std::unique_ptr<ConstantBuffer<DirectX::XMMATRIX>> constantBuffer;
		DirectX::XMMATRIX world;
		std::vector<std::unique_ptr<Animation>> animations;
		//transform構造体にしてもよい
		Math::Vector3 pos;
		Math::Vector3 scale;
		union {
			//後に使う予定
			DirectX::XMVECTOR quaternion;
			Math::Vector3 euler;
		};
		int animationIndex;
	private:
		Model();
	private:
		void Construction(const Desc& desc);
	public:
		Model(const Desc& desc);
		Model(const char* file_path);
		~Model();
		void LoadAnimation(const char* animation_path);
		void AnimationUpdate(float time);
		void ActivateAnimation(int index);
		std::map<std::string, std::shared_ptr<Material>>& GetMaterials();
		Material* GetMaterial(const std::string& material_name);
		//現在未実装 Raypic実装後の予定
		std::string GetMaterialName(int polygon_index);
		void SetPos(const Math::Vector3& pos);
		void SetScale(const Math::Vector3& scale);
		void SetEuler(const Math::Vector3& euler);
		void Update();
		void Render(bool set_default_pipeline = true);
	};
}