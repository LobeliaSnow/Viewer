#pragma once
namespace Lobelia {
	//ƒc[ƒ‹‘¤‚Åî•ño—Í‚µ‚½‚¢ê‡‚Í‚±‚±‚©‚çæ“¾
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