
// Description:
// This is a basic game where the player controls a paddle at the bottom of
// the screen to catch blocks falling from the top. The game starts with a
// menu and a play button. The game ends after the player misses 5 blocks.
//
// How to Compile:
// Make sure you have the SDL2, SDL2_image, and SDL2_mixer development
// libraries installed.
// g++ main.cpp -o game -lSDL2 -lSDL2_image -lSDL2_mixer
//
// How to Run:
// Place the following files in the same directory as the executable:
// - "play_button.png"
// - "game_over.png"
// - "background_music.mp3" (or other supported audio format)
//
// Controls:
// - Mouse Click on Play Button: Start the game
// - Mouse Movement: Move paddle left and right
// - Left/Right Arrow Keys: Move paddle left and right
// - Escape Key or Window Close: Quit the game
// =============================================================================

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // Include for PNG loading
#include <SDL2/SDL_mixer.h> // Include for audio
#include <iostream>
#include <vector>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

// --- Configuration Constants ---
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 20;
const int BLOCK_SIZE = 30;
const int PADDLE_SPEED = 10;
const int BLOCK_SPEED = 5;
const int MAX_MISTAKES = 5;

// --- Game State Enum ---
// To manage different states of the game
enum GameState
{
     MENU,
     PLAYING,
     GAME_OVER
};

// --- Game Structures ---

// Represents the player's paddle
struct Player
{
     SDL_Rect rect; // Position and dimensions
};

// Represents a single falling block
struct Block
{
     SDL_Rect rect; // Position and dimensions
};

// --- Helper Function ---

// Function to load a texture from a file
SDL_Texture *loadTexture(const std::string &path, SDL_Renderer *renderer)
{
     SDL_Texture *newTexture = nullptr;
     SDL_Surface *loadedSurface = IMG_Load(path.c_str());
     if (loadedSurface == nullptr)
     {
          std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
     }
     else
     {
          newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
          if (newTexture == nullptr)
          {
               std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
          }
          SDL_FreeSurface(loadedSurface);
     }
     return newTexture;
}

int main(int argc, char *args[])
{
     // --- 1. Initialization ---

     // Initialize SDL video and audio subsystems
     if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
     {
          std::cerr << "Could not initialize SDL! SDL_Error: " << SDL_GetError() << std::endl;
          return 1;
     }

     // Initialize SDL_image for PNG loading
     int imgFlags = IMG_INIT_PNG;
     if (!(IMG_Init(imgFlags) & imgFlags))
     {
          std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
          SDL_Quit();
          return 1;
     }

     // Initialize SDL_mixer for audio playback
     if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
     {
          std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
          IMG_Quit();
          SDL_Quit();
          return 1;
     }

     // Create a window
     SDL_Window *window = SDL_CreateWindow(
         "Catch the Block",
         SDL_WINDOWPOS_UNDEFINED,
         SDL_WINDOWPOS_UNDEFINED,
         SCREEN_WIDTH,
         SCREEN_HEIGHT,
         SDL_WINDOW_SHOWN);
     if (window == nullptr)
     {
          std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
          Mix_Quit();
          IMG_Quit();
          SDL_Quit();
          return 1;
     }

     // Create a renderer for drawing
     SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
     if (renderer == nullptr)
     {
          std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
          SDL_DestroyWindow(window);
          Mix_Quit();
          IMG_Quit();
          SDL_Quit();
          return 1;
     }

     // Seed the random number generator
     srand(time(0));

     // --- 2. Game Asset and Variable Setup ---

     // Game state variable, start in the menu
     GameState currentState = MENU;
     int mistakes = 0;

     // Load menu and game over textures
     SDL_Texture *playButtonTexture = loadTexture("play_button.png", renderer);
     SDL_Texture *gameOverTexture = loadTexture("game_over.png", renderer);

     if (playButtonTexture == nullptr || gameOverTexture == nullptr)
     {
          std::cerr << "Failed to load one or more textures. Make sure they are in the correct directory." << std::endl;
          SDL_DestroyRenderer(renderer);
          SDL_DestroyWindow(window);
          Mix_Quit();
          IMG_Quit();
          SDL_Quit();
          return 1;
     }

     // Define the play button's position and size
     SDL_Rect playButtonRect;
     playButtonRect.w = 250;
     playButtonRect.h = 100;
     playButtonRect.x = (SCREEN_WIDTH - playButtonRect.w) / 2;
     playButtonRect.y = (SCREEN_HEIGHT - playButtonRect.h) / 2;

     // Load background music
     Mix_Music *backgroundMusic = Mix_LoadMUS("background_music.mp3");
     if (backgroundMusic == nullptr)
     {
          std::cerr << "Failed to load background music! SDL_mixer Error: " << Mix_GetError() << std::endl;
          SDL_DestroyTexture(playButtonTexture);
          SDL_DestroyTexture(gameOverTexture);
          SDL_DestroyRenderer(renderer);
          SDL_DestroyWindow(window);
          Mix_Quit();
          IMG_Quit();
          SDL_Quit();
          return 1;
     }

     // Create the player's paddle
     Player player;
     player.rect.w = PADDLE_WIDTH;
     player.rect.h = PADDLE_HEIGHT;
     player.rect.x = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
     player.rect.y = SCREEN_HEIGHT - PADDLE_HEIGHT - 10;

     // Create the first falling block
     Block block;
     block.rect.w = BLOCK_SIZE;
     block.rect.h = BLOCK_SIZE;
     block.rect.x = rand() % (SCREEN_WIDTH - BLOCK_SIZE);
     block.rect.y = 0;

     // --- 3. Game Loop ---

     bool isRunning = true;
     SDL_Event event;

     while (isRunning)
     {
          // --- Event Handling ---
          while (SDL_PollEvent(&event) != 0)
          {
               if (event.type == SDL_QUIT)
               {
                    isRunning = false;
               }
               if (event.type == SDL_KEYDOWN)
               {
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                         isRunning = false;
                    }
               }
               // Handle mouse clicks for the menu
               if (event.type == SDL_MOUSEBUTTONDOWN)
               {
                    if (currentState == MENU)
                    {
                         int mouseX, mouseY;
                         SDL_GetMouseState(&mouseX, &mouseY);
                         SDL_Point mousePoint = {mouseX, mouseY};
                         if (SDL_PointInRect(&mousePoint, &playButtonRect))
                         {
                              currentState = PLAYING;
                              // Start music when game starts
                              Mix_PlayMusic(backgroundMusic, -1);
                         }
                    }
               }
               // Handle mouse movement for the paddle
               if (event.type == SDL_MOUSEMOTION)
               {
                    if (currentState == PLAYING)
                    {
                         player.rect.x = event.motion.x - (player.rect.w / 2);
                    }
               }
          }

          // --- KEYBOARD INPUT ---
          const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);
          if (currentState == PLAYING)
          {
               if (currentKeyStates[SDL_SCANCODE_LEFT])
               {
                    player.rect.x -= PADDLE_SPEED;
               }
               if (currentKeyStates[SDL_SCANCODE_RIGHT])
               {
                    player.rect.x += PADDLE_SPEED;
               }
          }

          // --- Game Logic (Only runs if we are in the PLAYING state) ---
          if (currentState == PLAYING)
          {
               // Keep paddle within screen bounds
               if (player.rect.x < 0)
               {
                    player.rect.x = 0;
               }
               if (player.rect.x > SCREEN_WIDTH - player.rect.w)
               {
                    player.rect.x = SCREEN_WIDTH - player.rect.w;
               }

               // Move the block down
               block.rect.y += BLOCK_SPEED;

               // Check for collision between paddle and block
               if (SDL_HasIntersection(&player.rect, &block.rect))
               {
                    std::cout << "Caught it!" << std::endl;
                    block.rect.y = 0;
                    block.rect.x = rand() % (SCREEN_WIDTH - BLOCK_SIZE);
               }

               // Check if block missed the paddle and hit the bottom
               if (block.rect.y > SCREEN_HEIGHT)
               {
                    mistakes++;
                    std::cout << "Missed! Mistakes: " << mistakes << std::endl;
                    block.rect.y = 0;
                    block.rect.x = rand() % (SCREEN_WIDTH - BLOCK_SIZE);

                    if (mistakes >= MAX_MISTAKES)
                    {
                         std::cout << "GAME OVER!" << std::endl;
                         currentState = GAME_OVER;
                         Mix_HaltMusic(); // Stop the music on game over
                    }
               }
          }

          // --- Rendering ---
          SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
          SDL_RenderClear(renderer);

          switch (currentState)
          {
          case MENU:
               SDL_RenderCopy(renderer, playButtonTexture, NULL, &playButtonRect);
               break;
          case PLAYING:
               SDL_SetRenderDrawColor(renderer, 100, 180, 255, 255);
               SDL_RenderFillRect(renderer, &player.rect);

               SDL_SetRenderDrawColor(renderer, 255, 220, 50, 255);
               SDL_RenderFillRect(renderer, &block.rect);
               break;

          case GAME_OVER:
               SDL_RenderCopy(renderer, gameOverTexture, NULL, NULL);
               break;
          }

          SDL_RenderPresent(renderer);
          SDL_Delay(16);
     }

     // --- 4. Cleanup ---
     Mix_FreeMusic(backgroundMusic);
     backgroundMusic = nullptr;

     SDL_DestroyTexture(playButtonTexture);
     SDL_DestroyTexture(gameOverTexture);
     playButtonTexture = nullptr;
     gameOverTexture = nullptr;

     SDL_DestroyRenderer(renderer);
     SDL_DestroyWindow(window);
     renderer = nullptr;
     window = nullptr;

     Mix_Quit();
     IMG_Quit();
     SDL_Quit();

     return 0;
}
