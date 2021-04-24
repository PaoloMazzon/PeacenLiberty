#define SDL_MAIN_HANDLED
#include <JamUtil.h>
#include <SDL2/SDL.h>

/********************** Typedefs **********************/
typedef long double real;
typedef struct {real x; real y;} physvec2;
typedef enum {
	pd_Easy,
	pd_Medium,
	pd_Hard,
	pd_SeventhCircle,
} PlanetDifficulty;
typedef enum {
	ws_Home = 0,
	ws_Offsite = 1,
} WorldSelection;

/********************** Constants **********************/
const int GAME_WIDTH = 600;
const int GAME_HEIGHT = 400;
const char *GAME_TITLE = "Peace & Liberty";
const real PHYS_TERMINAL_VELOCITY = 50;
const real PHYS_FRICTION = 0.5;
const real PHYS_ACCELERATION = 1;

JULoadedAsset ASSETS[] = {
		{"assets/player.png", 0, 0, 16, 24}
};
const int ASSET_COUNT = sizeof(ASSETS) / sizeof(JULoadedAsset);

/********************** Struct **********************/

typedef struct PNLPlayer {
	physvec2 pos;
	physvec2 velocity;
	JUSprite sprite;
} PNLPlayer;

typedef struct PNLEnemy {
	real x, y;
	real dosh;
	real fame;
} PNLEnemy;

// Information of a planet the sprite might go to
typedef struct PNLPlanetSpecs {
	real fameBonus;
	real doshBonus;
	PlanetDifficulty planetDifficulty;
} PNLPlanetSpecs;

// Information of the planet the sprite is on
typedef struct PNLPlanet {

} PNLPlanet;

typedef struct PNLRuntime {
	PNLPlayer player;

	// Current planet, only matters if out on an expidition
	PNLPlanet planet;
	bool onSite; // on a planet or not

	// All assets
	JULoader loader;
} *PNLRuntime;

/********************** Utility **********************/
physvec2 addPhysVec2(physvec2 v1, physvec2 v2) {
	physvec2 v = {v1.x + v2.x, v1.y + v2.y};
	return v;
}

/********************** Player and other entities **********************/
void pnlPlayerUpdate(PNLRuntime game) {
	juSpriteDraw(game->player.sprite, game->player.pos.x, game->player.pos.y);
	game->player.velocity.x += (((real)juKeyboardGetKey(SDL_SCANCODE_D)) - ((real)juKeyboardGetKey(SDL_SCANCODE_A))) * PHYS_ACCELERATION * juDelta();
	game->player.velocity.y += (((real)juKeyboardGetKey(SDL_SCANCODE_S)) - ((real)juKeyboardGetKey(SDL_SCANCODE_W))) * PHYS_ACCELERATION * juDelta();
	game->player.pos = addPhysVec2(game->player.pos, game->player.velocity);
}

/********************** Functions specific to regions **********************/
void pnlInitHome(PNLRuntime game) {

}

WorldSelection pnlUpdateHome(PNLRuntime game) {
	vk2dRendererSetColourMod(VK2D_BLACK);
	vk2dDrawRectangle(0, 0, 20, 20);
	vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
	pnlPlayerUpdate(game);
	return ws_Home;
}

void pnlQuitHome(PNLRuntime game) {

}

void pnlInitPlanet(PNLRuntime game) {

}

WorldSelection pnlUpdatePlanet(PNLRuntime game) {
	return ws_Offsite;
}

void pnlQuitPlanet(PNLRuntime game) {

}

/********************** Core game functions **********************/
void pnlInit(PNLRuntime game) {
	game->player.sprite = juLoaderGetSprite(game->loader, "assets/player.png");

	pnlInitHome(game);
}

// Called before the rendering begins
void pnlPreFrame(PNLRuntime game) {
	VK2DCamera cam = {};
	cam.x = game->player.pos.x - (GAME_WIDTH / 2);
	cam.y = game->player.pos.y - (GAME_HEIGHT / 2);
	cam.w = GAME_WIDTH;
	cam.h = GAME_HEIGHT;
	cam.zoom = 1;
	vk2dRendererSetCamera(cam);
}

// Called during rendering
void pnlUpdate(PNLRuntime game) {
	if (game->onSite && pnlUpdatePlanet(game) == ws_Home) {
		game->onSite = false;
		pnlQuitPlanet(game);
		pnlInitHome(game);
	} else if (!game->onSite && pnlUpdateHome(game) == ws_Offsite) {
		game->onSite = false;
		pnlQuitHome(game);
		pnlInitPlanet(game);
	}
}

void pnlQuit(PNLRuntime game) {
	if (game->onSite)
		pnlQuitPlanet(game);
	else
		pnlQuitHome(game);
}

/********************** main lmao **********************/
int main() {
	// Init
	SDL_Window *window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GAME_WIDTH, GAME_HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	VK2DRendererConfig config = {
			msaa_16x,
			sm_TripleBuffer,
			ft_Nearest,
	};
	vk2dRendererInit(window, config);
	juInit(window);
	bool running = true;
	SDL_Event e;

	// Backbuffer
	VK2DTexture backbuffer = vk2dTextureCreate(vk2dRendererGetDevice(), GAME_WIDTH, GAME_HEIGHT);
	int w, h, lw, lh;
	SDL_GetWindowSize(window, &w, &h);
	lw = w;
	lh = h;
	vk2dRendererSetTextureCamera(true);

	// Game
	PNLRuntime game = calloc(1, sizeof(struct PNLRuntime));
	game->loader = juLoaderCreate(ASSETS, ASSET_COUNT);
	pnlInit(game);

	while (running) {
		juUpdate();
		while (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT)
				running = false;

		if (running) {
			// Update viewport and reset backbuffer on window size change
			lw = w;
			lh = h;
			SDL_GetWindowSize(window, &w, &h);
			if (w != lw || h != lh) {
				vk2dRendererWait();
				vk2dTextureFree(backbuffer);
				backbuffer = vk2dTextureCreate(vk2dRendererGetDevice(), w, h);
				vk2dRendererSetViewport(0, 0, w, h);
			}

			// Update game
			pnlPreFrame(game);
			vk2dRendererStartFrame(VK2D_BLACK);
			vk2dRendererSetTarget(backbuffer);
			vk2dRendererClear();
			pnlUpdate(game);
			vk2dRendererSetTarget(VK2D_TARGET_SCREEN);
			vk2dDrawTexture(backbuffer, 0, 0);
			vk2dRendererEndFrame();
		}
	}


	// Free assets
	vk2dRendererWait();
	pnlQuit(game);
	juLoaderFree(game->loader);
	free(game);
	vk2dTextureFree(backbuffer);

	// Free
	juQuit();
	vk2dRendererQuit();
	SDL_DestroyWindow(window);
	return 0;
}
