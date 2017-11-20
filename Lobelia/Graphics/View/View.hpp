#pragma once
namespace Lobelia::Graphics {
	class View {
	private:
		struct Constant {
			DirectX::XMMATRIX view;
			DirectX::XMMATRIX projection;
			//Math::Vector4 pos;
		};
		struct Data {
			DirectX::XMVECTOR eye;
			DirectX::XMVECTOR at;
			DirectX::XMVECTOR up;
		};
	private:
		static DirectX::XMMATRIX nowView;
		static DirectX::XMMATRIX nowProjection;
	public:
		//privateÇ…ÇµÇƒÉQÉbÉ^Å[çÏÇÈ
		static Math::Vector2 nowSize;
	private:
		D3D11_VIEWPORT viewport = {};
		std::unique_ptr<ConstantBuffer<Constant>> constantBuffer;
		Constant buffer;
		Data data;
		Math::Vector2 size;
	private:
		void CreateProjection(float fov_rad, float aspect, float near_z, float far_z);
		void CreateView(const Data& data);
		void CreateViewport(const Math::Vector2& pos, const Math::Vector2& size);
	public:
		View(const Math::Vector2& left_up, const Math::Vector2& size, float fov_rad = PI / 4.0f, float near_z = 1.0f, float far_z = 1000.0f);
		virtual ~View();
		void SetEyePos(const Math::Vector3& pos);
		void SetEyeTarget(const Math::Vector3& target);
		void SetEyeUpDirection(const Math::Vector3& up_direction);
		void ChangeViewport(const Math::Vector2& pos, const Math::Vector2& size);
		void Activate();
		DirectX::XMMATRIX GetColumnViewMatrix();
		DirectX::XMMATRIX GetColumnProjectionMatrix();
		DirectX::XMMATRIX GetRawViewMatrix();
		DirectX::XMMATRIX GetRawProjectionMatrix();
		static DirectX::XMMATRIX GetNowColumnViewMatrix();
		static DirectX::XMMATRIX GetNowColumnProjectionMatrix();
		static DirectX::XMMATRIX GetNowRawViewMatrix();
		static DirectX::XMMATRIX GetNowRawProjectionMatrix();

	};
}
