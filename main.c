#define SDL_MAIN_HANDLED
#include <JamUtil.h>
#include <SDL2/SDL.h>
#include <time.h>

/********************** Typedefs **********************/
typedef long double real;
typedef struct {real x; real y;} physvec2;
typedef enum {
	pd_Easy = 1,
	pd_Medium = 2,
	pd_Hard = 3,
	pd_SeventhCircle = 4,
} PlanetDifficulty;

typedef enum {
	ws_Home = 0,
	ws_Offsite = 1,
} WorldSelection;

typedef enum { // Various elements in the home map
	hb_None = 0,
	hb_MissionSelect = 1,
	hb_Stocks = 2,
	hb_Memorial = 3,
	hb_Weapons = 4,
	hb_Help = 5,
} HomeBlocks;

/********************** Constants **********************/
const int GAME_WIDTH = 600;
const int GAME_HEIGHT = 400;
const char *GAME_TITLE = "Peace & Liberty";
const real PHYS_TERMINAL_VELOCITY = 25;
const real PHYS_FRICTION = 25;
const real PHYS_ACCELERATION = 18;
const float PHYS_CAMERA_FRICTION = 8;
const real WORLD_GRID_WIDTH = 50;
const real WORLD_GRID_HEIGHT = 90;
const int DEST_PLANETS = 3; // How many planets to choose from for missions
const real FAME_PER_PLANET = 30; // Base fame per planet for 1 star difficulty
const real FAME_VARIANCE = 10;
const real FAME_MULTIPLIER = 1.6; // Multiplier per difficulty level
const real FAME_2_STAR_CUTOFF = 20; // Required fame to get these missions
const real FAME_3_STAR_CUTOFF = 100;
const real FAME_4_STAR_CUTOFF = 300;
const real FAME_LOWER_STAR_CHANCE = 0.3; // Chance of mission being a star below current level
const real DOSH_MULTIPLIER = 2.5; // dosh multiplier per difficulty level
const real DOSH_PLANET_COST = 200;
const real DOSH_PLANET_COST_VARIANCE = 50;
const real DOSH_UPKEEP_COST = 400; // Charged
const real DOSH_UPKEEP_VARIANCE = 70;
const real IN_RANGE_TERMINAL_DISTANCE = 30; // Distance away from a terminal considered to be "in-range"

const HomeBlocks HOME_WORLD_GRID[] = {
		hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
		hb_None, hb_MissionSelect, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
		hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
		hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
		hb_Stocks, hb_None, hb_None, hb_None, hb_Memorial, hb_None, hb_None, hb_None,
		hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
		hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
		hb_None, hb_None, hb_Help, hb_None, hb_None, hb_None, hb_Weapons, hb_None,
};
#define HOME_WORLD_GRID_WIDTH ((int)8)
#define HOME_WORLD_GRID_HEIGHT ((int)8)

JULoadedAsset ASSETS[] = {
		{"assets/player.png", 0, 0, 16, 24},
		{"assets/home.png"},
		{"assets/overlay.jufnt"},
		{"assets/helpterm.png"},
		{"assets/memorialterm.png"},
		{"assets/missionterm.png"},
		{"assets/stockterm.png"},
		{"assets/weaponterm.png"},
		{"assets/cursor.png"},
};
const int ASSET_COUNT = sizeof(ASSETS) / sizeof(JULoadedAsset);

/********************** Struct **********************/

typedef struct PNLHomeBlock { // Things in the home world for the player to interact with
	HomeBlocks type;
	real x, y;
} PNLHomeBlock;

typedef struct PNLPlayer {
	physvec2 pos;
	physvec2 velocity;
	JUSprite sprite;
	real dosh;
	real fame;
} PNLPlayer;

const PNLPlayer PLAYER_DEFAULT_STATE = {
		{300, 200},
		{0, 0},
		NULL,
		1000,
		0,
};

typedef struct PNLEnemy {
	real x, y;
	real dosh;
	real fame;
} PNLEnemy;

// Information of a planet the sprite might go to
typedef struct PNLPlanetSpecs {
	real fameBonus;
	real doshCost;
	PlanetDifficulty planetDifficulty;
} PNLPlanetSpecs;

typedef struct PNLHome {
	// just maximum size because w/e
	PNLHomeBlock blocks[HOME_WORLD_GRID_WIDTH * HOME_WORLD_GRID_HEIGHT];
	int size;
} PNLHome;

// Information of the planet the sprite is on
typedef struct PNLPlanet {

} PNLPlanet;

typedef struct PNLAssets {
	VK2DTexture bgHome;
	VK2DTexture texHelpTerminal;
	VK2DTexture texMemorialTerminal;
	VK2DTexture texMissionTerminal;
	VK2DTexture texStockTerminal;
	VK2DTexture texWeaponTerminal;
	VK2DTexture texCursor;
	JUFont fntOverlay;
} PNLAssets;

typedef struct PNLRuntime {
	PNLPlayer player;

	// Current planet, only matters if out on an expedition
	PNLPlanet planet;
	bool onSite; // on a planet or not
	PNLHome home; // home area

	// All assets
	JULoader loader;
	PNLAssets assets;

	// Window interface stuff
	float mouseX, mouseY; // Mouse x/y in the game world - not the window relative
	bool mouseLPressed, mouseLReleased, mouseLHeld;
	bool mouseRPressed, mouseRReleased, mouseRHeld;
	bool mouseMPressed, mouseMReleased, mouseMHeld;
	int ww, wh;
} *PNLRuntime;

/********************** Utility **********************/
physvec2 addPhysVec2(physvec2 v1, physvec2 v2) {
	physvec2 v = {v1.x + v2.x, v1.y + v2.y};
	return v;
}

physvec2 subPhysVec2(physvec2 v1, physvec2 v2) {
	physvec2 v = {v1.x - v2.x, v1.y - v2.y};
	return v;
}

real sign(real a) {
	return a > 0 ? 1 : (a < 0 ? -1 : 0);
}

real absr(real a) {
	return a < 0 ? -a : a;
}

real roundTo(real a, real to) {
	return floorl(a / to) * to;
}

real randr() { // Returns a real from 0 - 1
	return (real)rand() / (real)RAND_MAX;
}

real weightedChance(real percent) { // 70% is 0.7
	return randr() < percent;
}

/********************** Player and other entities **********************/
void pnlPlayerUpdate(PNLRuntime game, bool drawPlayer) {
	// Move
	physvec2 oldVel = game->player.velocity;
	game->player.velocity.x += (((real)juKeyboardGetKey(SDL_SCANCODE_D)) - ((real)juKeyboardGetKey(SDL_SCANCODE_A))) * PHYS_ACCELERATION * juDelta();
	game->player.velocity.y += (((real)juKeyboardGetKey(SDL_SCANCODE_S)) - ((real)juKeyboardGetKey(SDL_SCANCODE_W))) * PHYS_ACCELERATION * juDelta();
	physvec2 diff = subPhysVec2(oldVel, game->player.velocity);
	bool movedX = diff.x != 0;
	bool movedY = diff.y != 0;

	// Apply friction
	real rfric = PHYS_FRICTION * juDelta();
	if (absr(game->player.velocity.x) - rfric < 0 && !movedX) // x
		game->player.velocity.x = 0;
	else if (!movedX)
		game->player.velocity.x += -sign(game->player.velocity.x) * rfric;
	if (absr(game->player.velocity.y) - rfric < 0 && !movedY) // y
		game->player.velocity.y = 0;
	else if (!movedY)
		game->player.velocity.y += -sign(game->player.velocity.y) * rfric;

	// Cap velocity
	if (absr(game->player.velocity.x) > PHYS_TERMINAL_VELOCITY) game->player.velocity.x = sign(game->player.velocity.x) * PHYS_TERMINAL_VELOCITY;
	if (absr(game->player.velocity.y) > PHYS_TERMINAL_VELOCITY) game->player.velocity.y = sign(game->player.velocity.y) * PHYS_TERMINAL_VELOCITY;

	// Apply velocity
	game->player.pos = addPhysVec2(game->player.pos, game->player.velocity);

	// Draw player
	if (drawPlayer)
		juSpriteDraw(game->player.sprite, game->player.pos.x, game->player.pos.y);
}

void pnlDrawTiledBackground(PNLRuntime game, VK2DTexture bg) {
	// Draw background
	VK2DCamera cam = vk2dRendererGetCamera();
	int tx = (cam.w / bg->img->width) + 3;
	int ty = (cam.h / bg->img->height) + 3;
	float ssx = roundTo(cam.x, bg->img->width) - bg->img->width;
	float sx = ssx;
	float sy = roundTo(cam.y, bg->img->height) - bg->img->height;
	for (int i = 0; i < ty; i++) {
		for (int j = 0; j < tx; j++) {
			vk2dDrawTexture(bg, sx, sy);
			sx += bg->img->width;
		}
		sy += bg->img->height;
		sx = ssx;
	}
}

// Creates a planet's specs based on player's current fame
PNLPlanetSpecs pnlCreatePlanetSpec(PNLRuntime game) {
	PNLPlanetSpecs specs = {};
	PlanetDifficulty difficulty;

	// Find cutoff
	if (game->player.fame >= FAME_4_STAR_CUTOFF) {
		if (weightedChance(FAME_LOWER_STAR_CHANCE))
			difficulty = pd_Hard;
		else
			difficulty = pd_SeventhCircle;
	} else if (game->player.fame >= FAME_3_STAR_CUTOFF) {
		if (weightedChance(FAME_LOWER_STAR_CHANCE))
			difficulty = pd_Medium;
		else
			difficulty = pd_Hard;
	} else if (game->player.fame >= FAME_2_STAR_CUTOFF) {
		if (weightedChance(FAME_LOWER_STAR_CHANCE))
			difficulty = pd_Easy;
		else
			difficulty = pd_Medium;
	} else  {
		difficulty = pd_Easy;
	}

	// Find multipliers
	real doshMult = powl(DOSH_MULTIPLIER, (real)difficulty);
	real fameMult = powl(FAME_MULTIPLIER, (real)difficulty);

	// Contruct planet specs
	specs.doshCost = (DOSH_PLANET_COST * doshMult) + (DOSH_PLANET_COST_VARIANCE * doshMult * randr());
	specs.fameBonus = (FAME_PER_PLANET * fameMult) + (FAME_VARIANCE * fameMult * randr());
	specs.planetDifficulty = difficulty;

	return specs;
}

bool pnlUpdateBlock(PNLRuntime game, int index) { // returns true if the player should be rendered
	PNLHomeBlock *block = &game->home.blocks[index];
	bool drawPlayer = true;

	if (block->type == hb_Memorial) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texMemorialTerminal, block->x, block->y);
		} else {
			drawPlayer = false;
			// TODO: Draw menu
		}
	} else if (block->type == hb_MissionSelect) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texMissionTerminal, block->x, block->y);
		} else {
			drawPlayer = false;
			// TODO: Draw menu
		}
	} else if (block->type == hb_Help) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texHelpTerminal, block->x, block->y);
		} else {
			drawPlayer = false;
			// TODO: Draw menu
		}
	} else if (block->type == hb_Stocks) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texStockTerminal, block->x, block->y);
		} else {
			drawPlayer = false;
			// TODO: Draw menu
		}
	} else if (block->type == hb_Weapons) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texWeaponTerminal, block->x, block->y);
		} else {
			drawPlayer = false;
			// TODO: Draw menu
		}
	}

	return drawPlayer;
}

/********************** Functions specific to regions **********************/
void pnlInitHome(PNLRuntime game) {

}

WorldSelection pnlUpdateHome(PNLRuntime game) {
	// Draw background
	pnlDrawTiledBackground(game, game->assets.bgHome);

	// Draw/update blocks
	bool drawPlayer = true;
	for (int i = 0; i < game->home.size; i++)
		drawPlayer = pnlUpdateBlock(game, i) && drawPlayer != false ? true : false;

	pnlPlayerUpdate(game, drawPlayer);

	// Overlay
	VK2DCamera cam = vk2dRendererGetCamera();
	vk2dRendererSetColourMod(VK2D_BLACK);
	vk2dDrawRectangle(cam.x, cam.y, cam.w, 20);
	vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
	juFontDraw(game->assets.fntOverlay, cam.x, cam.y - 5, "Dosh: $%.2f | Fame: %.0f | FPS: %.1f", game->player.dosh, game->player.fame, 1.0f / juDelta());
	return ws_Home;
}

void pnlQuitHome(PNLRuntime game) {

}

void pnlInitPlanet(PNLRuntime game) {
	// TODO: This
}

WorldSelection pnlUpdatePlanet(PNLRuntime game) {
	return ws_Offsite; // TODO: This
}

void pnlQuitPlanet(PNLRuntime game) {
	// TODO: This
}

/********************** Core game functions **********************/
void pnlInit(PNLRuntime game) {
	// Load assets
	game->assets.bgHome = juLoaderGetTexture(game->loader, "assets/home.png");
	game->assets.fntOverlay = juLoaderGetFont(game->loader, "assets/overlay.jufnt");
	game->assets.texHelpTerminal = juLoaderGetTexture(game->loader, "assets/helpterm.png");
	game->assets.texMemorialTerminal = juLoaderGetTexture(game->loader, "assets/memorialterm.png");
	game->assets.texMissionTerminal = juLoaderGetTexture(game->loader, "assets/missionterm.png");
	game->assets.texStockTerminal = juLoaderGetTexture(game->loader, "assets/stockterm.png");
	game->assets.texWeaponTerminal = juLoaderGetTexture(game->loader, "assets/weaponterm.png");
	game->assets.texCursor = juLoaderGetTexture(game->loader, "assets/cursor.png");

	// Build home grid
	for (int i = 0; i < HOME_WORLD_GRID_HEIGHT; i++) {
		for (int j = 0; j < HOME_WORLD_GRID_WIDTH; j++) {
			HomeBlocks slot = HOME_WORLD_GRID[(i * HOME_WORLD_GRID_WIDTH) + j];
			if (slot != hb_None) {
				game->home.blocks[game->home.size].x = j * WORLD_GRID_WIDTH;
				game->home.blocks[game->home.size].y = i * WORLD_GRID_HEIGHT;
				game->home.blocks[game->home.size].type = slot;
				game->home.size++;
			}
		}
	}

	// Load default player state
	memcpy(&game->player, &PLAYER_DEFAULT_STATE, sizeof(struct PNLPlayer));
	game->player.sprite = juLoaderGetSprite(game->loader, "assets/player.png");

	pnlInitHome(game);
}

// Called before the rendering begins
void pnlPreFrame(PNLRuntime game) {
	VK2DCamera cam = vk2dRendererGetCamera();
	float destX = game->player.pos.x - (game->ww / 2);
	float destY = game->player.pos.y - (game->wh / 2);
	cam.x += (destX - cam.x) * PHYS_CAMERA_FRICTION * juDelta();
	cam.y += (destY - cam.y) * PHYS_CAMERA_FRICTION * juDelta();
	cam.w = game->ww;
	cam.h = game->wh;
	cam.zoom = 1;//game->wh / GAME_HEIGHT;
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
	vk2dDrawTexture(game->assets.texCursor, game->mouseX - 4, game->mouseY - 4);
}

void pnlQuit(PNLRuntime game) {
	if (game->onSite)
		pnlQuitPlanet(game);
	else
		pnlQuitHome(game);
}

/********************** main lmao **********************/
int main() {
	// Init TODO: Make resizeable
	SDL_Window *window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GAME_WIDTH, GAME_HEIGHT, SDL_WINDOW_VULKAN);
	VK2DRendererConfig config = {
			msaa_16x,
			sm_TripleBuffer,
			ft_Nearest,
	};
	vk2dRendererInit(window, config);
	juInit(window);
	bool running = true;
	SDL_Event e;
	srand(time(NULL));
	SDL_ShowCursor(0);

	// Backbuffer
	VK2DTexture backbuffer = vk2dTextureCreate(vk2dRendererGetDevice(), GAME_WIDTH, GAME_HEIGHT);
	int w, h, lw, lh;
	int lastState, state;
	SDL_GetWindowSize(window, &w, &h);
	lw = w;
	lh = h;
	vk2dRendererSetTextureCamera(true);

	// Game
	PNLRuntime game = calloc(1, sizeof(struct PNLRuntime));
	game->loader = juLoaderCreate(ASSETS, ASSET_COUNT);
	game->ww = w;
	game->wh = h;
	pnlInit(game);

	while (running) {
		juUpdate();
		while (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT)
				running = false;

		if (running) {
			// Update window interface stuff for the game
			lw = w;
			lh = h;
			int mx, my;
			lastState = state;
			SDL_GetWindowSize(window, &w, &h);
			state = SDL_GetMouseState(&mx, &my);
			game->mouseLHeld = state & SDL_BUTTON(SDL_BUTTON_LEFT);
			game->mouseLPressed = (state & SDL_BUTTON(SDL_BUTTON_LEFT)) && !(lastState & SDL_BUTTON(SDL_BUTTON_LEFT));
			game->mouseLReleased = !(state & SDL_BUTTON(SDL_BUTTON_LEFT)) && (lastState & SDL_BUTTON(SDL_BUTTON_LEFT));
			game->mouseRHeld = state & SDL_BUTTON(SDL_BUTTON_RIGHT);
			game->mouseRPressed = (state & SDL_BUTTON(SDL_BUTTON_RIGHT)) && !(lastState & SDL_BUTTON(SDL_BUTTON_RIGHT));
			game->mouseRReleased = !(state & SDL_BUTTON(SDL_BUTTON_RIGHT)) && (lastState & SDL_BUTTON(SDL_BUTTON_RIGHT));
			game->mouseMHeld = state & SDL_BUTTON(SDL_BUTTON_MIDDLE);
			game->mouseMPressed = (state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) && !(lastState & SDL_BUTTON(SDL_BUTTON_MIDDLE));
			game->mouseMReleased = !(state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) && (lastState & SDL_BUTTON(SDL_BUTTON_MIDDLE));

			// Update game
			pnlPreFrame(game);
			VK2DCamera cam = vk2dRendererGetCamera();
			game->mouseX = (mx / (w / GAME_WIDTH)) + cam.x;
			game->mouseY = (my / (h / GAME_HEIGHT)) + cam.y;
			vk2dRendererStartFrame(VK2D_BLACK);
			vk2dRendererSetViewport(0, 0, GAME_WIDTH, GAME_HEIGHT);
			vk2dRendererSetTarget(backbuffer);
			vk2dRendererClear();
			pnlUpdate(game);
			vk2dRendererSetTarget(VK2D_TARGET_SCREEN);
			vk2dRendererSetViewport(0, 0, w, h);
			cam = vk2dRendererGetCamera();
			vk2dDrawTexture(backbuffer, cam.x, cam.y);
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
