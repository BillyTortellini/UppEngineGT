#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <dlfcn.h>
#include <cmath>

// Non-standard Libraries
#include <SDL.h>

// Own headers
#include <game.hpp>
#include "datatypes.hpp"
#include "timing.cpp"
#include "filewatcher.cpp"

/* TODO:
 * ------------------------------------
 * Adding sound output
 * OpenGL availability
 * Vector library
 * SIMD introduction
 * Logging would be nice
 *                                     */

// Dynamic stuff
static gameInitFunc gameInit;
static gameTickFunc gameTick;
static gameShutdownFunc gameShutdown;
static gameAudioFunc gameAudio;

void defaultGameInit(GameMemory* memory) {return;};
void defaultGameTick(GameState* memory) {return;};
void defaultGameShutdown(GameMemory* memory) {return;};
void defaultGameAudio(GameState* memory, byte* stream, int length)
{ 
    memset(stream, 0, length); 
    return;
};

void* libHandle = NULL;
void loadGameDynamic()
{
    if (libHandle != NULL)
    {
        // Close previous handle
        dlclose(libHandle);
    }

    system("cp build/libgame_tmp.so build/libgame_tmp2.so");

    libHandle = dlopen("build/libgame_tmp2.so", RTLD_LAZY);
    if (!libHandle) {
        gameInit = &defaultGameInit;
        gameTick = &defaultGameTick;
        gameShutdown = &defaultGameShutdown;
        gameAudio = &defaultGameAudio;
        
        printf("dlopen failed: %s\n",
                dlerror());
        return;
    }

    gameInit = (gameInitFunc) dlsym(libHandle, "gameInit");
    gameShutdown = (gameShutdownFunc) dlsym(libHandle, "gameShutdown");
    gameTick = (gameTickFunc) dlsym(libHandle, "gameTick");
    gameAudio = (gameAudioFunc) dlsym(libHandle, "gameAudio");

    if (gameInit == NULL ||
            gameTick == NULL || 
            gameShutdown == NULL ||
            gameAudio == NULL) {
        gameInit = &defaultGameInit;
        gameTick = &defaultGameTick;
        gameShutdown = &defaultGameShutdown;
        gameAudio = &defaultGameAudio;
        printf("dlsym failed: %s\n", dlerror());
        return;
    }
}
GameState gameState;
uint64 runningSampleIndex = 0;
void mixAudio(void* unused, uint8* stream, int length)
{
    gameAudio(&gameState, stream, length);
}

SDL_AudioDeviceID audioID;
void lockSound() {
    if (audioID != 0) 
    {
        SDL_LockAudioDevice(audioID);
    }
}

void unlockSound() {
    if (audioID != 0) {
        SDL_UnlockAudioDevice(audioID);
    }
}

int main(int argc, char** argv)
{
    // Init Subsystems
    init_timing();
    int watchToken = init_filewatcher("build/libgame_tmp.so");
    // Load game code
    loadGameDynamic();

    // GameState
    GameInput* input = &gameState.input;
    GameMemory* memory = &gameState.memory;
    GameTime* time = &gameState.time;
    SoundInfo* soundInfo = &gameState.soundInfo;
    WindowState* winState = &gameState.winState;
    GameVideo* video = &gameState.video;
    PlatformFunctions* platformFuncs = &gameState.platformFunctions;

    winState->width = 600;
    winState->height = 400;
    video->width = winState->width;
    video->height = winState->height;

    // SDL
    if (SDL_Init(SDL_INIT_VIDEO | 
                SDL_INIT_AUDIO |
                SDL_INIT_EVENTS) < 0) {
        printf("Could not initialize SDL! Error: %s\n", 
                SDL_GetError());
        return 1;
    }

    // Init SDL Audio
    {
        const char* audioDevName = NULL;
        //int audioDeviceNum = SDL_GetNumAudioDevices(0);
        //printf("Available audio devices:\n");
        //for (int i = 0; i < audioDeviceNum; i++) {
        //    const char* name = SDL_GetAudioDeviceName(i, 0);
        //    if (name == NULL) {
        //        printf("  %d Error\n", i);
        //    }
        //    else {
        //        printf("  %d %s\n", i, name);
        //    }
        //}

        //printf("Enter audio device number: ");
        //int selectedNum;
        //int res = scanf("%d", &selectedNum);
        //if (res > 0) {
        //    audioDevName = SDL_GetAudioDeviceName(selectedNum, 0);
        //}

        SDL_AudioSpec receivedFormat;
        SDL_AudioSpec requestedFormat; 
        requestedFormat.freq = 48000;
        requestedFormat.format = AUDIO_S16;
        requestedFormat.channels = 1;
        requestedFormat.samples = 512;
        requestedFormat.callback = &mixAudio;
        requestedFormat.userdata = NULL;

        audioID = SDL_OpenAudioDevice(audioDevName, false, 
                &requestedFormat, &receivedFormat,
                SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
        if (audioID == 0) {
            printf("Error using sdl_openaudio, %s\n", SDL_GetError());
            return 1;
        }

        soundInfo->sampleFreq = receivedFormat.freq;
        soundInfo->numChannels = receivedFormat.channels;
        soundInfo->sampleDelay = receivedFormat.samples;
        soundInfo->bytePerSample = 2;

        platformFuncs->lockSound = &lockSound;
        platformFuncs->unlockSound = &unlockSound;
    }

    // Init SDL Video
    SDL_Window* window = NULL;

    uint32 winFlags = SDL_WINDOW_SHOWN |
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("TestWindow", 
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            winState->width, winState->height,
            winFlags);

    if (window == NULL) {
        printf("Could not create SDL Window: %s\n",
                SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer;
    renderer = SDL_CreateRenderer(window, -1, 0);

    if (renderer == NULL) {
        printf("Could not create SDL Renderer: %s\n",
                SDL_GetError());
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STATIC,
            winState->width, winState->height);

    if (texture == NULL) {
        printf("Could not create SDL Texture: %s\n",
                SDL_GetError());
        return 1;
    }

    byte* pixelData = (byte*) malloc(4 * 600 * 400);
    memset(pixelData, 255, 4 * 600 * 400);
    video->pixelData = pixelData;

    // Initialize memory
    memory->size = 1024 * 1024 * 1024; // 1GB
    memory->memory = (byte*) malloc(memory->size);

    // Initialize input
    int numKeys;
    SDL_GetKeyboardState(&numKeys);

    const byte* keyboardState = 
        SDL_GetKeyboardState(NULL);
    byte* oldKeyboardState = new byte[numKeys];
    byte* keysPressed = new byte[numKeys];

    byte mouseKeyDown[3];
    byte oldMouseKeyDown[3];
    byte mouseKeyPressed[3];

    input->keysDown = keyboardState;
    input->keysPressed = keysPressed;
    input->mouseKeyDown = mouseKeyDown;
    input->mouseKeyPressed = mouseKeyPressed;

    // Init game
    gameInit(memory);

    // Timing info
    uint64 frameStartTime = getTime();
    uint64 gameStartTime = frameStartTime;
    int FPS = 60;
    uint64 timePerFrame = (1000000 / FPS);

    SDL_PauseAudioDevice(audioID, 0); // Start playing audio

    // Message Loop
    bool quit = false;
    while (!quit)
    {
        // Save old keyboard state
        memcpy(oldKeyboardState, keyboardState, 
                numKeys * sizeof(byte));
        memcpy(oldMouseKeyDown, mouseKeyDown,
                3 * sizeof(byte));

        SDL_Event event;
        while (SDL_PollEvent(&event) == 1) 
        {
            if (event.type == SDL_QUIT) {
                quit = true;
            }

            // Handle window events
            if (event.type == SDL_WINDOWEVENT)
            {
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_SIZE_CHANGED: 
                        // Resize texture
                        winState->width = event.window.data1;
                        winState->height = event.window.data2;
                        video->width = winState->width;
                        video->height = winState->height;

                        SDL_DestroyTexture(texture);
                        free(pixelData);

                        pixelData = (byte*) malloc(4 * winState->width * winState->height);
                        video->pixelData = pixelData;

                        texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STATIC,
                                winState->width, winState->height);
                        break;
                    case SDL_WINDOWEVENT_CLOSE: 
                        quit= true;
                        break;
                }
            }
        }

        // Calc keypressed
        for (int i = 0; i< numKeys; i++) {
            keysPressed[i] = 
                keyboardState[i] && !oldKeyboardState[i];
        }

        // Get Mouse Input
        uint32 mouseState = SDL_GetMouseState(
                &input->mouseX, &input->mouseY);
        mouseKeyDown[0] = mouseState & 
            SDL_BUTTON(SDL_BUTTON_LEFT);
        mouseKeyDown[1] = mouseState & 
            SDL_BUTTON(SDL_BUTTON_RIGHT);
        mouseKeyDown[2] = mouseState & 
            SDL_BUTTON(SDL_BUTTON_MIDDLE);
        for (int i = 0; i < 3; i++) {
            mouseKeyPressed[i] = 
                mouseKeyDown[i] && !oldMouseKeyDown[i];
        }

        // Update and draw
        gameTick(&gameState);

        SDL_UpdateTexture(texture, NULL, 
                pixelData,
                4 * winState->width);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // Handle game feedback
        if (winState->fullscreenTarget !=
                winState->fullscreen)
        {
            if (winState->fullscreenTarget) {
                SDL_SetWindowFullscreen(window,
                        SDL_WINDOW_FULLSCREEN_DESKTOP);
            }
            else {
                SDL_SetWindowFullscreen(window, 0);
            }
            winState->fullscreen = winState->fullscreenTarget;
        }

        if (winState->quitTarget) {
            quit = true;
        }

        // Dynamic loading
        if (hasFileChanged(watchToken)) {
            printf("File changed\n");
            SDL_PauseAudioDevice(audioID, 1); // Stop audio
            loadGameDynamic();
            SDL_PauseAudioDevice(audioID, 0); // Resume audio
        }

        // Timing/Sleeping
        uint64 workEndTime = getTime();
        uint64 sleepUntil = frameStartTime + timePerFrame;

        // Sleep
        int64 sleepFor = sleepUntil - workEndTime - 1000;
        if (sleepFor > 0) {
            microsleep(sleepFor);
        }

        if (workEndTime > sleepUntil) {
            // Warn about not sleeping, we are over time!
            uint64 diff = workEndTime - frameStartTime;
            printf("Did not sleep, frame took: %3.2fms\n", diff/1000.0f);
        }

        // Busy wait until time really went past
        uint64 now = getTime(); while (now < sleepUntil) {
            now = getTime();
        }

        uint64 frameEndTime = getTime();

        // Passed time
        time->now = (frameEndTime - gameStartTime) / 
            1000000.0;
        time->tslf = (frameEndTime - frameStartTime) / 
            1000000.0;

        // Reset frameStartTime for next loop
        frameStartTime = frameEndTime;
    }

    //waitEnter();

    gameShutdown(memory);
    free(pixelData);
    SDL_CloseAudio();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

