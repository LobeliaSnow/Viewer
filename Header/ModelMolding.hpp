#pragma once

namespace Lobelia {
	class MeshWidget;
	class MaterialWidget;
	//�����ŕ`��⃂�f���̐��`��S��
	//�Ƃ肠�����X�^�e�B�b�N���b�V��
	//��X�X�e�[�W�f�[�^�ƃL�����N�^�[�f�[�^���Ă��������œǂݍ��߂�悤�ɂ���
	//�X�e�[�W�z�u���L�����N�^�[�ɃR���g���[���[���������ē�������悤�ɂ���
	class ModelMolding {
	public:
		//���_�o�b�t�@�����b�V�����Ƃ�
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
			//���_��
			std::vector<int> impactSize;
		public:
			void CreateDataToDirectX();
			void Render(ModelMolding* model);
		};
		//TODO : �e�N�X�`�����I����ւ��̏o�͍\�z�B
		//TODO : ��MaterialWidget�Ɠ����������ق����y�����B���̕����Ō���
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
				//�v��������XMMATRIX�ł悭�ˁH
				DirectX::XMFLOAT4X4 keyFrame[KEY_FRAME::MAX] = {};
			};
			//���b�V��->�N���X�^�[->�t���[��
			using KeyFrames = std::vector<std::vector<std::vector<DirectX::XMFLOAT4X4>>>;
		public:
			std::string name;
			int framePerCount;
			int frameCount;
			int meshCount;
			//���b�V����
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
			//�A�j���[�V�������擾
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