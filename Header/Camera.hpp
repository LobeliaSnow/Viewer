#pragma once
namespace Lobelia {
	class ModelMolding;
	struct Mouse {
		Math::Vector2 move;
		int xWheel;
		int yWheel;
		bool left;
		bool right;
		bool center;
	};
	//ビュアーで使うもののみ
	struct Keyboard {
		bool ctrl;
		bool alt;
	};
	class Camera {
	private:
		std::unique_ptr<Lobelia::Graphics::View> view;
		Math::Vector3 pos;
		Math::Vector3 target;
		Math::Vector3 up;
		Math::Vector2 move;
		Math::Vector3 axis;
		float radian;
		float radius;
		float radianY;
	private:
		void MoveCursor();
		//カメラ座標系でのZ移動
		void MoveZ(const Lobelia::Mouse& mouse);
		void TargetPosMove(const Lobelia::Mouse& mouse, const Lobelia::Keyboard& keyboard);
		void CylinderMove(const Lobelia::Mouse& mouse);
		void CalcAndSetUpDirection();
	public:
		Camera(const Math::Vector2& view_scale);
		~Camera();
		void Update(const Lobelia::Mouse& mouse, const Lobelia::Keyboard& keyboard);
	};
}