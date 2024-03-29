#pragma once
#include "AnimatedSprite.h"

#include <iostream>
#include <string>

#include <SDL2/SDL.h>
#include "Vector.h"
#include <memory>
#include "Size.h"
#include "LevelPiece.h"
#include "Audio.h"

class Character :
	public AnimatedSprite
{
public:
	Character(SDL_Renderer* ren, std::string imagePath, std::string JSONPath, Vector velocity1 = Vector(), Vector location = Vector(100, 100), Size size1 = Size(100, 100));
	~Character();

	void Jump();
	void MoveLeft();
	void MoveRight();
	void MoveUp();
	void MoveDown();

	void StopMovingLeft();
	void StopMovingRight();
	void StopMovingUp();
	void StopMovingDown();
	
	void StopAllMovement();

	void ResetPosition();

	void Update(float time, std::vector<std::unique_ptr<LevelPiece>> &level, Size windowSize, Vector(*checkLadder)(Vector,bool));


	void setOnGround();

	enum MoveState {LEFT = -1, IDLE = 0, RIGHT = 1, UP = -1, DOWN = 1};

	int UP_KEY;
	int DOWN_KEY;
	int LEFT_KEY;
	int RIGHT_KEY;
	int JUMP_KEY;

	MoveState _state = IDLE;
protected:

	float MOVE_SPEED = 200;

	bool _isAI = false;

private:
	bool _isOnGround = false;
	bool _isOnLadder = false;
	bool _isJumping = false;
	bool _isFalling = false;
	MoveState _ladderState = IDLE;
	MoveState _afterState = IDLE;
	float _yVelocity = 0;
	float _xVelocity = 0;

	bool _horizontalButtonDown = false;

	Vector _originalLocation;

	bool _walkingPlaying = false;
	int _walkingChannel = 0;
	bool _ladderClimbPlaying = false;
	int _ladderClimbChannel = 0;

	bool _ladderAllowedRight = false;
	bool _ladderAllowedLeft = false;

	void UpdateCollisions(std::vector<std::unique_ptr<LevelPiece>> &level, Size windowSize);

	void changeState(Character::MoveState newState, bool flip = true);
	void changeLadderState(Character::MoveState newState);
};

