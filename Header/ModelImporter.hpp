#pragma once
#include <mutex>

namespace Lobelia {
	//ツール側で情報出力したい場合はここから取得
	class FbxImporter {
	private:
		std::string path;
		std::string saveAppName;
		std::shared_ptr<FL::Model> model;
	public:
		FbxImporter();
		~FbxImporter();
		const std::string GetPath();
		const std::string GetSaveAppName();
		void Load(const char* file_path);
		FL::Model* GetModel();
		bool IsEmpty();
	};

}