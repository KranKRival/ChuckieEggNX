#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <map>

#include <random>
#include <time.h>
#include <chrono>
#include <math.h>
#include <fstream>

#ifdef _WIN32 // compiling on windows
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#else // NOT compiling on windows
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#endif

#if defined(__WIIU__)
#include <romfs-wiiu.h>
#include <unistd.h>
#endif


// custom classes
#include "Text.h"
#include "AnimatedSprite.h"
#include "Size.h"
#include "Vector.h"
#include "Audio.h"
#include "Character.h"
#include "LevelPiece.h"
#include "TileMap.h"
#include "Menu.h"
#include "AI.h"

std::string exeName;
SDL_Window *win; //pointer to the SDL_Window
SDL_Renderer *ren; //pointer to the SDL_Renderer

std::unique_ptr<TileMap> levelMap;

bool done = false;

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::system_clock::time_point TimePoint;

std::vector<std::unique_ptr<AI>> enemies;

std::vector<std::unique_ptr<Character>> players;

Size windowSize = Size(1300, 594);
Vector windowPosition = Vector(50, 50);

int highScore = 0;

int minMaxVolume[2] = { 0, 128 };
int currentVolume = 128;

bool _isPaused = false;
bool _pauseDisabled = false;
bool _windowed = true;

int musicChannel = 0;

int lives = 5;

void renderLoadingScreen();
void addScore(LevelPiece::TileType);
void restartLevel();

std::vector<std::string> levelStrings = { "assets/level1.txt", "assets/level2.txt", "assets/level3.txt" };

Menu* menu;

int currentEggs = 0;
long unsigned int currentLevel = 0;

// TEMP
TimePoint prevTime;
std::map<std::string, std::unique_ptr<Text>> texts;
int score = 0;
std::unique_ptr<Text> scoreTxt;
std::unique_ptr<Text> livesText;
// END TEMP

void addEnemy(Vector position, Vector tilePosition)
{
	enemies.push_back(std::unique_ptr<AI>(new AI(ren, "assets/enemy.png", "assets/enemy.json", levelMap.get(), Vector(), position, Size(40, 60), tilePosition)));
}

void loadKeyMaps()
{
	//std::ifstream file;
	//std::string line;
	//file.open("assets/keys1.txt");
	//if (!file.is_open())
		//return;
	//int i = 0;
	//int keys[5];
	//while (std::getline(file, line))
	//{
		//keys[i] = std::stoi(line);
		//i++;
	//}

	//players[0]->UP_KEY = 13;
	//players[0]->DOWN_KEY = 15;
	//players[0]->LEFT_KEY = 12;
	//players[0]->RIGHT_KEY = 14;
	//players[0]->JUMP_KEY = 0;

	//file.close();

	//file.open("assets/keys2.txt");
	//if (!file.is_open())
		//return;
	
	//i = 0;
	//while (std::getline(file, line))
	//{
		//keys[i] = std::stoi(line);
		//i++;
	//}

	//file.close();

	//players[1]->UP_KEY = keys[0];
	//players[1]->DOWN_KEY = keys[1];
	//players[1]->LEFT_KEY = keys[2];
	//players[1]->RIGHT_KEY = keys[3];
	//players[1]->JUMP_KEY = keys[4];

}

void HandleGameOverMenu(const Text* txt)
{
	std::string textString = txt->GetText();

	if (textString == "EXIT")
		done = true;
	else if (textString == "RESTART")
		restartLevel();
}

void readHighscore()
{
	std::ifstream file;
	std::string line;
	file.open("assets/highscore.txt");
	if (!file.is_open())
		return;
	while (std::getline(file, line))
	{
		highScore = std::stoi(line);
	}

	file.close();
}

void afterMenu(std::string text)
{
	_pauseDisabled = true;
	_isPaused = true;


	menu = new Menu();
	menu->addText(new Text(ren, "assets/Hack-Regular.ttf", text, { 255,0,0 }, Size(250, 100), Vector(windowSize.width / 2, windowSize.height / 2 - 200), 150), HandleGameOverMenu); // create pause text in middle of screen
	if (score > highScore)
	{
		std::string highScoreText = "New Highscore: " + std::to_string(score);
		menu->addText(new Text(ren, "assets/Hack-Regular.ttf", highScoreText, { 0, 0, 255 }, Size(400, 100), Vector(windowSize.width / 2, windowSize.height / 2 - 100), 150), HandleGameOverMenu); // create pause text in middle of screen
		std::ofstream myfile;
		myfile.open("assets/highscore.txt");
		myfile << score;
		myfile.close();
		highScore = score;
	}
	else
	{
		std::string highScoreText = "Highscore: " + std::to_string(highScore);
		menu->addText(new Text(ren, "assets/Hack-Regular.ttf", highScoreText, { 255, 0, 0 }, Size(400, 100), Vector(windowSize.width / 2, windowSize.height / 2 - 100), 150), HandleGameOverMenu); // create pause text in middle of screen
	}

	menu->addText(new Text(ren, "assets/Hack-Regular.ttf", "( R ) To Restart", { 255,255,255 }, Size(550, 100), Vector(windowSize.width / 2, windowSize.height / 2), 150), HandleGameOverMenu); // create pause text in middle of screen
	menu->addText(new Text(ren, "assets/Hack-Regular.ttf", "( - ) To Exit", { 255,255,255 }, Size(550, 100), Vector(windowSize.width / 2, windowSize.height / 2 + 100), 150), HandleGameOverMenu); // create pause text in middle of screen
	Audio::Pause_SFX(musicChannel);

	score = 0;
	lives = 5;
	currentLevel = 0;
}

void NextLevel()
{
	currentEggs = 0;
	currentLevel++;

	if (currentLevel == levelStrings.size())
		afterMenu("YOU WIN!");
	else
		restartLevel();
}

void StartLevel()
{
	renderLoadingScreen();

	texts["SCORE"] = (std::unique_ptr<Text>(new Text(ren, "assets/Hack-Regular.ttf", "SCORE", { 255,0,255 }, Size(100, 50), Vector(70, 30), 25)));

	scoreTxt = (std::unique_ptr<Text>(new Text(ren, "assets/Hack-Regular.ttf", "000000", { 255,0,255 }, Size(100, 50), Vector(200, 30), 25)));

	texts["LIVES"] = (std::unique_ptr<Text>(new Text(ren, "assets/Hack-Regular.ttf", "LIVES", { 255,0,255 }, Size(100, 50), Vector(1100, 30), 25)));
	
	livesText = (std::unique_ptr<Text>(new Text(ren, "assets/Hack-Regular.ttf", "5", { 255,0,255 }, Size(25, 50), Vector(1200, 30), 25)));

	levelMap = std::unique_ptr<TileMap>(new TileMap(levelStrings[currentLevel], ren, addScore, players));

	//loadKeyMaps();

	//run through enemy lists
	for (long unsigned int i = 0; i < levelMap->enemyList.size(); i++)
	{
		addEnemy(levelMap->enemyList[i], levelMap->enemyList2[i]);
	}

	musicChannel = Audio::Fade_In_SFX_And_Loop("Music", 5.0f, -1);
}

void restartLevel()
{
	// delete menu
	delete menu;
	menu = nullptr;

	// delete objects
	for (long unsigned int i = 0; i < players.size(); i++)
		players[i].release();
	texts.clear();
	scoreTxt.release();
	levelMap.release();
	players.clear();

	enemies.clear();

	// stop music on channel
	Audio::Stop_SFX(musicChannel);

	// recreate level
	StartLevel();

	_isPaused = false;
	score = 0;
}

void enemyCollisionCallback()
{
	lives--;
	livesText->ChangeText(std::to_string(lives), ren);
	if (lives == 0)
	{
		lives = 5;
		afterMenu("GAME OVER!");
	}

	for (long unsigned int i = 0; i < players.size(); i++)
		players[i]->ResetPosition();

	for (auto const& spr : enemies)
		spr->ResetPosition();
}

float toSeconds(float nanoseconds)
{
	return nanoseconds / 1000000000;
}

void addScore(LevelPiece::TileType type)
{
	int scoreToAdd = type;
	score += scoreToAdd;
	std::stringstream ss;
	ss << std::setw(6) << std::setfill('0') << score;
	scoreTxt->ChangeText(ss.str(), ren);

	if (type == LevelPiece::EGG)
		currentEggs++;
}

void updateVolume(int valueToChange)
{
	if ((currentVolume + valueToChange) < minMaxVolume[0] || (currentVolume + valueToChange) > minMaxVolume[1])
		return;
	currentVolume += valueToChange;
	Audio::Set_All_SFX_Volumes(currentVolume);
}

void MenuCallback(const Text* myText)
{
	std::string textString = myText->GetText();
	
	if (textString == "EXIT")
		done = true;
	else if (textString == "RESUME")
		_isPaused = false;
	else if (textString == "RESTART")
	{
		restartLevel();
	}
}

void KeyDown(const int keycode)
{
	if (keycode == 0)
	{
		if (!_isPaused) players[1]->Jump();
	}
	else if (keycode == 12)
	{
		if (!_isPaused) players[1]->MoveLeft();
	}
	else if (keycode == 14)
	{
		if (!_isPaused) players[1]->MoveRight();
	}
	else if (keycode == 13)
	{
		if (!_isPaused) players[1]->MoveUp();
	}
	else if (keycode == 15)
	{
		if (!_isPaused) players[1]->MoveDown();
	}

	//player2
	else if (keycode == players[0]->JUMP_KEY)
	{
		if (!_isPaused) players[0]->Jump();
	}
	else if (keycode == players[0]->LEFT_KEY)
	{
		if (!_isPaused) players[0]->MoveLeft();
	}
	else if (keycode == players[0]->RIGHT_KEY)
	{
		if (!_isPaused) players[0]->MoveRight();
	}
	else if (keycode == players[0]->UP_KEY)
	{
		if (!_isPaused) players[0]->MoveUp();
	}
	else if (keycode == players[0]->DOWN_KEY)
	{
		if (!_isPaused) players[0]->MoveDown();
	}
}

void KeyUp(const int keycode)
{
	if (keycode == 12)
	{
		if (!_isPaused) players[1]->StopMovingLeft();
	}
	else if (keycode == 14)
	{
		if (!_isPaused) players[1]->StopMovingRight();
	}
	else if (keycode == 13)
	{
		if (!_isPaused) players[1]->StopMovingUp();
	}
	else if (keycode == 15)
	{
		if (!_isPaused) players[1]->StopMovingDown();
	}

	//plqyer2
	else if (keycode == players[0]->LEFT_KEY)
	{
		if (!_isPaused) players[0]->StopMovingLeft();
	}
	else if (keycode == players[0]->RIGHT_KEY)
	{
		if (!_isPaused) players[0]->StopMovingRight();
	}
	else if (keycode == players[0]->UP_KEY)
	{
		if (!_isPaused) players[0]->StopMovingUp();
	}
	else if (keycode == players[0]->DOWN_KEY)
	{
		if (!_isPaused) players[0]->StopMovingDown();
	}
}

void handleInput()
{
	//Event-based input handling
	//The underlying OS is event-based, so **each** key-up or key-down (for example)
	//generates an event.
	//  - https://wiki.libsdl.org/SDL_PollEvent
	//In some scenarios we want to catch **ALL** the events, not just to present state
	//  - for instance, if taking keyboard input the user might key-down two keys during a frame
	//    - we want to catch based, and know the order
	//  - or the user might key-down and key-up the same within a frame, and we still want something to happen (e.g. jump)
	//  - the alternative is to Poll the current state with SDL_GetKeyboardState

	SDL_Event event; //somewhere to store an event

	//NOTE: there may be multiple events per frame
	while (SDL_PollEvent(&event)) //loop until SDL_PollEvent returns 0 (meaning no more events)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			done = true; //set donecreate remote branch flag if SDL wants to quit (i.e. if the OS has triggered a close event,
							//  - such as window close, or SIGINT
			break;

			//keydown handling - we should to the opposite on key-up for direction controls (generally)
		case SDL_JOYBUTTONDOWN:
			//Keydown can fire repeatable if key-repeat is on.
			//  - the repeat flag is set on the keyboard event, if this is a repeat event
			//  - in our case, we're going to ignore repeat events
			//  - https://wiki.libsdl.org/SDL_KeyboardEvent
			//if (!event.key.repeat)
			//{
				KeyDown(event.jbutton.button);
				switch (event.jbutton.button)
				{
					//hit escape to exit
				case 11: 
				if (_isPaused) done = true; 
				break;
				case 10:
					if (!_pauseDisabled)
					{
						_isPaused = !_isPaused; // toggle pause
					} 
					if (_isPaused && _pauseDisabled)
					{
						restartLevel();
					}
					
					break;
				case 7:
					if (_isPaused) restartLevel();
					break;
				////case SDLK_F11:
					//if (_windowed)
					//{
					//	SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN); // set fullscreen
					//	_windowed = false;
					//}
					//else
					//{
					//	SDL_SetWindowFullscreen(win, 0); // set back to windowed mode
						//_windowed = true;
					//}
					//break;
				}
			//}

			switch (event.jbutton.button)
			{
				case SDLK_EQUALS:
					updateVolume(2);
					break;
				case SDLK_MINUS:
					updateVolume(-2);
					break;
			}
			break;
		case SDL_JOYBUTTONUP:
			KeyUp(event.jbutton.button);
			break;
		case SDL_WINDOWEVENT:
			prevTime = Clock::now(); // make sure that animation doesn't keep going when has been moved
		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				if (menu != nullptr)
				{
					menu->clickMenuItem(Vector(event.button.x, event.button.y));
				}
			}
			break;
		}
	}
}
// end::handleInput[]

Vector checkLadderMovement(Vector playerpos, bool right)
{
	return levelMap->checkPlayerLadderMovement(playerpos, right);
}

// tag::updateSimulation[]
void updateSimulation(double simLength = 0.02) //update simulation with an amount of time to simulate for (in seconds)
{
	// Time since last frame
	auto currTime = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - prevTime).count();
	prevTime = Clock::now();

	if (currentEggs >= levelMap->eggsRequired)
	{
		NextLevel();
	}

	if (_isPaused)
		currTime = 0;

	for (long unsigned int i = 0; i < players.size(); i++)
		players[i]->Update(toSeconds(currTime), levelMap->level, windowSize, checkLadderMovement);

	for (auto const& enemy : enemies)
		enemy->Update(toSeconds(currTime), levelMap->level, players, windowSize, enemyCollisionCallback);

	for (auto const& spr : levelMap->level_animated)
		spr->Update(currTime);

	if (players[0]->getPosition().y > windowSize.height || players[1]->getPosition().y > windowSize.height)
		enemyCollisionCallback();
}

void render()
{
	// check pause state
	if (_isPaused)
	{
		if (menu == nullptr)
		{
			menu = new Menu();
			menu->addText(new Text(ren, "assets/Hack-Regular.ttf", "PAUSED", { 255,255,255 }, Size(250, 100), Vector(windowSize.width / 2, windowSize.height / 2 - 200), 150), MenuCallback); // create pause text in middle of screen
			menu->addText(new Text(ren, "assets/Hack-Regular.ttf", "( + ) To Resume", { 255,255,255 }, Size(550, 100), Vector(windowSize.width / 2, windowSize.height / 2 - 100), 150), MenuCallback); // create pause text in middle of screen
			menu->addText(new Text(ren, "assets/Hack-Regular.ttf", "( R ) To Restart", { 255,255,255 }, Size(550, 100), Vector(windowSize.width / 2, windowSize.height / 2), 150), MenuCallback); // create pause text in middle of screen
			menu->addText(new Text(ren, "assets/Hack-Regular.ttf", "( - ) To Exit", { 255,255,255 }, Size(550, 100), Vector(windowSize.width / 2, windowSize.height / 2 + 100), 150), MenuCallback); // create pause text in middle of screen
			Audio::Pause_SFX(musicChannel);
		}
	}
	else
	{
		if (menu != nullptr)
		{
			delete menu;
			menu = nullptr;
			Audio::Resume_SFX(musicChannel);
		}
	}

	//First clear the renderer
	SDL_RenderClear(ren);

	//Draw the level on screen
	for (auto const& spr : levelMap->level)
		spr->render(ren);

	for (auto const& spr : levelMap->level_animated)
		spr->render(ren);

	//Draw the text on screen
    for (auto const& spr : texts)
		spr.second->render(ren);

	for (auto const& player : players)
		player->render(ren);

	for (auto const& enemy : enemies)
		enemy->render(ren);

	scoreTxt->render(ren);
	livesText->render(ren);
	
	if (menu != nullptr)
		menu->renderMenu(ren);

	//Update the screen
	SDL_RenderPresent(ren);
}

void renderLoadingScreen()
{
	SDL_RenderClear(ren);

	// instantiate the loading text
	texts["LOADING"] = std::unique_ptr<Text>(new Text(ren, "assets/Hack-Regular.ttf", "LOADING...", { 255,255,255 }, Size(250, 100), Vector(windowSize.width / 2, windowSize.height / 2), 150)); // create pause text in middle of screen

	// render the loading text
	texts["LOADING"]->render(ren);

	// present the renderer
	SDL_RenderPresent(ren);

	// cleanup the text (so the screen gets overwritten when everything is loaded)
	texts.erase("LOADING");
}

void cleanExit(int returnValue)
{
	if (ren != nullptr) SDL_DestroyRenderer(ren);
	if (win != nullptr) SDL_DestroyWindow(win);
	SDL_Quit();
	exit(returnValue);
}

// based on http://www.willusher.io/sdl2%20tutorials/2013/08/17/lesson-1-hello-world/
int main( int argc, char* args[] )
{
	
	#if defined(__WIIU__)
	#define ROOT_PATH "fs:/vol/external01/"
	#define HBAS_PATH ROOT_PATH "wiiu/apps/ChuckieEggWiiU"
	//#define ELF_PATH HBAS_PATH "/ChuckieEggWiiU.elf"
	//#define RPX_PATH HBAS_PATH "/ChuckieEggWiiU.rpx"
    //mkdir(HBAS_PATH, 0700);
	chdir(HBAS_PATH);
	#endif

	srand(time(NULL));

	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK  | SDL_INIT_GAMECONTROLLER ) != 0)
	{
		//std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		cleanExit(1);
	}
	//std::cout << "SDL initialised OK!\n";

	//create window
	win = SDL_CreateWindow("Chuckie Egg", windowPosition.x, windowPosition.y, windowSize.width, windowSize.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	//error handling
	if (win == nullptr)
	{
		//std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		cleanExit(1);
	}
	//std::cout << "SDL CreatedWindow OK!\n";

	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr)
	{
		//std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		cleanExit(1);
	}

	SDL_RenderSetLogicalSize(ren, windowSize.width, windowSize.height); 

	if( TTF_Init() == -1 )
	{
		//std::cout << "TTF_Init Failed: " << TTF_GetError() << std::endl;
		cleanExit(1);
	}

	readHighscore();

	StartLevel();

	if (!Audio::init())
	{
		//std::cout << "Issue loading Audio" << std::endl;
	}
	else
	{
		Audio::Load_SFX("assets/audio/bgMusic.mp3", "Music");
		Audio::Load_SFX("assets/audio/pickups.wav", "Pickup");
		Audio::Load_SFX("assets/audio/walk.wav", "Walk");
		Audio::Load_SFX("assets/audio/jump.wav", "Jump");
		Audio::Load_SFX("assets/audio/climb.wav", "LadderClimb");

		musicChannel = Audio::Fade_In_SFX_And_Loop("Music", 5.0f, -1);
	}

	prevTime = Clock::now();
	
	SDL_Joystick *joystick;
	SDL_JoystickEventState(SDL_ENABLE);
    

	for(int i = 0; i < SDL_NumJoysticks(); i++ ) 
    {
        //printf("    %s\n", SDL_JoystickName(i));
		joystick = SDL_JoystickOpen(i);
    }

	while (!done) //loop until done flag is set)
	{
		handleInput(); // this should ONLY SET VARIABLES

		updateSimulation(); // this should ONLY SET VARIABLES according to simulation

		render(); // this should render the world state according to VARIABLES

		//SDL_Delay(20); // unless vsync is on??

		//std::cout << players[1]->_state << std::endl;
	}

	cleanExit(0);
	return 0;
}

