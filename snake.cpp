#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <string>

//  ......................
const int WINDOW_WIDTH = 854;
const int WINDOW_HEIGHT = 580;
const int RECTANGLE_SIZE = 20;
const int FOOD_SIZE = 20;
const int SNAKE_SPEED = 150;

struct Snake {
    int x, y;
    double angle; // Angle for rotation
};

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

// Function to ensure food doesn't spawn on the snake's body
bool checkFoodOnSnake(const std::vector<Snake>& body, int x, int y) {
    for (const auto& segment : body) {
        if (segment.x == x && segment.y == y) {
            return true;
        }
    }
    return false;
}

// Function to load texture and check for errors
SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Unable to load image! SDL_image Error: " << IMG_GetError() << " for image: " << path << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// Function to clean up and quit SDL
void cleanUp(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font, std::vector<SDL_Texture*> textures) {
    for (auto texture : textures) {
        SDL_DestroyTexture(texture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* args[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("font.ttf", 35);
    if (font == NULL) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load cover photo
    SDL_Texture* coverTexture = loadTexture(renderer, "snake_cover.png");
    if (!coverTexture) {
        cleanUp(window, renderer, font, {});
        return 1;
    }

    // Show cover photo
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, coverTexture, NULL, NULL);


    SDL_RenderPresent(renderer);

    // Wait for a key press to start the game
    bool startGame = false;
    SDL_Event e;
    while (!startGame) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                cleanUp(window, renderer, font, {coverTexture});
                return 0;
            } else if (e.type == SDL_KEYDOWN) {
                startGame = true;
            }
        }
    }

    SDL_DestroyTexture(coverTexture);

    // Load snake head, body, and tail images
    SDL_Texture* headTexture = loadTexture(renderer, "snake_head.png");
    if (!headTexture) {
        cleanUp(window, renderer, font, {});
        return 1;
    }

    SDL_Texture* bodyTexture = loadTexture(renderer, "snake_body.png");
    if (!bodyTexture) {
        cleanUp(window, renderer, font, {headTexture});
        return 1;
    }

    SDL_Texture* tailTexture = loadTexture(renderer, "snake_tail.png");
    if (!tailTexture) {
        cleanUp(window, renderer, font, {headTexture, bodyTexture});
        return 1;
    }

    // Load background image
    SDL_Texture* backgroundTexture = loadTexture(renderer, "snake_background.png");
    if (!backgroundTexture) {
        cleanUp(window, renderer, font, {headTexture, bodyTexture, tailTexture});
        return 1;
    }

    // Load food image
    SDL_Texture* foodTexture = loadTexture(renderer, "food.png");
    if (!foodTexture) {
        cleanUp(window, renderer, font, {headTexture, bodyTexture, tailTexture, backgroundTexture});
        return 1;
    }

    // Load bonus food image
    SDL_Texture* bonusFoodTexture = loadTexture(renderer, "bonus_food.png");
    if (!bonusFoodTexture) {
        cleanUp(window, renderer, font, {headTexture, bodyTexture, tailTexture, backgroundTexture, foodTexture});
        return 1;
    }

    // Game loop variables
    bool quit = false;
    std::vector<Snake> body;
    // Initialize snake with head, body, and tail segments
    body.push_back({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0}); // Head
    body.push_back({WINDOW_WIDTH / 2 - RECTANGLE_SIZE, WINDOW_HEIGHT / 2, 0.0}); // Body
    body.push_back({WINDOW_WIDTH / 2 - 2 * RECTANGLE_SIZE, WINDOW_HEIGHT / 2, 0.0}); // Tail
    srand(time(NULL));
    int foodX = (rand() % (WINDOW_WIDTH / FOOD_SIZE)) * FOOD_SIZE;
    int foodY = (rand() % (WINDOW_HEIGHT / FOOD_SIZE)) * FOOD_SIZE;
    while (checkFoodOnSnake(body, foodX, foodY)) {
        foodX = (rand() % (WINDOW_WIDTH / FOOD_SIZE)) * FOOD_SIZE;
        foodY = (rand() % (WINDOW_HEIGHT / FOOD_SIZE)) * FOOD_SIZE;
    }
    int dx = RECTANGLE_SIZE;
    int dy = 0;
    bool ateFood = false;
    bool gameover = false;

    // Initialize score
    int score = 0;
    int foodEatenCount = 0;
    bool bonusFoodActive = false;
    int bonusFoodX = 0;
    int bonusFoodY = 0;
    Uint32 bonusFoodStartTime = 0;

    // Main game loop
    while (!quit) {
        // Handle events
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
                // Activate bonus food after eating 10 regular food
                if (foodEatenCount % 10 == 0) {
                    bonusFoodActive = true;
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
                score += 50;
                bonusFoodActive = false;
            }

            // Deactivate bonus food after 3 seconds
            if (bonusFoodActive && SDL_GetTicks() - bonusFoodStartTime > 5000) {
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

        // Render
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
            SDL_Color textColor = {255, 0, 0, 255}; // Red color for text
            SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, "YOUR GAME IS OVER", textColor);
            SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
            SDL_Rect messageRect;
            messageRect.x = WINDOW_WIDTH / 2 - surfaceMessage->w / 2;
            messageRect.y = WINDOW_HEIGHT / 2 - surfaceMessage->h / 2;
            messageRect.w = surfaceMessage->w;
            messageRect.h = surfaceMessage->h;
            SDL_RenderCopy(renderer, message, NULL, &messageRect);
            SDL_FreeSurface(surfaceMessage);
            SDL_DestroyTexture(message);
            // ...................
         std::string finalScoreText = "SCORE : " + std::to_string(score);
        SDL_Surface* finalScoreSurface = TTF_RenderText_Solid(font, finalScoreText.c_str(), textColor);
        SDL_Texture* finalScoreTexture = SDL_CreateTextureFromSurface(renderer, finalScoreSurface);
        SDL_Rect finalScoreRect;
        finalScoreRect.x = WINDOW_WIDTH / 2 - finalScoreSurface->w / 2;
        finalScoreRect.y = messageRect.y + messageRect.h + 10; // 10 pixels below the "GAME OVER" message
        finalScoreRect.w = finalScoreSurface->w;
        finalScoreRect.h = finalScoreSurface->h;
        SDL_RenderCopy(renderer, finalScoreTexture, NULL, &finalScoreRect);
        SDL_FreeSurface(finalScoreSurface);
        SDL_DestroyTexture(finalScoreTexture);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(SNAKE_SPEED);
    }

    // Cleanup
    cleanUp(window, renderer, font, {headTexture, bodyTexture, tailTexture, backgroundTexture, foodTexture, bonusFoodTexture});

    return 0;
}