#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}


#define SCREEN_WIDTH 640 //piksele
#define SCREEN_HEIGHT 480 //piksele
#define TILE_SIZE 32 //piksele
#define PLATFORM 0
#define LADDER 1
#define INFO 2
#define JUMP_HEIGHT 60 //piksele
#define OBJ_COUNT 15 //ilosc obiektow na planszy
#define NULL_OBJECT { {-1}, {-1}, {-1}, {-1} }
#define SPEED 100
#define JUMP_SPEED 150
#define GRAVITY 80
#define OFFSET 1 //piksele
#define INFO_COLOR SDL_MapRGB(game->screen->format, 17, 17, 204) //niebieski
#define BACKGROUND_COLOR SDL_MapRGB(game->screen->format, 0, 0, 0) //czarny
#define LADDER_PARAMS LADDER, 1, 3
#define START {50, 200} //punkt poczatkowy gracza


typedef struct point_t {
	double x, y;
};
typedef struct object_t {
	point_t start;
	int type, width, height;
};
typedef struct player_t {
	point_t coords;
	int width, height;
	bool jumping, on_ground, on_ladder;
	double speedX, speedY;
};
typedef struct game_t {
	double gravity;
	object_t objects[OBJ_COUNT];
	player_t player;
	SDL_Event event;
	SDL_Surface* platform, * character, * ladder, * screen, * charset;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
};


void restart_game(game_t* game, double* worldTime) {
	game->player.coords = START;
	*worldTime = 0;
}


void draw_string(game_t* game, const char* text, int x, int y) {
	SDL_Rect destination;
	destination.x = x;
	destination.y = y;

	int char_width = 8;
	while (*text) {
		int c = *text & 255;
		destination.w = char_width;
		destination.h = char_width;

		SDL_Rect src = {
			(c % 16) * char_width,
			(c / 16) * char_width,
			char_width,
			char_width,
		};

		SDL_BlitSurface(game->charset, &src, game->screen, &destination);

		destination.x += char_width;
		text++;
	}

};
void draw_obj(game_t* game, int object_idx) {
	SDL_Rect rectangle = {
		game->objects[object_idx].start.x,
		game->objects[object_idx].start.y,
		game->objects[object_idx].width,
		game->objects[object_idx].height
	};
	switch (game->objects[object_idx].type) {
	case INFO:
		SDL_FillRect(game->screen, &rectangle, INFO_COLOR);
		break;
	case PLATFORM:
		for (int y = 0; y < game->objects[object_idx].height; ++y) {
			for (int x = 0; x < game->objects[object_idx].width; ++x) {
				SDL_Rect rectangle = {
					game->objects[object_idx].start.x + x * TILE_SIZE,
					game->objects[object_idx].start.y + y * TILE_SIZE,
					TILE_SIZE,
					TILE_SIZE
				};
				SDL_BlitSurface(game->platform, NULL, game->screen, &rectangle);
			}
		}
		break;
	case LADDER:
		SDL_BlitSurface(game->ladder, NULL, game->screen, &rectangle);
		break;
	}
}
void draw_player(game_t* game) {
	SDL_Rect rectangle = {
		game->player.coords.x,
		game->player.coords.y,
		game->player.width,
		game->player.height
	};
	SDL_BlitSurface(game->character, NULL, game->screen, &rectangle);
}


object_t get_object(object_t objects[OBJ_COUNT], double x, double y) {
	for (int i = 1; i < OBJ_COUNT; i++) {
		SDL_Rect rect = {
			objects[i].start.x,
			objects[i].start.y,
			TILE_SIZE * objects[i].width,
			TILE_SIZE * objects[i].height
		};

		if (x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h) {
			return objects[i];
		}
	}
	return NULL_OBJECT;
}
bool check_if_on_ladder(game_t* game, double x, double y) {
	for (int i = 1; i < OBJ_COUNT; i++) {
		if (game->objects[i].type == LADDER) {
			SDL_Rect rect = {
				game->objects[i].start.x,
				game->objects[i].start.y,
				TILE_SIZE * game->objects[i].width,
				TILE_SIZE * game->objects[i].height
			};
			SDL_Rect player = {
				x,
				y,
				game->player.width,
				game->player.height
			};
			if (SDL_HasIntersection(&rect, &player)) return true;
		}
	}
	return false;
}
bool check_if_over_ladder(game_t* game) {
	return check_if_on_ladder(game, game->player.coords.x, game->player.coords.y + 2 * TILE_SIZE + game->player.height);
}


int handle_inputs(game_t* game, double* worldTime) {
	SDL_Keycode key;
	while (SDL_PollEvent(&game->event)) {
		key = game->event.key.keysym.sym;
		switch (game->event.type) {
		case SDL_KEYDOWN:
			if (key == SDLK_ESCAPE) return 1;
			if (key == SDLK_n) restart_game(game, worldTime);
			if (game->player.on_ladder) {
				if (key == SDLK_UP) game->player.speedY = -SPEED;
				if (key == SDLK_DOWN) game->player.speedY = SPEED;
			}
			else {
				if (key == SDLK_UP &&
					(game->player.on_ground && check_if_on_ladder(game, game->player.coords.x, game->player.coords.y))) {
					game->player.speedY = -SPEED;
				}
				if (key == SDLK_DOWN &&
					(check_if_over_ladder(game) && game->player.on_ground)) {
					game->player.speedY = SPEED;
				}
			}
			if (!check_if_on_ladder(game, game->player.coords.x, game->player.coords.y) ||
				(game->player.on_ladder && game->player.on_ground)) {
				if (key == SDLK_LEFT) game->player.speedX = -SPEED;
				if (key == SDLK_RIGHT) game->player.speedX = SPEED;

				if ((game->player.on_ground && !game->player.on_ladder) && key == SDLK_SPACE) {
					game->player.speedY = -JUMP_SPEED;
					game->player.jumping = true;
				}
			}
			break;
		case SDL_KEYUP:
			if (key == SDLK_LEFT || key == SDLK_RIGHT) game->player.speedX = 0;
			if (!game->player.jumping) game->player.speedY = 0;
			break;
		case SDL_QUIT:
			return 1;
		}
	}
	return 0;
}
void check_collisions_horizontal(game_t* game, double delta) {
	player_t* player = &game->player;
	double new_p_x = player->coords.x + player->speedX * delta;
	if (!player->on_ground) {
		if (player->speedX <= 0) {
			if (get_object(game->objects, new_p_x, player->coords.y).type == PLATFORM ||
				get_object(game->objects, new_p_x, player->coords.y + player->height - OFFSET).type == PLATFORM) {
				player->speedX = 0;
			}
		}
		else {
			if (get_object(game->objects, new_p_x + player->width, player->coords.y).type == PLATFORM ||
				get_object(game->objects, new_p_x + player->width, player->coords.y + player->height - OFFSET).type == PLATFORM) {
				player->speedX = 0;
			}
		}
	}
	if (new_p_x <= 0 || new_p_x + game->player.width >= SCREEN_WIDTH) player->speedX = 0;
}
void check_collisions_vertical(game_t* game, double delta) {
	player_t* player = &game->player;
	double vel_x = player->speedX;
	double vel_y = player->speedY;
	double p_x = player->coords.x;
	double p_y = player->coords.y;
	int p_w = player->width;
	int p_h = player->height;

	double new_p_x = p_x + vel_x * delta;
	double new_p_y = p_y + vel_y * delta;

	object_t points[4] = {
		get_object(game->objects, new_p_x, new_p_y),
		get_object(game->objects, new_p_x + p_w, new_p_y),
		get_object(game->objects, new_p_x, new_p_y + p_h),
		get_object(game->objects, new_p_x + p_w, new_p_y + p_h)
	};

	bool over_ladder = check_if_over_ladder(game);


	if (points[0].type == LADDER || points[1].type == LADDER || points[2].type == LADDER || points[3].type == LADDER) {
		player->on_ladder = true;
	}

	if (game->player.speedY < 0) {
		if (player->jumping) {
			if (points[0].type == PLATFORM || points[1].type == PLATFORM || new_p_y <= game->objects[0].height) {
				player->jumping = false;
				player->speedY = 0;
			}
		}
		else if (!check_if_on_ladder(game, p_x, p_y) && !player->on_ground) {
			player->speedY = 0;
		}
	}
	else {
		if (points[2].type == PLATFORM || points[3].type == PLATFORM) {
			player->on_ground = true;
			if (player->on_ladder) {
				player->coords.y = ((points[3].type == PLATFORM) ? points[3].start.y : points[2].start.y) - player->height;
			}
			if (!over_ladder) player->speedY = 0;
		}
	}
}
void check_collisions(game_t* game, double delta) {
	check_collisions_horizontal(game, delta);

	game->player.on_ladder = false;
	game->player.on_ground = false;

	check_collisions_vertical(game, delta);
}
void handle_jumping(player_t* player, bool* flag, double* old_y) {
	if (player->jumping && *flag) {
		*old_y = player->coords.y;
		*flag = false;
	}

	if ((*old_y - player->coords.y) <= -OFFSET) {
		player->jumping = false;
		*flag = true;
	}

	if ((*old_y - player->coords.y) >= JUMP_HEIGHT && player->jumping) {
		player->jumping = false;
		player->on_ground = false;
		player->speedY = 0;
		*flag = true;
	}
}
void handle_movement(game_t* game, double delta) {
	if (game->player.on_ground == false && game->player.on_ladder == false && game->player.speedY == 0)
		game->player.coords.y += game->gravity * delta;

	game->player.coords.x += game->player.speedX * delta;
	game->player.coords.y += game->player.speedY * delta;

}


void update_info_box(game_t* game, double worldTime, double fps) {
	char text[128];
	sprintf(text, "Szablon drugiego zadania, czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
	draw_string(game, text, SCREEN_WIDTH / 2 - strlen(text) * 4, 10);

	sprintf(text, "Esc - wyjscie, \032 \030 \033 \031 - sterowanie, Spacja - skok");
	draw_string(game, text, SCREEN_WIDTH / 2 - strlen(text) * 4, 20);

	sprintf(text, "Wykonane podpunkty: 1,2,3,4 oraz A. Autor: Mikolaj Wiszniewski 197925");
	draw_string(game, text, SCREEN_WIDTH / 2 - strlen(text) * 4, 30);
}
void cleanupAndQuit(game_t* game, int errorCode) {
	printf("Error code: %d\n", errorCode);

	SDL_FreeSurface(game->charset);
	SDL_FreeSurface(game->screen);
	SDL_FreeSurface(game->platform);
	SDL_FreeSurface(game->character);
	SDL_FreeSurface(game->ladder);
	SDL_DestroyTexture(game->scrtex);
	SDL_DestroyRenderer(game->renderer);
	SDL_DestroyWindow(game->window);
	SDL_Quit();

	exit(errorCode);
}
void load_bmp(game_t* game) {
	game->charset = SDL_LoadBMP("./img/cs8x8.bmp");
	if (game->charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		cleanupAndQuit(game, 1);
	};
	SDL_SetColorKey(game->charset, true, 0x000000);

	game->platform = SDL_LoadBMP("./img/platform.bmp");
	if (game->platform == NULL) {
		printf("SDL_LoadBMP(platform.bmp) error: %s\n", SDL_GetError());
		cleanupAndQuit(game, 1);
	};
	SDL_SetColorKey(game->platform, true, 0xFFFFFF);

	game->ladder = SDL_LoadBMP("./img/ladder.bmp");
	if (game->ladder == NULL) {
		printf("SDL_LoadBMP(ladder.bmp) error: %s\n", SDL_GetError());
		cleanupAndQuit(game, 1);
	};
	SDL_SetColorKey(game->ladder, true, 0xFFFFFF);

	game->character = SDL_LoadBMP("./img/character.bmp");
	if (game->character == NULL) {
		printf("SDL_LoadBMP(character.bmp) error: %s\n", SDL_GetError());
		cleanupAndQuit(game, 1);
	}
	SDL_SetColorKey(game->character, true, 0xFFFFFF);
}


void init_SDL(game_t* game) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		exit(1);
	}

	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &game->window, &game->renderer) != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(game->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(game->window, "Projekt drugi - king donkey");

	game->screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	game->scrtex = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_ShowCursor(SDL_DISABLE);
}
void init_other(game_t* game) {
	object_t* info = &game->objects[0];
	info->type = INFO;
	info->start.x = 0;
	info->start.y = 0;
	info->height = 64;
	info->width = SCREEN_WIDTH;

	game->objects[1] = { {32, 352}, PLATFORM, 4, 1 };
	game->objects[2] = { {0, SCREEN_HEIGHT - TILE_SIZE}, PLATFORM, SCREEN_WIDTH / TILE_SIZE, 1 };
	game->objects[3] = { {224, 352}, PLATFORM, 3, 1 };
	game->objects[4] = { {384, 352}, PLATFORM, 5, 1 };
	game->objects[5] = { {32, 256}, PLATFORM, 8, 1 };
	game->objects[6] = { {384, 256}, PLATFORM, 1, 1 };
	game->objects[7] = { {480 ,256}, PLATFORM, 3, 1 };
	game->objects[8] = { {96, 160}, PLATFORM, 7, 1 };
	game->objects[9] = { {416, 160} , PLATFORM, 5, 1 };
}
void init_ladders(game_t* game) {
	game->objects[10] = { {64, 352}, LADDER_PARAMS };
	game->objects[11] = { {256, 256}, LADDER_PARAMS };
	game->objects[12] = { {384, 256}, LADDER_PARAMS };
	game->objects[13] = { {96, 160}, LADDER_PARAMS };
	game->objects[14] = { {544, 160}, LADDER_PARAMS };
}
void init_player(game_t* game) {
	game->player = { START, 15, 15, false, false, false, 0, 0 };
}
void init_game(game_t* game) {
	init_SDL(game);
	init_player(game);
	init_other(game);
	init_ladders(game);
	game->gravity = GRAVITY;
	load_bmp(game);
}


void game_loop(game_t* game, int* quit, bool* flag, double* old_y, double* worldTime, double delta, double fps) {
	SDL_FillRect(game->screen, NULL, BACKGROUND_COLOR);

	for (int i = 0; i < OBJ_COUNT; i++)
		draw_obj(game, i);
	draw_player(game);


	*quit = handle_inputs(game, worldTime);
	check_collisions(game, delta);
	handle_jumping(&game->player, flag, old_y);

	handle_movement(game, delta);

	update_info_box(game, *worldTime, fps);
	SDL_UpdateTexture(game->scrtex, NULL, game->screen->pixels, game->screen->pitch);
	SDL_RenderCopy(game->renderer, game->scrtex, NULL, NULL);
	SDL_RenderPresent(game->renderer);

}


int main(int argc, char** argv) {
	int t1, t2, quit, frames;
	double delta, worldTime, fpsTimer, fps, old_y;
	bool flag = true;

	game_t game;
	init_game(&game);

	t1 = SDL_GetTicks();
	frames = fpsTimer = fps = quit = worldTime = old_y = 0;

	while (!quit) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		worldTime += delta;
		fpsTimer += delta;
		if (fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};

		game_loop(&game, &quit, &flag, &old_y, &worldTime, delta, fps);

		frames++;
	};

	cleanupAndQuit(&game, 1);
	return 0;
}
