#include <iostream>
#include <eigen3/Eigen/Core>
#include <SDL2/SDL.h>

#define MAP_WIDTH 12
#define MAP_HEIGHT 12
#define TILE_SIZE 64
#define WIN_WIDTH TILE_SIZE*MAP_WIDTH
#define WIN_HEIGHT TILE_SIZE*MAP_HEIGHT

#define FPS 60

int map[MAP_HEIGHT][MAP_WIDTH] = {
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,1,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,1,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,1,0,0,0,0,0,0,0},
};

Eigen::Vector2d rotate(Eigen::Vector2d vec, double rads);

int main() {

	if (SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "Unable to initialize SDL2:" << SDL_GetError() << "\n";	
		return 1;
	}

	SDL_Window* win = SDL_CreateWindow("Raycaster", 100, 100, WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN);

	if (!win) {
		std::cerr << "SDL2 Window error:" << SDL_GetError() << "\n";	
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (!ren) {
		std::cerr << "SDL2 Renderer error:" << SDL_GetError() << "\n";
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;
	}

	bool running = true;
	Uint32 currTime;
	Uint32 lastTime;
	Uint32 acc = 0;
	long ticks = 0;
	SDL_Event e;

	Eigen::Vector2d pos(50, 50);
	Eigen::Vector2d dir(1, 0);
	Eigen::Vector2d plane(0, .6);
	double turnSp = .2;
	double lineLen = 26;

	while (running) {
		while (SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT:
					running = false;
					break;
				case SDLK_LEFT: // @todo: input and turning
					rotate(pos, turnSp);
					break;
			}
		}

		lastTime = SDL_GetTicks();
		SDL_Delay(1);
		currTime = SDL_GetTicks();
		acc += currTime - lastTime;
		
		////////////
		// UPDATE //
		////////////
		if (acc >= 1000 / FPS) {
			acc = 0;
			ticks++;

				
		}

			
		//////////
		// DRAW //
		//////////
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
		SDL_RenderClear(ren);

		SDL_Rect rect; // multi-purpose rect
		// Grid
		for (int x = 0; x < MAP_WIDTH; x++) {
			for (int y = 0; y < MAP_HEIGHT; y++) {
				rect.x = x * TILE_SIZE;
				rect.y = y * TILE_SIZE;
				rect.w = TILE_SIZE - 2;
				rect.h = TILE_SIZE - 2;
		
				if (map[y][x] == 1) {
					SDL_SetRenderDrawColor(ren, 0, 0, 122, 255);
					SDL_RenderFillRect(ren, &rect);
				} else {
					SDL_SetRenderDrawColor(ren, 22, 22, 22, 255);
					SDL_RenderFillRect(ren, &rect);
				}
			}
		}

		// Player		
		rect.w = 8;
		rect.h = 8;
		rect.x = pos.x()-rect.w/2;
		rect.y = pos.y()-rect.h/2;

		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		SDL_RenderFillRect(ren, &rect);
		// Line
		SDL_RenderDrawLine(ren, (int)pos.x(), (int)pos.y(),
								(int)(pos.x()+dir.x()*lineLen), (int)(pos.y()+dir.y()*lineLen));
				

		SDL_RenderPresent(ren);
	}

	SDL_DestroyRenderer(ren);	
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}

Eigen::Vector2d rotate(Eigen::Vector2d vec, double rads) {
	
	Eigen::Matrix2d rotMat;
	rotMat << std::cos(rads), -std::sin(rads),
	       std::sin(rads), std::cos(rads);
	
	return rotMat * vec; 
}
