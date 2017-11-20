#pragma once
namespace Lobelia {
	//�c�[�����ŏ��o�͂������ꍇ�͂�������擾
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