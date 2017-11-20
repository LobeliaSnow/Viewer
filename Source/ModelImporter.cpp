#include "Lobelia.hpp"
#include "FbxLoader.h"
#include "Header/ModelImporter.hpp"

namespace Lobelia {
	FbxImporter::FbxImporter() {}
	FbxImporter::~FbxImporter() {	}
	void FbxImporter::Load(const char* file_path) {
		model = std::make_shared<FL::Model>(file_path);
	}
	FL::Model* FbxImporter::GetModel() { return model.get(); }
	
}