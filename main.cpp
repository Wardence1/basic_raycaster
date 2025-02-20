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
#define COLUMN_WIDTH WIN_WIDTH/COLUMNS

#define FPS 60

#define DEGREE 0.0174533

bool debug = false;
int accuracy = 300;

int map[MAP_HEIGHT][MAP_WIDTH] = {
	{1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,2,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,2,0,0,0,0,1},
	{1,0,0,0,0,0,1,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,1,0,0,0,0,2},
	{1,0,0,0,0,0,1,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,2,0,1},
	{1,2,0,0,0,0,0,0,0,0,2,1},
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
char getSide(Eigen::Vector2d rayPos, Eigen::Vector2d dir);
void lowerColor(SDL_Renderer* ren, Uint8 lower);

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
    double renderDis = 20*accuracy;

	while (running) {
		while (SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT:
					running = false;
					break;
				case SDL_KEYDOWN:
					switch(e.key.keysym.sym) {
						case SDLK_SPACE:
							debug = !debug;
							break;
						case SDLK_ESCAPE:
							running = false;
							break;
					}
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

			/* DRAW FIRST PERSON ROOF AND FLOOR */
			if (!debug) {
				SDL_RenderClear(ren);

				SDL_Rect roof;
				roof.x = 0;
				roof.y = 0;
				roof.w = WIN_WIDTH;
				roof.h = WIN_HEIGHT/2;
				SDL_SetRenderDrawColor(ren, 82, 82, 82, 255);
				SDL_RenderFillRect(ren, &roof);

				SDL_Rect floor;
				floor.x = 0;
				floor.y = WIN_HEIGHT/2;
				floor.w = WIN_WIDTH;
				floor.h = WIN_HEIGHT/2;
				SDL_SetRenderDrawColor(ren, 142, 142, 142, 255);
				SDL_RenderFillRect(ren, &floor);

				SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
			}

            /* CAST THE RAYS */
			for (int i = 0; i < COLUMNS; i++) {

            	std::pair<double, double> rayPos = {pos.x(), pos.y()};

				Eigen::Vector2d rayDir;

				rayDir(0) = rotate(dir, (DEGREE*i) - DEGREE * (COLUMNS/2)).x();
  				rayDir(1) = rotate(dir, (DEGREE*i) - DEGREE * (COLUMNS/2)).y();

				for (int j = 0; j <= renderDis; j++) {

					// @optimize: This checks for wall collisions at every few pixels of the line, check at every tile edge instead.
					rayPos.first += rayDir.x()/accuracy;
					rayPos.second += rayDir.y()/accuracy;

					if ((int(rayPos.first) > MAP_WIDTH || int(rayPos.first) < 0) ||
						(int(rayPos.second) > MAP_HEIGHT || int(rayPos.second) < 0)) {

						perror("Ray went out of bounds\n");
						return 1;
					}

					if (map[int(rayPos.second)][int(rayPos.first)] > 0) {


						///////////////////////
						// DRAW FIRST PERSON //
						///////////////////////

						if (!debug) {

							double distance = sqrt(pow(rayPos.first - pos.x(), 2) + pow(rayPos.second - pos.y(), 2));
							distance *= dir.dot(rayDir); // fix fisheye
							int type = map[int(rayPos.second)][int(rayPos.first)];

							switch(type) {
								case 1:
									SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
									break;
								case 2:
									SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
									break;
								default:
									fprintf(stderr, "Invalid wall type: %d\n", type);
									return 1;
							}

							switch (getSide(Eigen::Vector2d(rayPos.first, rayPos.second), rayDir)) {
								case 'b':
									lowerColor(ren, 80);
									break;
								case 't':
									lowerColor(ren, 80);
									break;
							}

							SDL_Rect column;
							column.x = i * COLUMN_WIDTH;
							column.w = COLUMN_WIDTH + 1;
							column.h = WIN_HEIGHT / distance;
							column.y = WIN_HEIGHT/2 - column.h/2;
							SDL_RenderFillRect(ren, &column);

						} else {

							rays[i] = {pos.x(), pos.y(), rayPos.first, rayPos.second};
						}

						break;
					}
				}
			}
        }
			
		////////////////
		// DRAW DEBUG //
		////////////////

		if (debug) {
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

					switch (map[y][x]) {
						case 1:
							SDL_SetRenderDrawColor(ren, 0, 0, 122, 255);
							break;
						case 2:
							SDL_SetRenderDrawColor(ren, 122, 0, 0, 255);
							break;
						default:
							SDL_SetRenderDrawColor(ren, 22, 22, 22, 255);
							break;
					}
						SDL_RenderFillRect(ren, &rect);
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

		}

		SDL_RenderPresent(ren);
	}

	SDL_DestroyRenderer(ren);	
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}

// @todo: this should be done differently when checking on each tile frame
// Returns a char based on the colliding side: b, t, l, r
char getSide(Eigen::Vector2d rayPos, Eigen::Vector2d dir) {

	double tileX = floor(rayPos.x());
	double tileY = floor(rayPos.y());

	if (rayPos.y() - tileY < 0.005 && !map[int(tileY)-1][int(tileX)]) {
		return 'b';
	} else if (rayPos.y() - tileY > 0.995 && !map[int(tileY)+1][int(tileX)]) {
		return 't';
	}

	if (rayPos.x() - tileX < 0.01) {
		return 'l';
	} else if (rayPos.x() - tileX > 0.99) {
		return 'r';
	}

	return -1;
}

void lowerColor(SDL_Renderer* ren, Uint8 lower) {

	Uint8 r, g, b, a;
	SDL_GetRenderDrawColor(ren, &r, &g, &b, &a);
	SDL_SetRenderDrawColor(ren, std::max(r-lower, 0), std::max(g-lower, 0), std::max(b-lower, 0), 255);
}

Eigen::Vector2d rotate(Eigen::Vector2d vec, double rads) {
	
	Eigen::Matrix2d rotMat;
	rotMat << std::cos(rads), -std::sin(rads),
	       std::sin(rads), std::cos(rads);
	
	return rotMat * vec; 
}