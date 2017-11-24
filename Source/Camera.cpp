#include "Lobelia/Lobelia.hpp"
#include "Header/Camera.hpp"

namespace Lobelia {
	Camera::Camera(const Math::Vector2& view_scale) :view(std::make_unique<Lobelia::Graphics::View>(Lobelia::Math::Vector2(0, 0), view_scale)) {
		pos = Math::Vector3(0, 30, 30);
		target = Math::Vector3(0, 15, 0);
		axis = target - pos;
		axis.Normalize();
		radius = 30;
		radianY = radian = 0.0f;
	};
	Camera::~Camera() = default;
	void Camera::MoveCursor() {
		static POINT old = {};
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		move = Math::Vector2(cursorPos.x - old.x, cursorPos.y - old.y);
		old = cursorPos;
	}
	void Camera::MoveZ(const Lobelia::Mouse& mouse) {
		//Math::Vector3 direction = target - pos;
		//direction.Normalize();
		//if (mouse.yWheel < 0)pos -= direction;
		//else if (mouse.yWheel > 0)pos += direction;
		radius += mouse.yWheel*0.01f;
	}
	void Camera::TargetPosMove(const Lobelia::Mouse& mouse) {
		if (mouse.center) {
			Math::Vector3 direction = target - pos;
			//Œ»Ý‚Ì’·‚³ŽZo
			float length = direction.Length();
			direction.Normalize();
			//‰E•ûŒüŽZo
			Math::Vector3 right = Math::Vector3::Cross(Math::Vector3(0, 1, 0), direction);
			right.Normalize();
			target += right*move.x*0.1f;
			Math::Vector3 up = Math::Vector3::Cross(direction, right);
			target += up*move.y*0.1f;
		}
	}
	void Camera::SphereMove(const Lobelia::Mouse& mouse) {
		if (mouse.left) {
			radian += move.x*0.01f;
			radianY += move.y*0.01f;
		}
		while (radianY > PI) { radianY = PI; }
		while (radianY < 0) { radianY = 0; }
		pos.x = sinf(radian)*radius;
		pos.y = cosf(radianY)*radius;
		pos.z = cosf(radian)*radius;
		pos += target;
		//Math::Vector3 direction = target - pos;
		////Œ»Ý‚Ì’·‚³ŽZo
		//float length = direction.Length();
		//direction.Normalize();
		////‰E•ûŒüŽZo
		//Math::Vector3 right = Math::Vector3::Cross(up, direction);
		//right.Normalize();
		////ˆÚ“®
		//pos -= right*move.x * 0.1f;
		//pos += up*move.y * 0.1f;
		////‹——£•â³
		//direction = pos - target;
		//direction.Normalize();
		////ˆÊ’uŒˆ’è
		//pos = target + direction*length;
	}
	void Camera::CalcAndSetUpDirection() {
		Math::Vector3 direction = target - pos;
		direction.Normalize();
		Math::Vector3 base(1, 0, 0);
		base.Normalize();
		up = Math::Vector3::Cross(base, direction);
		up.Normalize();
		view->SetEyeUpDirection(Math::Vector3(0, 1, 0));
	}
	void Camera::Update(const Lobelia::Mouse& mouse, const Lobelia::Keyboard& keyboard) {
		MoveCursor();
		MoveZ(mouse);
		SphereMove(mouse);
		CalcAndSetUpDirection();

		view->SetEyePos(pos);
		view->SetEyeTarget(target);
		view->Activate();
	}
}