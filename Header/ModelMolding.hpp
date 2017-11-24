#pragma once
namespace Lobelia {
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
		public:
			void CreateDataToDirectX();
			void Render(ModelMolding* model);
		};
		class Material {
		public:
			std::string textureName;
			std::unique_ptr<Graphics::Material> material;
		public:
			Material(const char* name, const char* texture_path);
			~Material();
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
	private:
		void CreateDirectXState();
		void ConfigureStaticMesh(FL::Mesh* mesh);

	public:
		ModelMolding(FbxImporter* importer, Tool* tool);
		~ModelMolding();
		void Render(Tool* tool);
	};

}