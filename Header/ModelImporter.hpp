#pragma once
namespace Lobelia {
	//ツール側で情報出力したい場合はここから取得
	class FbxImporter {
	private:
		std::shared_ptr<FL::Model> model;
	public:
		FbxImporter();
		~FbxImporter();
		void Load(const char* file_path);
		FL::Model* GetModel();
	};

}