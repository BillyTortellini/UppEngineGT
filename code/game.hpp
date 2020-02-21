#ifndef __GAME_HPP__
#define __GAME_HPP__

// This file contains structures that MUST
// be defined by the game.cpp. 
// It also includes uniforms that are automatically 
// set each frame by the platform layer
// --------------------

// GameData should store all information that is consisten across
// dynamic game reloads.
struct GameData;

// These Prototypes must be implemented:
void gameAfterReset(); // Called after dynamic reload
void gameBeforeReset(); // Called before dynamic reload

void gameTick(); // Called each frame
void gameInit(); // Called once on program startup
void gameShutdown(); // Called once on program shutdown
void gameAudioTick(int size, byte* byte); 

// Globals that are set each frame
GameState* gameState;
GameData* gameData;
Allocator* gameAlloc;










#endif
