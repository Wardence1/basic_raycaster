#include <iostream>
#include <eigen3/Eigen/Core>
#include <SDL2/SDL.h>

#define WIN_WIDTH 1080
#define WIN_HEIGHT 720

#define FPS 60


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
	while (running) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				running = false;
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
		SDL_RenderPresent(ren);

	}

	SDL_DestroyRenderer(ren);	
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}

// @test: make sure this works
Eigen::Vector2d rotate(Eigen::Vector2d vec, double rads) {
	
	Eigen::Matrix2d rotMat;
	rotMat << std::cos(rads), std::sin(rads),
	       -std::sin(rads), std::cos(rads);

	return rotMat * vec; 
}
