#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include "Platform.h"

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 600;

struct Vector2 {
    float x, y;
};


// Các class khác giữ nguyên như trước (Platform, MovingPlatform, Coin, Spike, Door, Key, Hero, Spider)

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    Hero* hero;
    std::vector<Platform> *platforms;
    std::vector<MovingPlatform*> movingPlatforms;
    std::vector<Coin*> coins;
    std::vector<Spider*> spiders;
    std::vector<Spike*> spikes;
    Door* door;
    Key* key;
    bool running = true;
    int coinCount = 0;
    bool hasKey = true;

public:
    Game() {
        class Sprite {
        protected:
            SDL_Texture* texture;
            SDL_Rect rect;
            Vector2 velocity = { 0.0f, 0.0f };
            bool onGround = false;

        public:
            Sprite(SDL_Renderer* renderer, const std::string& filepath, int x, int y, int w, int h) {
                SDL_Surface* surface = IMG_Load(filepath.c_str());
                if (!surface) {
                    std::string error = "Failed to load image " + filepath + ": " + SDL_GetError();
                    throw std::runtime_error(error);
                }
                texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (!texture) {
                    SDL_FreeSurface(surface);
                    throw std::runtime_error("Failed to create texture: " + std::string(SDL_GetError()));
                }
                SDL_FreeSurface(surface);

                rect.x = x;
                rect.y = y;
                rect.w = w;
                rect.h = h;
            }

            virtual ~Sprite() {
                if (texture) SDL_DestroyTexture(texture);
            }

            virtual void update(float dt) = 0;
            void render(SDL_Renderer* renderer) {
                SDL_RenderCopy(renderer, texture, nullptr, &rect);
            }

            SDL_Rect* getRect() { return &rect; }  // Sửa lỗi cú pháp
            Vector2* getVelocity() { return &velocity; }
            bool isOnGround() { return onGround; }
            void setOnGround(bool value) { onGround = value; }
        };

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error("SDL Init failed: " + std::string(SDL_GetError()));
        }

        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            SDL_Quit();
            throw std::runtime_error("IMG Init failed: " + std::string(IMG_GetError()));
        }

        window = SDL_CreateWindow("Platformer", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN);
        if (!window) {
            IMG_Quit();
            SDL_Quit();
            throw std::runtime_error("Window creation failed: " + std::string(SDL_GetError()));
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            SDL_DestroyWindow(window);
            IMG_Quit();
            SDL_Quit();
            throw std::runtime_error("Renderer creation failed: " + std::string(SDL_GetError()));
        }

        try {
            loadLevel();
        }
        catch (const std::exception& e) {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            IMG_Quit();
            SDL_Quit();
            throw; // Ném lại ngoại lệ để main xử lý
        }
    }

    // Phần còn lại của class Game giữ nguyên (loadLevel, handleInput, update, render, run)

    ~Game() {
        delete hero;
        for (auto p : platforms) delete p;
        for (auto m : movingPlatforms) delete m;
        for (auto c : coins) delete c;
        for (auto s : spiders) delete s;
        for (auto s : spikes) delete s;
        delete door;
        delete key;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
    }

    void loadLevel() {
        // Platforms
        platforms.push_back(new Platform(renderer, "ground", -700, 470));
        platforms.push_back(new Platform(renderer, "ground", 350, 470));
        platforms.push_back(new Platform(renderer, "dirt", 0, 0));
        platforms.push_back(new Platform(renderer, "dirt", 0, 72));
        platforms.push_back(new Platform(renderer, "dirt", -700, 546));
        platforms.push_back(new Platform(renderer, "dirt", 350, 546));

        // Moving platform
        movingPlatforms.push_back(new MovingPlatform(renderer, 265, 470, "right"));

        // Coins
        int coinPosX[] = { 231, 273, 315, 357, 819, 861, 903, 945, 399, 357, 336, 777, 819, 861, 903, 945, 189, 231, 525, 567, 609, 651, 819, 861 };
        int coinPosY[] = { 524, 524, 524, 524, 524, 524, 524, 524, 294, 315, 357, 357, 357, 357, 357, 357, 231, 231, 147, 147, 147, 147, 63, 63 };
        for (int i = 0; i < 24; i++) {
            coins.push_back(new Coin(renderer, coinPosX[i], coinPosY[i]));
        }

        // Spikes
        int spikePosX[] = { 350, 300, 420, 370, 520, 570, 620, 670 };
        int spikePosY[] = { 570, 570, 350, 350, 350, 350, 350, 350 };
        for (int i = 0; i < 8; i++) {
            spikes.push_back(new Spike(renderer, spikePosX[i], spikePosY[i]));
        }

        // Hero, Spider, Door, Key
        hero = new Hero(renderer, 21, 445);
        spiders.push_back(new Spider(renderer, 300, 455));
        door = new Door(renderer, 800, 470);
        key = new Key(renderer, 903, 105);
    }

    void handleInput() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_w:
                case SDLK_UP:
                case SDLK_SPACE:
                    hero->jump();
                    break;
                }
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) hero->move(-1);
        else if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) hero->move(1);
        else hero->move(0);
    }

    void update(float dt) {
        hero->update(dt);

        for (auto m : movingPlatforms) m->update(dt);
        for (auto s : spiders) s->update(dt);

        // Platform collision
        bool onPlatform = false;
        for (auto p : platforms) {
            if (SDL_HasIntersection(hero->getRect(), p->getRect()) &&
                hero->getVelocity()->y >= 0 &&
                hero->getRect()->y + hero->getRect()->h - p->getRect()->y <= 10) {
                hero->getRect()->y = p->getRect()->y - hero->getRect()->h;
                hero->setOnGround(true);
                hero->getVelocity()->y = 0;
                onPlatform = true;
            }
        }
        for (auto m : movingPlatforms) {
            if (SDL_HasIntersection(hero->getRect(), m->getRect()) &&
                hero->getVelocity()->y >= 0 &&
                hero->getRect()->y + hero->getRect()->h - m->getRect()->y <= 10) {
                hero->getRect()->y = m->getRect()->y - hero->getRect()->h;
                hero->setOnGround(true);
                hero->getVelocity()->y = 0;
                hero->getRect()->x += static_cast<int>(m->getVelocity()->x * dt);
                onPlatform = true;
            }
        }
        if (!onPlatform && hero->getRect()->y + hero->getRect()->h < SCREEN_HEIGHT) {
            hero->setOnGround(false);
        }

        // Coin collection
        for (auto c : coins) {
            if (!c->isCollected() && SDL_HasIntersection(hero->getRect(), c->getRect())) {
                c->collect();
                coinCount++;
            }
        }

        // Spider collision
        for (auto s : spiders) {
            if (s->isAlive() && SDL_HasIntersection(hero->getRect(), s->getRect())) {
                if (hero->getVelocity()->y > 0) {
                    s->die();
                    hero->jump();
                }
            }
        }

        // Spike collision
        for (auto s : spikes) {
            if (SDL_HasIntersection(hero->getRect(), s->getRect())) {
                hero->getRect()->x = 21 - 18;
                hero->getRect()->y = 445 - 21;
                hero->getVelocity()->y = 0;
            }
        }

        // Door collision
        if (hasKey && SDL_HasIntersection(hero->getRect(), door->getRect())) {
            std::cout << "Level Complete! Coins: " << coinCount << std::endl;
            running = false;
        }

        // Key collision
        if (!hasKey && SDL_HasIntersection(hero->getRect(), key->getRect())) {
            hasKey = true;
            key->getRect()->x = -100;
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (auto p : platforms) p->render(renderer);
        for (auto m : movingPlatforms) m->render(renderer);
        for (auto c : coins) if (!c->isCollected()) c->render(renderer);
        for (auto s : spikes) s->render(renderer);
        door->render(renderer);
        if (!hasKey) key->render(renderer);
        hero->render(renderer);
        for (auto s : spiders) if (s->isAlive()) s->render(renderer);

        SDL_RenderPresent(renderer);
    }

    void run() {
        Uint32 lastTime = SDL_GetTicks();
        while (running) {
            Uint32 currentTime = SDL_GetTicks();
            float dt = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;

            handleInput();
            update(dt);
            render();
        }
    }
};

int main(int argc, char* argv[]) {
    try {
        Game game;
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}