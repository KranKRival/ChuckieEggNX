#pragma once

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Vector.h"
#include "Size.h"

#include "Drawable.h"

class Sprite : public Drawable
{
public:
	Sprite(SDL_Renderer* ren, const std::string imagePath, const Vector location = Vector(100,100), const Size size1 = Size(100,100));
	~Sprite();
	bool isColliding(Sprite & other);

	enum collisionDirection { LEFT, RIGHT, UP, DOWN, NO_COLLISION };

	collisionDirection checkCollisionDirection(Sprite& other);

	Sprite(const Sprite& sprite);
};

