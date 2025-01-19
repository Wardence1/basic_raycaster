#include <iostream>
#include <eigen3/Eigen/Core>
#include <SDL2/SDL.h>
#include <cmath>

#define MAP_WIDTH 12
#define MAP_HEIGHT 12
#define FAKE_TILE_SIZE 64 // for drawing to the screen, real tile size is 1
#define WIN_WIDTH FAKE_TILE_SIZE*MAP_WIDTH
#define WIN_HEIGHT FAKE_TILE_SIZE*MAP_HEIGHT
#define COLUMNS 66

#define FPS 60

#define DEGREE 0.0174533

int map[MAP_HEIGHT][MAP_WIDTH] = {
	{1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1,0,0,0,1},
	{1,1,0,0,0,0,0,1,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,1,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1},
};

typedef struct {
    double x1;
    double y1;
    double x2;
    double y2;
} Line;

Line rays[COLUMNS];

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

	Eigen::Vector2d pos(3, 3);
	Eigen::Vector2d dir(1, 0);
	Eigen::Vector2d plane(0, .6);
	double movSp = .05;
	double turnSp = .1;
	double lineLen = 26;
    double renderDis = 20*100;

	while (running) {
		while (SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT:
					running = false;
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

            std::pair<int, int> mapPos(int(pos.x()), int(pos.y()));
		
			/* INPUT */
			auto keyS = SDL_GetKeyboardState(NULL);

            // @todo: Delta Time???
            // @todo: When the player collides with a wall, move them out

			if (keyS[SDL_SCANCODE_W]) {
				if (map[mapPos.second][int(pos.x() + dir.x() * movSp)] < 1) pos(0) += dir.x() * movSp;					
				if (map[int(pos.y() + dir.y() * movSp)][mapPos.first] < 1) pos(1) += dir.y() * movSp;					
			}
			if (keyS[SDL_SCANCODE_A]) {
                if (map[mapPos.second][int(pos.x() - rotate(dir, M_PI/2).x() * movSp)] < 1) pos(0) -= rotate(dir, M_PI/2).x() * movSp;
                if (map[int(pos.y() - rotate(dir, M_PI/2).y() * movSp)][mapPos.first] < 1) pos(1) -= rotate(dir, M_PI/2).y() * movSp;
			}
			if (keyS[SDL_SCANCODE_S]) {
                if (map[mapPos.second][int(pos.x() - dir.x() * movSp)] < 1) pos(0) -= dir.x() * movSp;					
				if (map[int(pos.y() - dir.y() * movSp)][mapPos.first] < 1) pos(1) -= dir.y() * movSp;	
			}
			if (keyS[SDL_SCANCODE_D]) {
				if (map[mapPos.second][int(pos.x() + rotate(dir, M_PI/2).x() * movSp)] < 1) pos(0) += rotate(dir, M_PI/2).x() * movSp;
                if (map[int(pos.y() + rotate(dir, M_PI/2).y() * movSp)][mapPos.first] < 1) pos(1) += rotate(dir, M_PI/2).y() * movSp;
			}

			if (keyS[SDL_SCANCODE_LEFT]) {
				dir = rotate(dir, -turnSp);
			}
			if (keyS[SDL_SCANCODE_RIGHT]) {
				dir = rotate(dir, turnSp);
			}

            /* CAST THE RAYS */
			for (int i = 0; i < COLUMNS; i++) {

            	std::pair<double, double> rayPos = {pos.x(), pos.y()};

				Line l = {
					pos.x(), pos.y(),
					(pos.x() + dir.x() * renderDis),
					(pos.y() + dir.y() * renderDis),
				};

            	double dx = l.x2 - l.x1;
            	double dy = l.y2 - l.y1;

				dx = dir.x() + DEGREE * i;
  				dy = dir.y() + DEGREE * i;

				for (int j = 0; j <= renderDis; j++) {

					// @optimize: This checks for wall collisions at every few pixels of the line, check at every tile edge instead.
					rayPos.first += dx/100;
					rayPos.second += dy/100;

					if ((int(rayPos.first) > MAP_WIDTH || int(rayPos.first) < 0) ||
						(int(rayPos.second) > MAP_HEIGHT || int(rayPos.second) < 0)) {

						rays[i] = {l.x1, l.y1, l.x2, l.y2};
						break;
					}

					if (map[int(rayPos.second)][int(rayPos.first)] > 0) {

						rays[i] = {l.x1, l.y1, rayPos.first, rayPos.second};
						break;
					}
				}
			}
        }
			
		//////////
		// DRAW //
		//////////
		Eigen::Vector2d screenPos(pos*FAKE_TILE_SIZE);
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
		SDL_RenderClear(ren);

		SDL_Rect rect; // multi-purpose rect
		// Grid
		for (int x = 0; x < MAP_WIDTH; x++) {
			for (int y = 0; y < MAP_HEIGHT; y++) {
				rect.x = x * FAKE_TILE_SIZE;
				rect.y = y * FAKE_TILE_SIZE;
				rect.w = FAKE_TILE_SIZE - 2;
				rect.h = FAKE_TILE_SIZE - 2;
		
				if (map[y][x] == 1) {
					SDL_SetRenderDrawColor(ren, 0, 0, 122, 255);
					SDL_RenderFillRect(ren, &rect);
				} else {
					SDL_SetRenderDrawColor(ren, 22, 22, 22, 255);
					SDL_RenderFillRect(ren, &rect);
				}
			}
		}

		// Rays
		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		for (int i = 0; i < COLUMNS; i++) {
			SDL_RenderDrawLine(ren, rays[i].x1*FAKE_TILE_SIZE, rays[i].y1*FAKE_TILE_SIZE, rays[i].x2*FAKE_TILE_SIZE, rays[i].y2*FAKE_TILE_SIZE);
		}

		// Player
		rect.w = 8;
		rect.h = 8;
		rect.x = screenPos.x()-rect.w/2;
		rect.y = screenPos.y()-rect.h/2;
		SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
		SDL_RenderFillRect(ren, &rect);

		// Line
		SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
		SDL_RenderDrawLine(ren, int(screenPos.x()), int(screenPos.y()),
								int((screenPos.x()+dir.x()*lineLen)), (int(screenPos.y()+dir.y()*lineLen)));


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