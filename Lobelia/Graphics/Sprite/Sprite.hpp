#pragma once
namespace Lobelia::Graphics {
	using Microsoft::WRL::ComPtr;
	class Sprite {
	private:
		struct Vertex {
			Math::Vector4 pos;	//x,y,z
			Math::Vector4 col;
			Math::Vector2 tex;	//u,v(x,y)
		};
	private:
		std::unique_ptr<Mesh<Vertex>> mesh;
		std::shared_ptr<Material> material;
		std::unique_ptr<InputLayout> inputLayout;
		//こいつが実際のスクリーンスペースの座標を所持している
		Math::Vector2 vertex[4] = { {-1,-1},{-1,0},{0,-1},{0,0} };

	private:
		Math::Vector4 Trans2DPosition(Math::Vector2 pos);
		void PositionPlant(const Transform2D& transform);
		void PositionRotation(const Transform2D& transform);
		void SetMeshColor(Utility::Color color);
		void CutUV(const Math::Vector2& uv_pos, const Math::Vector2& uv_size);
		void MeshTransform();
	public:
		Sprite(const char* file_path);
		~Sprite();
		Material* GetMaterial();
		void ChangeMaterial(std::shared_ptr<Material>& material);
		const Math::Vector2* GetSquareSSPos();
		void Render(const Transform2D& transform, const Math::Vector2& uv_pos, const Math::Vector2& uv_size, Utility::Color color, bool set_default_pipeline = true);
	};
	class SpriteBatch {
	private:
		struct Vertex {
			Math::Vector4 pos;	//x,y,z
			Math::Vector2 tex;	//u,v(x,y)
		};
		struct Instance {
			DirectX::XMMATRIX ndc;
			Math::Vector4 col;	//r,g,b,a(x,y,z,w)
			Math::Vector4 uvTrans;
		};
	private:
		const int RENDER_LIMIT;
	private:
		std::unique_ptr<Mesh<Vertex>> mesh;
		std::unique_ptr<Material> material;
		std::unique_ptr<InputLayout> inputLayout;
		ComPtr<ID3D11Buffer> instanceBuffer;
		Instance* instances;
		int renderCount;
	public:
		SpriteBatch(const char* file_path, int render_limit);
		~SpriteBatch();
		Material* GetMaterial();
		void BeginRender();
		void Render(const Transform2D& transform, const Math::Vector2& upos, const Math::Vector2& usize, float rotate_axis_x, float rotate_axis_y, Utility::Color color);
		void EndRender(bool set_default_pipeline = true);
	};
}