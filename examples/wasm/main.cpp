#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vectormath.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <GLES2/gl2.h>
#include <emscripten.h>

#include "Game/Game.h"
#include "Renderer/Renderer.h"

int main(int argc, const char* argv[])
{
    SDL_Window*     window;
    SDL_Renderer*   renderer;

    SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "1");
    SDL_Init(SDL_INIT_EVERYTHING);

    int windowWidth = 1280;
    int windowHeight = 720;

    int init_result = SDL_CreateWindowAndRenderer(windowWidth, windowHeight, SDL_WINDOW_OPENGL, &window, &renderer);
    if (init_result != 0)
    {
        printf("Cannot init SDL :(\n");
        printf("%s\n", SDL_GetError());
        return 0;
    }

    if (Renderer_Setup(window) != 0)
	{
		return -1;
	}

    int spriteCount = 8;
    SpriteBatch spriteBatch;
    Renderer_LoadSpritesHorizontal(&spriteBatch, "sprites.png", spriteCount);

    int objectCount = 1000;
    Game_Setup(&spriteBatch, objectCount);

    float   deltaTime           = 0.0f;
    uint64_t timerFrequency     = SDL_GetPerformanceCounter();
    uint64_t timerTicks         = SDL_GetPerformanceFrequency();
    uint64_t timerInterval      = timerFrequency / 60;
    double  invTimerFrequency   = 1.0 / (double)timerFrequency;

    printf("Start game =)\n");
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
                break;
            }
        }

        // Update game
        Game_Update(0.0f, deltaTime);

        // Clear back buffer
        Renderer_Clear();

        // Render game
        Game_Render();

        // Draw fps
        char fpsText[1024];
        snprintf(fpsText, sizeof(fpsText), "FPS: %.3f", deltaTime > FLOAT_EPSILON ? 1.0f / deltaTime : 0.0f);

        char objectCountText[1024];
        snprintf(objectCountText, sizeof(objectCountText), "Object count: %d", objectCount);
        
        vec2 fpsTextSize = Renderer_TextSize(fpsText) * vec2_new(2.0f, 2.0f);

        Renderer_DrawQuad(
            vec2_new(0.0f, (float)windowHeight), 
            vec2_new(fpsTextSize.x + 10.0f, (float)windowHeight - 1.5f * fpsTextSize.y - 10.0f), 
            vec3_new1(0.0f));
        Renderer_DrawText(fpsText, vec2_new(5.0f, (float)windowHeight - 5.0f), vec3_new(0.25f, 0.5f, 1.0f));
        Renderer_DrawText(objectCountText, vec2_new(5.0f, (float)windowHeight - 45.0f), vec3_new(0.25f, 0.5f, 1.0f));

        // Present back buffer
        Renderer_Present();

        // Update timers
        uint64_t currentTicks = SDL_GetPerformanceCounter();
        uint64_t elapsedTicks = currentTicks - timerTicks;
        
        if (elapsedTicks < timerInterval)
        {
            double sleepSeconds = (double)(timerInterval - elapsedTicks) * invTimerFrequency;
            emscripten_sleep((Uint32)(sleepSeconds * 1000));

            elapsedTicks = timerInterval;
        }

        deltaTime = (float)((double)elapsedTicks * invTimerFrequency);
        timerTicks = timerTicks + elapsedTicks;
    }

    Renderer_Shutdown(window);
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

//! LEAVE AN EMPTY LINE HERE, REQUIRE BY GCC/G++
