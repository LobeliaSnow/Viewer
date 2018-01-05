#pragma once
#include <mutex>

namespace Lobelia {
	//ƒc[ƒ‹‘¤‚Åî•ño—Í‚µ‚½‚¢ê‡‚Í‚±‚±‚©‚çæ“¾
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