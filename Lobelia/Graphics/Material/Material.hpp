#pragma once
namespace Lobelia::Graphics {
	//イメージとしては、メッシュに対してマテリアルを貼るというイメージ。
	//現状はテクスチャを持っているだけのクラスだが、情報は増える予定。
	//スペキュラ等
	//テクスチャのカラー補正用の定数バッファも持つ予定(Spriteもその際それに移行)
	class Material :private Origin<Material> {
		struct Data {
			Math::Vector4 diffuse = {};//現状は使わない
			Math::Vector4 ambient = {};//現状は使わない
			Math::Vector4 specular = {};//現状は使わない
			Math::Vector4 texColor = {};
		};
	private:
		std::string name;
		std::shared_ptr<Texture> texture;
		Data data;
		bool visible;
		std::unique_ptr<ConstantBuffer<Data>> constantBuffer;
	public:
		Material(const char* name, const char* texture_path);
		~Material();
		const std::string& GetName();
		Texture* GetTexture();
		void SetDiffuseColor(const Math::Vector4& diffuse);
		void SetAmbientColor(const Math::Vector4& ambient);
		void SetSpecularColor(const Math::Vector4& specular);
		void SetTexColor(Utility::Color tex_color);
		void ChangeVisible(bool visible);
		bool IsVisible();
		//いつでもテクスチャを切り替えれるように。
		void ChangeTexture(std::shared_ptr<Texture>& texture);
		void Set(bool texture = true, bool color = true);
	};
}