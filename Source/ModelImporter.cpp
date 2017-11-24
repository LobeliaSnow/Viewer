#include "Lobelia.hpp"
#include "FbxLoader.h"
#include "Header/ModelImporter.hpp"

namespace Lobelia {
	FbxImporter::FbxImporter() = default;
	FbxImporter::~FbxImporter() = default;
	const std::string FbxImporter::GetPath() { return path; }
	const std::string FbxImporter::GetSaveAppName() { return saveAppName; }
	void FbxImporter::Load(const char* file_path) {
		path = file_path;
		model = std::make_shared<FL::Model>(file_path);
		fbxsdk::FbxDocumentInfo* info = model->scene->GetSceneInfo();
		fbxsdk::FbxString appName = info->LastSaved_ApplicationName.Get();
		saveAppName = appName.Buffer();
	}
	FL::Model* FbxImporter::GetModel() { return model.get(); }
	bool FbxImporter::IsEmpty() { return !model.get(); }
}