#include <bits/stdc++.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
using namespace std;
//  ......................

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int RECTANGLE_SIZE = 20;
const int FOOD_SIZE = 20;
const int SNAKE_SPEED = 150;
bool quit = false;
Mix_Chunk* eatSound =NULL;
Mix_Music* startMusic =NULL;
Mix_Music* gameOverMusic =NULL;
Mix_Music* gameBackground = NULL;
Mix_Chunk* bonusSound=NULL;
SDL_Window* window =NULL;
SDL_Renderer* renderer =NULL;
TTF_Font* font = NULL;
SDL_Texture* coverTexture =NULL;
SDL_Texture* headTexture =NULL;
SDL_Texture* bodyTexture =NULL;
SDL_Texture* tailTexture =NULL;
SDL_Texture* backgroundTexture =NULL;
SDL_Texture* foodTexture =NULL;
SDL_Texture* bonusFoodTexture = NULL;
SDL_Texture* menuTexture = NULL;
SDL_Texture *gameIcon = NULL;
SDL_Texture *highScoresIcon = NULL;
SDL_Texture *settingsIcon = NULL;
SDL_Texture *highScoreBG = NULL;
SDL_Texture* returnIconTexture = NULL;
SDL_Texture* gameOverTexture = NULL;

const char *filename = "highscore.txt";
int highScore;

struct Snake {
    int x, y;
    double angle;
};
enum GameState {
   
    MAIN_MENU,
    GAME,
    HIGH_SCORES,
    SETTINGS,
    EXIT,
    QUIT
};

GameState currentState = MAIN_MENU;

// Function to draw a rectangle (used for food, no longer needed after adding food image)
void drawRectangle(SDL_Renderer* renderer, int x, int y, int size, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x, y, size, size};
    SDL_RenderFillRect(renderer, &rect);
}

// Function to check collision between snake head and food using SDL_HasIntersection
bool checkCollision(const Snake& snake, int x, int y) {
    SDL_Rect snakeRect = {snake.x, snake.y, RECTANGLE_SIZE, RECTANGLE_SIZE};
    SDL_Rect foodRect = {x, y, FOOD_SIZE, FOOD_SIZE};
    return SDL_HasIntersection(&snakeRect, &foodRect);
}

// Function to check if the snake bites itself
bool checkSelfBite(const std::vector<Snake>& body) {
    for (size_t i = 1; i < body.size(); ++i) {
        if (body[0].x == body[i].x && body[0].y == body[i].y) {
            return true;
        }
    }
    return false;
}


bool checkFoodOnSnake(const std::vector<Snake>& body, int x, int y) {
    for (const auto& segment : body) {
        if (segment.x == x && segment.y == y) {
            return true;
        }
    }
    return false;
}


SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path) {
    SDL_Texture* newTexture = NULL;
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        cout<< "Unable to load image! SDL_image Error: " << IMG_GetError() << " for image: " << path <<endl;
        return nullptr;
    }
    else{
        newTexture = SDL_CreateTextureFromSurface(renderer, surface);
        if (newTexture == NULL) {
            std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
        }
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}


void cleanUp(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font,  Mix_Music* startMusic, std::vector<SDL_Texture*> textures) {
    for (auto texture : textures) {
        SDL_DestroyTexture(texture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    Mix_FreeMusic(startMusic);

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    Mix_FreeChunk(eatSound);

    Mix_CloseAudio();
    SDL_Quit();
}
void saveHighScore(const char *filename, int highScore)
{
    std::ofstream file(filename);
    if (file.is_open())
    {
        file << highScore;
        file.close();
    }
    else
    {
        std::cout << "Unable to open file to write high score" << std::endl;
    }
}

void loadHighScore(const char *filename, int &highScore)
{
    std::ifstream file(filename);
    if (file.is_open())
    {
        file >> highScore;
        file.close();
    }
    else
    {
        std::cout << "File does not exist. Creating a new high score file." << std::endl;
        highScore = 0;
        saveHighScore(filename, highScore); // Create the file with initial score 0
    }
}

void handleEvents(SDL_Event& e, int& dx, int& dy){
    while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        if (dy != RECTANGLE_SIZE) {
                            dx = 0;
                            dy = -RECTANGLE_SIZE;
                        }
                        break;
                    case SDLK_DOWN:
                        if (dy != -RECTANGLE_SIZE) {
                            dx = 0;
                            dy = RECTANGLE_SIZE;
                        }
                        break;
                    case SDLK_LEFT:
                        if (dx != RECTANGLE_SIZE) {
                            dx = -RECTANGLE_SIZE;
                            dy = 0;
                        }
                        break;
                    case SDLK_RIGHT:
                        if (dx != -RECTANGLE_SIZE) {
                            dx = RECTANGLE_SIZE;
                            dy = 0;
                        }
                        break;
                }
            }
        }
}

void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y) {
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}/////
void renderMainMenu(SDL_Renderer* renderer, SDL_Texture* gameIcon, SDL_Texture* highScoresIcon, SDL_Texture* settingsIcon,GameState selectedItem, SDL_Texture*  menuTexture,SDL_Texture*  exitIcon) {
    SDL_RenderClear(renderer); // Clear the screen
    //SDL_Rect coverTexture = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer,  menuTexture, NULL, NULL);
    SDL_Rect dstRect;
    SDL_Color highlightColor = {255, 255, 0, 255}; // Yellow color for highlighting
    SDL_Color normalColor = {255, 255, 255, 255};  // White color for normal

    // Render Game Icon
    dstRect = {250, 100, 250, 100};
    if (selectedItem == GAME) {
        SDL_SetTextureColorMod(gameIcon, highlightColor.r, highlightColor.g, highlightColor.b); // Highlight
    } else {
        SDL_SetTextureColorMod(gameIcon, normalColor.r, normalColor.g, normalColor.b); // Normal
    }
    SDL_RenderCopy(renderer, gameIcon, NULL, &dstRect);

    // Render High Scores Icon
    dstRect = {250, 200, 250, 100};
    if (selectedItem == HIGH_SCORES) {
        SDL_SetTextureColorMod(highScoresIcon, highlightColor.r, highlightColor.g, highlightColor.b); // Highlight
    } else {
        SDL_SetTextureColorMod(highScoresIcon, normalColor.r, normalColor.g, normalColor.b); // Normal
    }
    SDL_RenderCopy(renderer, highScoresIcon, NULL, &dstRect);

    // Render Settings Icon
    dstRect = {250, 300, 250, 100};
    if (selectedItem == SETTINGS) {
        SDL_SetTextureColorMod(settingsIcon, highlightColor.r, highlightColor.g, highlightColor.b); // Highlight
    } else {
        SDL_SetTextureColorMod(settingsIcon, normalColor.r, normalColor.g, normalColor.b); // Normal
    }
    SDL_RenderCopy(renderer, settingsIcon, NULL, &dstRect);
    dstRect = {250, 400, 250, 100};
    if (selectedItem == EXIT) {
        SDL_SetTextureColorMod(exitIcon, highlightColor.r, highlightColor.g, highlightColor.b); // Highlight
    } else {
        SDL_SetTextureColorMod(exitIcon, normalColor.r, normalColor.g, normalColor.b); // Normal
    }
    SDL_RenderCopy(renderer, exitIcon, NULL, &dstRect);

    SDL_RenderPresent(renderer); // Present the renderer
}
void mainMenuLoop(SDL_Renderer* renderer) {
    SDL_Event e;
    bool menuRunning = true;
    GameState selectedItem = GAME; // Default selected item

    // Load icons
    SDL_Texture* gameIcon = loadTexture(renderer, "new_game.png");
    SDL_Texture* highScoresIcon = loadTexture(renderer, "high_score.png");
    SDL_Texture* settingsIcon = loadTexture(renderer, "setting.png");
    SDL_Texture* exitIcon = loadTexture(renderer, "exit.png");

    while (menuRunning) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                menuRunning = false;
                currentState = QUIT;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);

                    // Check if mouse is over any menu option
                    if (x >= 250 && x <= 500) {
                        if (y >= 100 && y <= 200) {
                            currentState = GAME;
                            menuRunning = false;
                        } else if (y >= 200 && y <= 300) {
                            currentState = HIGH_SCORES;
                            menuRunning = false;
                        } else if (y >= 300 && y <= 400) {
                            currentState = SETTINGS;
                            menuRunning = false;
                        }
                        else if (y >= 400  && y <= 500) {
                            currentState = EXIT;
                            menuRunning = false;
                        }
                    }
                }
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        if (selectedItem == GAME) {
                            selectedItem = EXIT;
                        }if (selectedItem == EXIT) {
                            selectedItem = SETTINGS;
                        }else if (selectedItem == SETTINGS) {
                            selectedItem = HIGH_SCORES;
                        } else if (selectedItem == HIGH_SCORES) {
                            selectedItem = GAME;
                        } 
                        break;
                    case SDLK_DOWN:
                        if (selectedItem == GAME) {
                            selectedItem = HIGH_SCORES;
                        } else if (selectedItem == HIGH_SCORES) {
                            selectedItem = SETTINGS;
                        } else if (selectedItem == SETTINGS) {
                            selectedItem = EXIT;
                        }
                        else if (selectedItem == EXIT) {
                            selectedItem = GAME;
                        }
                        break;
                    case SDLK_RETURN:
                        currentState = selectedItem;
                        menuRunning = false;
                        break;
                }
            }
        }

        renderMainMenu(renderer, gameIcon, highScoresIcon, settingsIcon, selectedItem,  menuTexture, exitIcon);
    }

    // Clean up
    if (gameIcon) SDL_DestroyTexture(gameIcon);
    if (highScoresIcon) SDL_DestroyTexture(highScoresIcon);
    if (settingsIcon) SDL_DestroyTexture(settingsIcon);
    if (exitIcon) SDL_DestroyTexture(exitIcon);
}


void renderGame(SDL_Renderer* renderer, const std::vector<Snake>& body, int foodX, int foodY, bool bonusFoodActive, int bonusFoodX, int bonusFoodY, int score, bool gameover){
    SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL); // Draw the background

        // Draw food using the food texture
        SDL_Rect foodRect = {foodX, foodY, FOOD_SIZE, FOOD_SIZE};
        SDL_RenderCopy(renderer, foodTexture, NULL, &foodRect);

        // Draw bonus food using the bonus food texture
        if (bonusFoodActive) {
            SDL_Rect bonusFoodRect = {bonusFoodX, bonusFoodY, FOOD_SIZE, FOOD_SIZE};
            SDL_RenderCopy(renderer, bonusFoodTexture, NULL, &bonusFoodRect);
        }

        // Draw snake body segments
        for (size_t i = 1; i < body.size() - 1; ++i) {
            SDL_Rect bodyRect = {body[i].x, body[i].y, RECTANGLE_SIZE, RECTANGLE_SIZE};
            SDL_RenderCopyEx(renderer, bodyTexture, NULL, &bodyRect, body[i].angle, NULL, SDL_FLIP_NONE);
        }

        // Draw snake tail
        SDL_Rect tailRect = {body.back().x, body.back().y, RECTANGLE_SIZE, RECTANGLE_SIZE};
        SDL_RenderCopyEx(renderer, tailTexture, NULL, &tailRect, body.back().angle, NULL, SDL_FLIP_NONE);

        // Draw snake head with rotation
        SDL_Rect headRect = {body[0].x, body[0].y, RECTANGLE_SIZE, RECTANGLE_SIZE};
        SDL_RenderCopyEx(renderer, headTexture, NULL, &headRect, body[0].angle, NULL, SDL_FLIP_NONE);

        // Render the score
        SDL_Color scoreColor = {0, 51, 102, 255}; // White color for score
        std::string scoreText = "Score: " + std::to_string(score);
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), scoreColor);
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        SDL_Rect scoreRect;
        scoreRect.x = 10; // Position the score at the top-left corner
        scoreRect.y = 10;
        scoreRect.w = scoreSurface->w;
        scoreRect.h = scoreSurface->h;
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
        SDL_FreeSurface(scoreSurface);
        SDL_DestroyTexture(scoreTexture);
        

        // Display game over message if the game is over
        if (gameover) {
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, gameOverTexture, NULL, NULL);
        SDL_Color color = {102, 102, 0, 0};
        std::string Score =std::to_string(score);
        SDL_Surface* surface = TTF_RenderText_Solid(font, Score.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    SDL_Rect dstrect = {390, 417, texW, texH};
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);

        }

        SDL_RenderPresent(renderer);
}
void initialize(){
if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        exit(1);
    }

if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
    exit(1);
}

if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
    exit(1);
}

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        Mix_FreeMusic(startMusic);
        exit(1);
    }

     renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    font = TTF_OpenFont("font.ttf", 35);
    if (font == NULL) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        exit(1);
    }
}
void initializeTextures(){
    coverTexture = loadTexture(renderer, "snake_cover.png");
    if (!coverTexture) {
        std::cerr << "Failed to load cover texture!" << std::endl;
        exit(1);
    }
    menuTexture=  loadTexture(renderer, "menu_cover.png");
    headTexture = loadTexture(renderer, "snake_head.png");
    bodyTexture = loadTexture(renderer, "snake_body.png");
    tailTexture = loadTexture(renderer, "snake_tail.png");
    highScoreBG = loadTexture(renderer, "high_score_bg.png");
    backgroundTexture = loadTexture(renderer, "snake_background.png");
    foodTexture = loadTexture(renderer, "food.png");
    bonusFoodTexture = loadTexture(renderer, "bonus_food.png");
    returnIconTexture = loadTexture(renderer, "returnbatton.png");
    gameOverTexture =  loadTexture(renderer, "game_over.png");
    if (!headTexture || !bodyTexture || !tailTexture || !backgroundTexture || !foodTexture || !bonusFoodTexture || !menuTexture || !highScoreBG || !returnIconTexture || !gameOverTexture) {
        std::cerr << "Failed to load one or more textures!" << std::endl;
        exit(1);
    }

}
void initializeSounds(){
    startMusic = Mix_LoadMUS("startgame.mp3");
    gameOverMusic = Mix_LoadMUS("gameover.mp3");
    gameBackground= Mix_LoadMUS("gameback.mp3");
    eatSound = Mix_LoadWAV("eat.wav");
    bonusSound = Mix_LoadWAV("ting.wav");
    if (!startMusic || !gameOverMusic || !gameBackground || !eatSound || !bonusSound) {
        std::cerr << "Failed to load one or more sound files!" << std::endl;
        exit(1);
    }


}

void gameLoop(){
     bool startGame = true;
    SDL_Event e;
    while (startGame) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                startGame=false;
                currentState=QUIT;
            cleanUp(window, renderer, font,startMusic,{coverTexture});
            Mix_FreeChunk(eatSound);
            Mix_CloseAudio();
                
            }
        }
    
    
    SDL_DestroyTexture(coverTexture);
    SDL_RenderClear(renderer);
  

    // Game loop variables
   
    std::vector<Snake> body;
    // Initialize snake with head, body, and tail segments
    body.push_back({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0}); // Head
    body.push_back({WINDOW_WIDTH / 2 - RECTANGLE_SIZE, WINDOW_HEIGHT / 2, 0.0}); // Body
    body.push_back({WINDOW_WIDTH / 2 - 2 * RECTANGLE_SIZE, WINDOW_HEIGHT / 2, 0.0}); // Tail
    srand(time(NULL));
    bool ateFood = false;
    int foodX = (rand() % (WINDOW_WIDTH / FOOD_SIZE)) * FOOD_SIZE;
    int foodY = (rand() % (WINDOW_HEIGHT / FOOD_SIZE)) * FOOD_SIZE;
    
    while (checkFoodOnSnake(body, foodX, foodY)) {
        foodX = (rand() % (WINDOW_WIDTH / FOOD_SIZE)) * FOOD_SIZE;
        foodY = (rand() % (WINDOW_HEIGHT / FOOD_SIZE)) * FOOD_SIZE;
    }
    int dx = RECTANGLE_SIZE;
    int dy = 0;
  
    bool gameover = false;
    int score = 0;
    int foodEatenCount = 0;
    bool bonusFoodActive = false;
    int bonusFoodX = 0;
    int bonusFoodY = 0;
    Uint32 bonusFoodStartTime = 0;
    auto currentMusic=startMusic;
    Mix_PlayMusic(currentMusic, -1);


    while (!quit) {
        handleEvents(e, dx, dy);
        
       if( !gameover && currentMusic!=gameBackground)
        {
            currentMusic=gameBackground;
             Mix_PlayMusic(currentMusic, -1);
        }
       
        if (!gameover) {
            // Move snake
            int nextX = body[0].x + dx;
            int nextY = body[0].y + dy;

            // Wrap snake position around screen edges
            if (nextX >= WINDOW_WIDTH) nextX = 0;
            else if (nextX < 0) nextX = WINDOW_WIDTH - RECTANGLE_SIZE;

            if (nextY >= WINDOW_HEIGHT) nextY = 0;
            else if (nextY < 0) nextY = WINDOW_HEIGHT - RECTANGLE_SIZE;

            // Check if snake eats regular food
            if (checkCollision(body[0], foodX, foodY)) {
                Mix_PlayChannel(-1, eatSound, 0);  // Play eat sound
                ateFood = true;
                foodX = (rand() % (WINDOW_WIDTH / FOOD_SIZE)) * FOOD_SIZE;
                foodY = (rand() % (WINDOW_HEIGHT / FOOD_SIZE)) * FOOD_SIZE;
                while (checkFoodOnSnake(body, foodX, foodY)) {
                    foodX = (rand() % (WINDOW_WIDTH / FOOD_SIZE)) * FOOD_SIZE;
                    foodY = (rand() % (WINDOW_HEIGHT / FOOD_SIZE)) * FOOD_SIZE;
                }
                
                // Increase score by 10
                score += 10;
                foodEatenCount++;
                // Activate bonus food after eating 5 regular food
                if (foodEatenCount % 5 == 0) {
                    bonusFoodActive = true;
                    Mix_PlayChannel(-1, bonusSound, 0); 
                    bonusFoodX = (rand() % (WINDOW_WIDTH / FOOD_SIZE)) * FOOD_SIZE;
                    bonusFoodY = (rand() % (WINDOW_HEIGHT / FOOD_SIZE)) * FOOD_SIZE;
                    while (checkFoodOnSnake(body, bonusFoodX, bonusFoodY)) {
                        bonusFoodX = (rand() % (WINDOW_WIDTH / FOOD_SIZE)) * FOOD_SIZE;
                        bonusFoodY = (rand() % (WINDOW_HEIGHT / FOOD_SIZE)) * FOOD_SIZE;
                    }
                    bonusFoodStartTime = SDL_GetTicks();
                }
            }

            // Check if snake eats bonus food
            if (bonusFoodActive && checkCollision(body[0], bonusFoodX, bonusFoodY)) {
                Mix_PlayChannel(-1, eatSound, 0);  // Play eat sound
                score += 50;
                bonusFoodActive = false;
            }

            if (bonusFoodActive && SDL_GetTicks() - bonusFoodStartTime > 5000) {
                //----------should add tong sound--------
                bonusFoodActive = false;
            }

            // Check if snake bites itself
            if (checkSelfBite(body)) {
                gameover = true;
            }

            // Update snake body
            if (!ateFood) {
                body.pop_back();
            } else {
                ateFood = false;
                // Add a new segment to the snake's tail
                Snake tail = body.back();
                body.push_back(tail);
            }
            body.insert(body.begin(), {nextX, nextY, atan2(dy, dx) * 180 / M_PI});
        }
        if(score>highScore){
            highScore=score;
            saveHighScore(filename,highScore);
        }

        // Render *************
         if(gameover && currentMusic!=gameOverMusic){
            currentMusic=gameOverMusic;
            Mix_FreeMusic(gameBackground);
            Mix_PlayMusic(currentMusic, -1);

        }
        renderGame(renderer, body, foodX, foodY, bonusFoodActive, bonusFoodX, bonusFoodY, score, gameover);
        SDL_Delay(SNAKE_SPEED);
    }
    // Clean up game over music
    Mix_FreeMusic(gameOverMusic);
    SDL_RenderPresent(renderer);
    }
}
void showHighScores(SDL_Renderer* renderer, TTF_Font* font, int highScore)
{
   
    SDL_Color color = {102, 102, 0, 0};
    std::string highScoreText =std::to_string(highScore);
    SDL_Surface* surface = TTF_RenderText_Solid(font, highScoreText.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    SDL_Rect dstrect = {390, 300, texW, texH};

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, highScoreBG, NULL, NULL);
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);

    
    SDL_RenderPresent(renderer);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
void highScoresLoop() {
    bool highScoresRunning = true;
    SDL_Event e;
    SDL_RenderClear(renderer);
   
    
    loadHighScore(filename, highScore);
    saveHighScore(filename, highScore);
    showHighScores(renderer, font, highScore);
    SDL_Rect dstRect = {390, 400, 50, 50};
    SDL_RenderCopy(renderer, returnIconTexture, NULL, &dstRect);
    
   
    while (highScoresRunning) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                highScoresRunning = false;
                //quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);

                    // Check if mouse is over any menu option
                    if (x >= 390 && x <= 440) {
                        if (y >= 400 && y <= 450) {
                            //currentState = GAME;
                            highScoresRunning = false;
                        } 
                    }
                }
            }
        }
       
        

        SDL_RenderPresent(renderer);
    }
}
void settingsLoop() {
    bool settingsRunning = true;
    SDL_Event e;
    SDL_RenderClear(renderer);
    /// add setting item-------
    SDL_Rect dstRect = {390, 400, 50, 50};
    SDL_RenderCopy(renderer, returnIconTexture, NULL, &dstRect);


    while (settingsRunning) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                settingsRunning = false;
                //quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);

                    // Check if mouse is over any menu option
                    if (x >= 390 && x <= 440) {
                        if (y >= 400 && y <= 450) {
                            //currentState = GAME;
                            settingsRunning = false;
                        } 
                    }
                }
            }
        }

        
        renderText(renderer, "Settings", 100, 100);
        renderText(renderer, "Press ESC to return to the main menu", 100, 200);
        SDL_RenderPresent(renderer);
    }
}
void coverSnakeLoop(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Event e;
    bool waiting = true;

    // Clear screen
    SDL_RenderClear(renderer);
    // Render cover snake screen (You can add more graphics if needed)
   SDL_RenderCopy(renderer, coverTexture, NULL, NULL);
    SDL_RenderPresent(renderer);

    // Wait for any key press
    while (waiting) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN || e.type == SDL_QUIT) {
                waiting = false;
            }
        }
    }
}

void exitLoop(){

}
int main(int argc, char* args[]) {
    
   initialize();
   initializeTextures();
   initializeSounds();
    
    Mix_PlayMusic(startMusic, -1);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, coverTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    coverSnakeLoop(renderer, font);

       while (currentState != QUIT) {
        switch (currentState) {
            case MAIN_MENU:
                mainMenuLoop(renderer);
                break;
            case GAME:
                gameLoop();
                currentState = MAIN_MENU;
                break;
            case HIGH_SCORES:
                highScoresLoop();
                currentState = MAIN_MENU;
                break;
            case SETTINGS:
                settingsLoop();
                currentState = MAIN_MENU;
                break;
            case EXIT:
                exitLoop();
                currentState = MAIN_MENU;
                break;
            default:
                break;
        }
    }

    
   
    // Cleanup
    cleanUp(window, renderer, font,startMusic, {headTexture, bodyTexture, tailTexture, backgroundTexture, foodTexture, bonusFoodTexture});
    
    return 0;
}