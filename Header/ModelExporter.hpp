#pragma once
namespace Lobelia {
	//TODO : 整形部分と保存部分を切り分けてビュアーで色々できるようにする。
	class DxdExporter {
	private:
		struct WriteVertex {
			Vector4 pos;
			Vector4 normal;
			Vector2 tex;
		};
		struct BoneInfo {
			int clusterIndex;
			float weight;
		};
	private:
		void MeshSave(std::weak_ptr<Utility::FileController> file, FL::Mesh* mesh);
		void ClusterSave(std::weak_ptr<Utility::FileController> file, FL::Mesh* mesh);
	public:
		DxdExporter();
		~DxdExporter();
		void Save(FbxImporter* importer, const char* file_path);
	};
	class MaterialExporter {
	public:
		MaterialExporter();
		~MaterialExporter();
		void Save(FbxImporter* importer, const char* file_path);
	};
	class AnimationExporter {
	private:
		void KeyFrameSave(std::weak_ptr<Utility::FileController> file, FL::Cluster* cluster, int animation_index);
	public:
		AnimationExporter();
		~AnimationExporter();
		void Save(FbxImporter* importer, const char* directroy);
	};
}