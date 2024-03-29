#pragma once

#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <chrono>

#include "Sprite.h"
#include "json.hpp"

class AnimatedSprite :
	public Sprite
{
public:
	AnimatedSprite(SDL_Renderer* ren, std::string imagePath, Vector velocity1 = Vector(), Vector location = Vector(100, 100), Size size1 = Size(100, 100));
	AnimatedSprite(SDL_Renderer* ren, std::string imagePath, std::string JSONPath, Vector velocity1 = Vector(), Vector location = Vector(100, 100), Size size1 = Size(100, 100));

	~AnimatedSprite();

	void setVelocity(Vector newVelocity) { _velocity = newVelocity; };
	void setVelocityX(float XVelocity) { _velocity = Vector(XVelocity, _velocity.y); };
	void setVelocityY(float YVelocity) { _velocity = Vector(_velocity.x, YVelocity); };
	Vector getVelocity() { return _velocity; };

	void Update(float time);

	void render(SDL_Renderer* ren);

	bool changeAnimationCycle(std::string newAnimationCycle);
protected:


	void flipAnimation(bool right);

private: 
	bool importFromJSON(std::string path);

	int countChildren(nlohmann::basic_json<> element);
	
	nlohmann::json j;
	int _currentFrame = 0;
	int _maxFrames = 0;

	std::chrono::system_clock::time_point last_animation_step;

	int _animation_speed = 0; // in nano seconds

	std::string _animationCycle = "IDLE"; // for storing what cycle the animation is at (e.g. walk, idle, jump, etc.)

	Vector _velocity;

	SDL_RendererFlip flip = SDL_FLIP_NONE;
};

