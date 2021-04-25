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
	pd_MAX = 5,
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

typedef enum {
	tc_NoDraw = 1, // Don't draw the player
	tc_Noop = 2, // Nothing, the usual
	tc_Goto = 3, // Go to a selected planet
} TerminalCode;

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
#define GENERATED_PLANET_COUNT ((int)5) // Number of planets the player can choose from

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

const char *PLANET_NAMES[] = {
		"Alpha Centauri",
		"Krieg",
		"Centurion 4",
		"Fr3dom O-1a",
		"Earth, Super",
		"Prosperity *",
		"S-0 Yland",
		"Irrumabo",
		"Merde P-3T1te",
		"Sram",
		"J-00Piter",
		"Jagras",
};
const int PLANET_NAMES_COUNT = sizeof(PLANET_NAMES) / sizeof(const char *);

JULoadedAsset ASSETS[] = {
		{"assets/player.png", 0, 0, 16, 24},
		{"assets/yesbutton.png", 0, 0, 50, 50, 0, 3},
		{"assets/stars.png", 0, 0, 29, 29, 0, 4},
		{"assets/launchbutton.png", 0, 0, 58, 58, 0, 3},
		{"assets/home.png"},
		{"assets/overlay.jufnt"},
		{"assets/helpterm.png"},
		{"assets/memorialterm.png"},
		{"assets/missionterm.png"},
		{"assets/stockterm.png"},
		{"assets/weaponterm.png"},
		{"assets/cursor.png"},
		{"assets/terminalbg.png"},
		{"assets/planet1.png"},
		{"assets/planet2.png"},
		{"assets/planet3.png"},
		{"assets/planet4.png"},
		{"assets/planet5.png"},
};
const int ASSET_COUNT = sizeof(ASSETS) / sizeof(JULoadedAsset);
#define PLANET_TEXTURE_COUNT ((int)5)

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
	int planetTexIndex;
	int planetNameIndex;
} PNLPlanetSpecs;

typedef struct PNLHome {
	// just maximum size because w/e
	PNLHomeBlock blocks[HOME_WORLD_GRID_WIDTH * HOME_WORLD_GRID_HEIGHT];
	int size;
} PNLHome;

// Information of the planet the sprite is on
typedef struct PNLPlanet {
	PNLPlanetSpecs spec; // Spec this planet comes from
} PNLPlanet;

typedef struct PNLAssets {
	VK2DTexture bgHome;
	VK2DTexture bgTerminal;
	VK2DTexture texHelpTerminal;
	VK2DTexture texMemorialTerminal;
	VK2DTexture texMissionTerminal;
	VK2DTexture texStockTerminal;
	VK2DTexture texWeaponTerminal;
	VK2DTexture texCursor;
	VK2DTexture texPlanets[PLANET_TEXTURE_COUNT];
	JUSprite sprButtonLaunch;
	JUSprite sprStars;
	JUSprite sprButtonYes;
	JUFont fntOverlay;
} PNLAssets;

typedef struct PNLRuntime {
	PNLPlayer player;

	// Current planet, only matters if out on an expedition
	PNLPlanet planet;
	PNLPlanetSpecs potentialPlanets[GENERATED_PLANET_COUNT];
	int selectedPlanet; // When the player selects a planet to go to it will be here
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

/********************** Drawing/Updating Terminal Menus **********************/

// Sprite should have 3 frames - normal, mouse over, and pressed
// Returns true if the button has been pressed
bool pnlDrawButton(PNLRuntime game, JUSprite button, real x, real y) {
	JURectangle r = {x, y, button->Internal.w, button->Internal.h};
	bool mouseOver = juPointInRectangle(&r, game->mouseX, game->mouseY);
	bool pressed = mouseOver && game->mouseLHeld;
	juSpriteDrawFrame(button, mouseOver ? (pressed ? 2 : 1) : 0, x, y);
	return mouseOver && game->mouseLReleased;
}

TerminalCode pnlUpdateMemorialTerminal(PNLRuntime game) {
	VK2DCamera cam = vk2dRendererGetCamera();
	// Coordinates to start drawing the background - the +3 is to account for the background's frame
	float x = cam.x + (GAME_WIDTH / 2) - (game->assets.bgTerminal->img->width / 2) + 3;
	float y = cam.y + (GAME_HEIGHT / 2) - (game->assets.bgTerminal->img->height / 2) + 3;
	vk2dDrawTexture(game->assets.bgTerminal, x - 3, y - 3);

	return tc_NoDraw;
}

TerminalCode pnlUpdateMissionSelectTerminal(PNLRuntime game) {
	VK2DCamera cam = vk2dRendererGetCamera();
	TerminalCode code = tc_NoDraw;

	// Coordinates to start drawing the background - the +3 is to account for the background's frame
	float x = cam.x + (GAME_WIDTH / 2) - (game->assets.bgTerminal->img->width / 2) + 4;
	float y = cam.y + (GAME_HEIGHT / 2) - (game->assets.bgTerminal->img->height / 2) + 4;
	vk2dDrawTexture(game->assets.bgTerminal, x - 4, y - 4);

	// Draw planets and their info
	float w = game->assets.texPlanets[0]->img->width;
	float h = game->assets.texPlanets[0]->img->height;
	for (int i = 0; i < GENERATED_PLANET_COUNT; i++) {
		vk2dDrawTexture(game->assets.texPlanets[game->potentialPlanets[i].planetTexIndex], x + 1, y);
		juFontDraw(game->assets.fntOverlay, x + w + 10, y, PLANET_NAMES[game->potentialPlanets[i].planetNameIndex]);
		juFontDraw(game->assets.fntOverlay, x + w + 10, y + 29, "Cost: $%.2f | Potential Fame: %.0f", (float)game->potentialPlanets[i].doshCost, (float)roundTo(game->potentialPlanets[i].fameBonus, 10));

		for (int j = 0; j < 4; j++) {
			if (game->potentialPlanets[i].planetDifficulty <= j)
				vk2dRendererSetColourMod(VK2D_BLACK);
			juSpriteDrawFrame(game->assets.sprStars, j, (x + game->assets.bgTerminal->img->width - w - w - 11) + ((j % 2) * 29), y + (j > 1 ? 29 : 0));
			vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
		}

		if (pnlDrawButton(game, game->assets.sprButtonLaunch, x + game->assets.bgTerminal->img->width - w - 9, y)) {
			game->selectedPlanet = i;
			code = tc_Goto;
		}
		y += h;
	}

	return code;
}

TerminalCode pnlUpdateHelpTerminal(PNLRuntime game) {
	VK2DCamera cam = vk2dRendererGetCamera();
	// Coordinates to start drawing the background - the +3 is to account for the background's frame
	float x = cam.x + (GAME_WIDTH / 2) - (game->assets.bgTerminal->img->width / 2) + 3;
	float y = cam.y + (GAME_HEIGHT / 2) - (game->assets.bgTerminal->img->height / 2) + 3;
	vk2dDrawTexture(game->assets.bgTerminal, x - 3, y - 3);

	return tc_NoDraw;
}

TerminalCode pnlUpdateStocksTerminal(PNLRuntime game) {
	VK2DCamera cam = vk2dRendererGetCamera();
	// Coordinates to start drawing the background - the +3 is to account for the background's frame
	float x = cam.x + (GAME_WIDTH / 2) - (game->assets.bgTerminal->img->width / 2) + 3;
	float y = cam.y + (GAME_HEIGHT / 2) - (game->assets.bgTerminal->img->height / 2) + 3;
	vk2dDrawTexture(game->assets.bgTerminal, x - 3, y - 3);

	return tc_NoDraw;
}

TerminalCode pnlUpdateWeaponsTerminal(PNLRuntime game) {
	VK2DCamera cam = vk2dRendererGetCamera();
	// Coordinates to start drawing the background - the +3 is to account for the background's frame
	float x = cam.x + (GAME_WIDTH / 2) - (game->assets.bgTerminal->img->width / 2) + 3;
	float y = cam.y + (GAME_HEIGHT / 2) - (game->assets.bgTerminal->img->height / 2) + 3;
	vk2dDrawTexture(game->assets.bgTerminal, x - 3, y - 3);

	return tc_NoDraw;
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
	specs.planetTexIndex = (int)floorl(randr() * (real)PLANET_TEXTURE_COUNT);

	// Make unique name
	int chosenName;
	bool nameTaken = true;
	while (nameTaken) {
		chosenName = (int)floorl(randr() * (real)PLANET_NAMES_COUNT);
		nameTaken = false;
		for (int i = 0; i < GENERATED_PLANET_COUNT; i++)
			if (chosenName == game->potentialPlanets[i].planetNameIndex)
				nameTaken = true;
	}
	specs.planetNameIndex = chosenName;

	return specs;
}

TerminalCode pnlUpdateBlock(PNLRuntime game, int index) { // returns true if the player should be rendered
	PNLHomeBlock *block = &game->home.blocks[index];
	TerminalCode code = tc_Noop;

	if (block->type == hb_Memorial) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texMemorialTerminal, block->x, block->y);
		} else {
			code = pnlUpdateMemorialTerminal(game);
		}
	} else if (block->type == hb_MissionSelect) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texMissionTerminal, block->x, block->y);
		} else {
			code = pnlUpdateMissionSelectTerminal(game);
		}
	} else if (block->type == hb_Help) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texHelpTerminal, block->x, block->y);
		} else {
			code = pnlUpdateHelpTerminal(game);
		}
	} else if (block->type == hb_Stocks) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texStockTerminal, block->x, block->y);
		} else {
			code = pnlUpdateStocksTerminal(game);
		}
	} else if (block->type == hb_Weapons) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texWeaponTerminal, block->x, block->y);
		} else {
			code = pnlUpdateWeaponsTerminal(game);
		}
	}

	return code;
}

/********************** Functions specific to regions **********************/
void pnlInitHome(PNLRuntime game) {
	for (int i = 0; i < GENERATED_PLANET_COUNT; i++)
		game->potentialPlanets[i] = pnlCreatePlanetSpec(game);
}

WorldSelection pnlUpdateHome(PNLRuntime game) {
	// Draw background
	pnlDrawTiledBackground(game, game->assets.bgHome);

	// Draw/update blocks
	TerminalCode code = tc_Noop;
	for (int i = 0; i < game->home.size && code == tc_Noop; i++) {
		code = pnlUpdateBlock(game, i);
	}

	pnlPlayerUpdate(game, code == tc_Noop);

	// Overlay
	VK2DCamera cam = vk2dRendererGetCamera();
	vk2dRendererSetColourMod(VK2D_BLACK);
	vk2dDrawRectangle(cam.x, cam.y, cam.w, 20);
	vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
juFontDraw(game->assets.fntOverlay, cam.x, cam.y - 5, "Dosh: $%.2f | Fame: %.0f | FPS: %.0f", (float)game->player.dosh, (float)game->player.fame, 1.0f / juDelta());
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
	game->assets.bgTerminal = juLoaderGetTexture(game->loader, "assets/terminalbg.png");
	game->assets.sprButtonYes = juLoaderGetSprite(game->loader, "assets/yesbutton.png");
	game->assets.texPlanets[0] = juLoaderGetTexture(game->loader, "assets/planet1.png");
	game->assets.texPlanets[1] = juLoaderGetTexture(game->loader, "assets/planet2.png");
	game->assets.texPlanets[2] = juLoaderGetTexture(game->loader, "assets/planet3.png");
	game->assets.texPlanets[3] = juLoaderGetTexture(game->loader, "assets/planet4.png");
	game->assets.texPlanets[4] = juLoaderGetTexture(game->loader, "assets/planet5.png");
	game->assets.sprButtonLaunch = juLoaderGetSprite(game->loader, "assets/launchbutton.png");
	game->assets.sprStars = juLoaderGetSprite(game->loader, "assets/stars.png");

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
