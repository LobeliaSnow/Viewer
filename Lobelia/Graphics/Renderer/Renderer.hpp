#pragma once
namespace Lobelia::Graphics {
	class Renderere {
	private:
		std::vector<std::pair<Sprite*, Transform2D>> sprites;
	public:
		void RegistSprite(Sprite* sprite, const Transform2D& transform);
	};
}