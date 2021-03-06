#define SDL_MAIN_HANDLED
#include <JamUtil.h>
#include <SDL2/SDL.h>
#include <time.h>

/********************** Typedefs **********************/
typedef double real;
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

typedef enum {
	wt_Sword = 0,
	wt_Shotgun = 1,
	wt_AssaultRifle = 2,
	wt_Sniper = 3,
	wt_Pistol = 4,
	wt_Any = 5,
} WeaponType;

/********************** Constants **********************/
const int GAME_WIDTH = 600;
const int GAME_HEIGHT = 400;
const int WINDOW_SCALE = 2;
const real TARGET_FRAMERATE = 60;
const char *VERSION_STRING = "v1.2";
const char *GAME_TITLE = "Peace & Liberty";
const char *SAVE_FILE = "save.bin";
const char *SAVE_HIGHSCORE = "hs";
const char *SAVE_DOSH = "dosh";
const char *SAVE_KILLS = "kills";
const char *SAVE_DEATH_PLANET = "planet";
const char *SAVE_DIFFICULTY = "diff";
const real PHYS_TERMINAL_VELOCITY = 7; // Physics variables - all in pixels/second
const real PHYS_FRICTION = 30; // Only applies while the player is not giving a keyboard input
const real PHYS_ACCELERATION = 18;
const float PHYS_CAMERA_FRICTION = 8;
const real WORLD_GRID_WIDTH = 50; // These are really only for the homeworld and honestly dont matter that much
const real WORLD_GRID_HEIGHT = 70;
const real FAME_PER_PLANET = 30; // Base fame per planet for 1 star difficulty
const real FAME_VARIANCE = 10; // Fame for a planet can fluctuate by +/- up to this amount
const real FAME_MULTIPLIER = 1.6; // Multiplier per difficulty level
const real FAME_2_STAR_CUTOFF = 50; // Required fame to get these missions
const real FAME_3_STAR_CUTOFF = 200;
const real FAME_4_STAR_CUTOFF = 500;
const real FAME_LOWER_STAR_CHANCE = 0.3; // Chance of mission being a star below current level
const real DOSH_PLANET_COST = 200; // Cost of departing to a planet
const real DOSH_MULTIPLIER = 1.3; // Cost multiplier per difficulty level
const real DOSH_PLANET_COST_VARIANCE = 50; // How much the cost can vary (also multiplied by cost multiplier)
const real IN_RANGE_TERMINAL_DISTANCE = 40; // Distance away from a terminal considered to be "in-range"
#define GENERATED_PLANET_COUNT ((int)5) // Number of planets the player can choose from
const int WEAPON_MIN_SPREAD = 3; // Minimum/maximum pellets per shotgun blast
const int WEAPON_MAX_SPREAD = 12;
const real WEAPON_MIN_DAMAGE = 20; // Minimum/maximum possible weapon damage (rolled at random)
const real WEAPON_MAX_DAMAGE = 50;
const real ENEMY_HP = 100; // Base enemy hp
const real ENEMY_HP_VARIANCE = 0.3; // Enemy hp can be +/- this percent hp
const real ENEMY_MAX_FAME = 3; // max/min dosh/fame an enemy kill grants
const real ENEMY_MIN_FAME = 1;
const real ENEMY_MAX_DOSH = 5;
const real ENEMY_MIN_DOSH = 2;
const real ENEMY_SPEED_MULTIPLIER = 1.3;
const real ENEMY_SPEED = 110;
const real ENEMY_DAMAGE = 10; // base enemy damage and how much it can vary
const real ENEMY_DAMAGE_VARIANCE = 0.2;
const real ENEMY_DAMAGE_MULTIPLIER = 1.3; // Multipliers based on the difficulty level
const real ENEMY_FAME_MULTIPLIER = 1.5;
const real ENEMY_DOSH_MULTIPLIER = 2;
const real ENEMY_HP_MULTIPLIER = 1.3;
const real ENEMY_HIT_DELAY = 1;
const real ENEMY_HIT_DISTANCE = 15; // distance from the player that counts as a "hit"
const real ENEMY_SPAWN_DELAY = 5; // delay in seconds between enemy spawns
const real ENEMY_SPAWN_DELAY_DECREASE = 0.05; // how much shorter the spawn delay gets each time an enemy spawn
const real PLAYER_MAX_HP = 100;
const real WEAPON_SWORD_DAMAGE_MULTIPLIER = 2; // Swords are risky so huge damage boost
const real WEAPON_SHOTGUN_DAMAGE_MULTIPLIER = 0.9; // Shotguns have lots of pellets so low damage
const real WEAPON_ASSAULTRIFLE_DAMAGE_MULTIPLIER = 0.6; // Assault rifles are fast and long-range so low damage
const real WEAPON_SNIPER_DAMAGE_MULTIPLIER = 3; // Sniper shoots slow but pierces so high damage
const real WEAPON_PISTOL_DAMAGE_MULTIPLIER = 1; // Starting weapon
const real WEAPON_SWORD_RECOIL = 0; // Velocity applied in the opposite direction when firing a given weapon
const real WEAPON_SHOTGUN_RECOIL = 10;
const real WEAPON_ASSAULTRIFLE_RECOIL = 2;
const real WEAPON_SNIPER_RECOIL = 15;
const real WEAPON_PISTOL_RECOIL = 5;
const int WEAPON_MAX_BPS = 10; // Max/minimum bullets fired per second for assault rifles
const int WEAPON_MIN_BPS = 5;
const real WEAPON_BASE_COST = 200; // How much a weapon costs base - can be more depending on how good the weapon is
const real WEAPON_COST_SPREAD_MULTIPLIER = 0.3; // How much more a weapon can cost (percentage) depending on its spread
const real WEAPON_COST_BPS_MULTIPLIER = 0.3; // How much more a weapon can cost (percentage) depending on its bullets per second
const real WEAPON_COST_DAMAGE_MULTIPLIER = 0.5; // How much more a weapon can cost (percentage) depending on its damage
const real WEAPON_SHOTGUN_SPREAD_ANGLE = VK2D_PI / 3; // Angle the pellets can leer off to
const real WEAPON_DROPOFF = 0.3; // Percent damage lost each pierce
const real WEAPON_SHOTGUN_DELAY = 0.5; // delay in seconds between shots on this weapon
const real WEAPON_SNIPER_DELAY = 1;
const real WEAPON_BULLET_SPEED = 400;
const real WEAPON_SWORD_BULLET_SPEED = 50;
const real WEAPON_BULLET_DECELERATION = 4;
const real WEAPON_BULLET_LIFETIME = 1; // How long before bullets despawn
const real WEAPON_BULLET_SPAWN_DISTANCE = 20;
const real FADE_IN_DURATION = 1; // In seconds
const real MAX_ROT = VK2D_PI * 6; // "Fading in/out" is just rotating/zooming
const real MAX_ZOOM = 1;
const int MAX_ON_HAND_INVENTORY = 20; // Maximum you can hold of any 1 item
const int MIN_ENEMY_SPAWN_DISTANCE = 400; // Nearest and farthest away from the player enemies can spawn
const int MAX_ENEMY_SPAWN_DISTANCE = 600;
const int MAX_MINERAL_SPAWN_DISTANCE = 4000; // How far minerals will spawn from the player
const int MIN_MINERAL_SPAWN_DISTANCE = 300;
const real MINERAL_PICKUP_RANGE = 20; // range at which minerals are "picked up"
const real MINERAL_MOVE_RANGE = 100; // range at which minerals move towards the player
const real MINERAL_MOVE_SPEED = 150; // how fast they move towards the player
const int MAX_MINERAL_PICKUP = 3; // max/min minerals you can pickup at once
const int MIN_MINERAL_PICKUP = 1;
const real MINERAL_DEPOSIT_RANGE = 100;
#define MAX_BULLETS ((int)50) // Maximum number of bullets present at once (because I'm allergic to the hepa)
#define MAX_MINERALS ((int)200) // Max minerals in a world
#define MAX_ENEMIES ((int)30) // Max enemies in a world at once
const real NOTIFICATION_TIME = 2; // time in seconds notifications remain on screen (they fade out for half of this)
const real PLAYER_HEALTHBAR_WIDTH = 40;
const real PLAYER_HEALTHBAR_HEIGHT = 8;
#define MAX_WEAPONS_AT_RINKYS ((int)3) // how many weapons rinky sells at once
const float VOLUME_MUSIC_LEFT = 1;
const float VOLUME_MUSIC_RIGHT = 1;
const float VOLUME_EFFECT_LEFT = 0.5;
const float VOLUME_EFFECT_RIGHT = 0.5;
#define MAX_SHOP_LINES ((int)3)
const real FAME_TO_DOSH_FAME_RATE = 50;
const real FAME_TO_DOSH_DOSH_RATE = 500;
const real CAMERA_ZOOM_DISTANCE = 0.15; // Percent the camera is towards the mouse
const real CAMERA_ZOOM_AIM_DISTANCE = 0.35; // Same as above but when the right mouse button is pressed
real MINIMUM_WEAPON_DAMAGE_PERCENT = 0.75; // the shop should always contain at least this much damage between all weapons

const real STOCK_BASE_PRICE = 5; // Base price of all stocks, they will fluctuate from this
const char *STOCK_NAMES[] = { // Names of the materials you gather
		"Ethro",
		"Lux",
		"Eojamam",
		"Petroleum", // freedom juice
		"Wenrad",
};
const real STOCK_FLUCTUATION[] = { // Percent that they can fluctuate on the market (so for example 0.4 means it can be anywhere from base price - 40% to base price + 40%)
	0.5,
	0.4,
	0.45,
	0.6,
	0.9,
};
#define STOCK_TRADE_COUNT ((int)5) // Number of items that can be traded

// amount equal to difficulty levels
const char *DIFFICULTY_NAMES[] = {
		"Local Solar System",
		"Galactic",
		"Deep Space",
		"The Seventh Circle",
};

const char *WEAPON_NAME_FIRST[] = {
		"Freedom",
		"Liberty",
		"Democracy",
		"Petrol",
		"Calculating",
		"Wenrad",
		"Gnome",
};
const int WEAPON_NAME_FIRST_COUNT = sizeof(WEAPON_NAME_FIRST) / sizeof(const char*);

const char *WEAPON_NAME_SECOND[] = {
		"Disperser",
		"Liberator",
		"Giver",
		"Savage",
		"Destroyer",
		"Hacker",
};
const int WEAPON_NAME_SECOND_COUNT = sizeof(WEAPON_NAME_SECOND) / sizeof(const char*);

const HomeBlocks HOME_WORLD_GRID[] = {
		hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
		hb_MissionSelect, hb_None, hb_None, hb_Stocks, hb_None, hb_None, hb_None, hb_Weapons,
		hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
		hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
		hb_Help, hb_None, hb_None, hb_Memorial, hb_None, hb_None, hb_None, hb_None,
		hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
		hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
		hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None, hb_None,
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
		{"assets/player.png", 0, 0, 16, 24, 0, 1, 8, 12},
		{"assets/yesbutton.png", 0, 0, 50, 50, 0, 3},
		{"assets/stars.png", 0, 0, 29, 29, 0, 4},
		{"assets/launchbutton.png", 0, 0, 58, 58, 0, 3},
		{"assets/buybutton.png", 0, 0, 58, 29, 0, 3},
		{"assets/sellbutton.png", 0, 0, 58, 29, 0, 3},
		{"assets/buy10button.png", 0, 0, 58, 29, 0, 3},
		{"assets/sell10button.png", 0, 0, 58, 29, 0, 3},
		{"assets/shipbutton.png", 0, 0, 75, 75, 0, 3, 0, 0},
		{"assets/purchase.png", 0, 0, 80, 20, 0, 3, 0, 0},
		{"assets/enemy.png", 0, 0, 16, 24, 0.25, 4, 8, 12},
		{"assets/retire.png", 0, 0, 80, 30, 0, 3},
		{"assets/fametodosh.png", 0, 0, 80, 30, 0, 3},
		{"assets/doshtofame.png", 0, 0, 80, 30, 0, 3},
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
		{"assets/stock1.png"},
		{"assets/stock2.png"},
		{"assets/stock3.png"},
		{"assets/stock4.png"},
		{"assets/stock5.png"},
		{"assets/down.png"},
		{"assets/up.png"},
		{"assets/assaultrifle.png"},
		{"assets/onsite.png"},
		{"assets/pistol.png"},
		{"assets/shotgun.png"},
		{"assets/sniper.png"},
		{"assets/sword.png"},
		{"assets/bullet.png"},
		{"assets/whoosh.png"},
		{"assets/compass.png"},
		{"assets/death.png"},
		{"assets/tutorial1.png"},
		{"assets/tutorial2.png"},
		{"assets/deathhighscore.png"},
		{"assets/goofytrack.wav"},
		{"assets/sombertrack.wav"},
		{"assets/newgun.wav"},
		{"assets/whatisawenrad.wav"},
		{"assets/dontturnmypizzainsideout.wav"},
		{"assets/justdontgethit.wav"},
		{"assets/sword.wav"},
		{"assets/pistol.wav"},
		{"assets/shotgun.wav"},
		{"assets/assaultrifle.wav"},
		{"assets/sniper.wav"},
		{"assets/hit.wav"},
		{"assets/mess.wav"},
		{"assets/heavy.wav"},
};
const int ASSET_COUNT = sizeof(ASSETS) / sizeof(JULoadedAsset);
#define PLANET_TEXTURE_COUNT ((int)5)

/********************** Struct **********************/

typedef struct PNLHomeBlock { // Things in the home world for the player to interact with
	HomeBlocks type;
	real x, y;
} PNLHomeBlock;

typedef struct PNLBullet {
	bool active;
	VK2DTexture tex;
	physvec2 pos;
	real direction;
	real lifetime; // Time in seconds until this bullet despawns
	real pierce; // how many enemies this has pierced
	bool canPierce; // only sniper/sword shots "pierce"
	real damage;
	real velocity;
} PNLBullet;

typedef struct PNLWeapon {
	real weaponDamage;
	real weaponPellets; // Number of pellets for a shotgun
	real weaponCost;
	real weaponBPS;
	int weaponNameFirstIndex; // Indices for random weapon names
	int weaponNameSecondIndex;
	vec4 weaponColourMod;
	WeaponType weaponType;
	real cooldown; // For weapons with set ROF
} PNLWeapon;

typedef struct PNLPlayer {
	physvec2 pos;
	physvec2 velocity;
	JUSprite sprite;
	PNLWeapon weapon;
	real dosh;
	real fame;
	real hp;
	int kills;
	real hitcooldown;
} PNLPlayer;

const PNLPlayer PLAYER_DEFAULT_STATE = {
		{300, 200},
		{0, 0},
		NULL,
		{},
		800,
		0,
		PLAYER_MAX_HP,
};

typedef struct PNLStockMarket {
	real stockCosts[STOCK_TRADE_COUNT]; // What each stock costs
	real previousCosts[STOCK_TRADE_COUNT]; // What they were yesterday
	int stockOwned[STOCK_TRADE_COUNT]; // What the player owns
} PNLStockMarket;

typedef struct PNLEnemy {
	real x, y;
	real dosh;
	real fame;
	real hp;
	vec4 colour;
	bool active;
} PNLEnemy;

// Information of a planet the sprite might go to
typedef struct PNLPlanetSpecs {
	real fameBonus;
	real doshCost;
	PlanetDifficulty planetDifficulty;
	int planetTexIndex;
	int planetNameIndex;
} PNLPlanetSpecs;

// For taking goods from planets to the ship
typedef struct PNLInventory {
	int onHandInventory[STOCK_TRADE_COUNT];
	int onShipInventory[STOCK_TRADE_COUNT];
} PNLInventory;

typedef struct PNLHome {
	// just maximum size because w/e
	PNLHomeBlock blocks[HOME_WORLD_GRID_WIDTH * HOME_WORLD_GRID_HEIGHT];
	int size;
} PNLHome;

typedef struct PNLMineral {
	physvec2 pos;
	int stockIndex;
	bool active;
	real randomSeed;
} PNLMineral;

// Information of the planet the sprite is on
typedef struct PNLPlanet {
	PNLPlanetSpecs spec; // Spec this planet comes from
	PNLMineral minerals[MAX_MINERALS];
	PNLEnemy enemies[MAX_ENEMIES];
	real enemySpawnDelay;
	real enemySpawnDelayPrevious;
	PNLInventory inventory;
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
	VK2DTexture texDown;
	VK2DTexture texUp;
	VK2DTexture texPlanets[PLANET_TEXTURE_COUNT];
	VK2DTexture texStocks[STOCK_TRADE_COUNT];
	JUSprite sprButtonLaunch;
	JUSprite sprStars;
	JUSprite sprButtonYes;
	JUSprite sprButtonBuy;
	JUSprite sprButtonSell;
	JUSprite sprButtonBuy10;
	JUSprite sprButtonSell10;
	JUSprite sprButtonShip;
	JUSprite sprButtonPurchase;
	JUSprite sprButtonRetire;
	JUSprite sprButtonFameToDosh;
	JUSprite sprButtonDoshToFame;
	VK2DTexture texAssaultRifle;
	VK2DTexture bgOnsite;
	VK2DTexture texPistol;
	VK2DTexture texShotgun;
	VK2DTexture texSniper;
	VK2DTexture texSword;
	VK2DTexture texBullet;
	VK2DTexture texWhoosh;
	VK2DTexture texCompass;
	VK2DTexture texDeathScreen;
	VK2DTexture texHighscoreScreen;
	VK2DTexture texTutorial1;
	VK2DTexture texTutorial2;
	JUSprite sprEnemy;
	JUFont fntOverlay;
	JUSound sndMusicSomber;
	JUSound sndMusicGoofy;
	JUSound sndNewGun;
	JUSound sndPistol;
	JUSound sndShotgun;
	JUSound sndAssaultRifle;
	JUSound sndSniper;
	JUSound sndHit;
	JUSound sndShop[MAX_SHOP_LINES]; // 3 shop sound effect
	JUSound sndSword;
	JUSound sndMusicMess;
	JUSound sndHeavyDrum;
} PNLAssets;

typedef struct PNLRuntime {
	PNLPlayer player;
	PNLStockMarket market;
	JUSave save;
	int tutorialPage;

	// Current planet, only matters if out on an expedition
	PNLPlanet planet;
	PNLPlanetSpecs potentialPlanets[GENERATED_PLANET_COUNT];
	bool onSite; // on a planet or not
	PNLHome home; // home area

	// All assets
	JULoader loader;
	PNLAssets assets;

	// For fading in/out
	bool fadeIn, fadeOut;
	real fadeClock; // Counts up to FADE_IN_DURATION
	bool deathCooldown;
	bool highscore;
	bool weaponLastFrame; // For playing sounds when walking into the shop
	bool weaponThisFrame;

	// Weapon store
	PNLWeapon shop[MAX_WEAPONS_AT_RINKYS];

	// For notifications
	real notificationTime;
	const char *notificationMessage;

	// Bullets
	PNLBullet bullets[MAX_BULLETS];
	int bulletIndex;

	// Window interface stuff
	float mouseX, mouseY; // Mouse x/y in the game world - not the window relative
	bool mouseLPressed, mouseLReleased, mouseLHeld;
	bool mouseRPressed, mouseRReleased, mouseRHeld;
	bool mouseMPressed, mouseMReleased, mouseMHeld;
	real time; // time in seconds since the program started
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

real clamp(real a, real low, real high) {
	if (a < low)
		return low;
	if (a > high)
		return high;
	return a;
}

real roundTo(real a, real to) {
	return floor(a / to) * to;
}

real randr() { // Returns a real from 0 - 1
	return (real)rand() / (real)RAND_MAX;
}

bool weightedChance(real percent) { // 70% is 0.7
	return randr() < percent;
}

// Returns true if the player can afford a purchase, removing the money if so
bool pnlPlayerPurchase(PNLRuntime game, real dosh) {
	if (game->player.dosh >= dosh) {
		game->player.dosh -= dosh;
		return true;
	} else {
		return false;
	}
}
void pnlSetNotification(PNLRuntime game, const char *string);

/********************** Drawing/Updating Terminal Menus **********************/

// Sprite should have 3 frames - normal, mouse over, and pressed
// Returns true if the button has been pressed
bool pnlDrawButton(PNLRuntime game, JUSprite button, real x, real y) {
	JURectangle r = {x, y, button->Internal.w, button->Internal.h};
	bool mouseOver = juPointInRectangle(&r, game->mouseX, game->mouseY);
	bool pressed = mouseOver && game->mouseLHeld;
	button->rotation = 0;
	juSpriteDrawFrame(button, mouseOver ? (pressed ? 2 : 1) : 0, x, y);
	return mouseOver && game->mouseLReleased;
}

// Same as above but can only be clicked if a condition is met
bool pnlDrawButtonExt(PNLRuntime game, JUSprite button, real x, real y, bool condition) {
	JURectangle r = {x, y, button->Internal.w, button->Internal.h};
	bool mouseOver = juPointInRectangle(&r, game->mouseX, game->mouseY);
	bool pressed = mouseOver && game->mouseLHeld;
	button->rotation = 0;
	if (condition)
		juSpriteDrawFrame(button, mouseOver ? (pressed ? 2 : 1) : 0, x, y);
	else
		juSpriteDrawFrame(button, 0, x, y);
	return mouseOver && game->mouseLReleased && condition;
}

void pnlDrawHealthbar(PNLRuntime game, real percent, vec4 colour, float x, float y, float w, float h) {
	vk2dRendererSetColourMod(VK2D_WHITE);
	vk2dDrawRectangle(x, y, w, h);
	vk2dRendererSetColourMod(colour);
	vk2dDrawRectangle(x + 1, y + 1, (w - 2) * percent, h - 2);
	vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
}

void pnlDrawWeaponStats(PNLRuntime game, PNLWeapon weapon, float x, float y) {
	juFontDraw(game->assets.fntOverlay, x + 3, y, "%s %s", WEAPON_NAME_FIRST[weapon.weaponNameFirstIndex], WEAPON_NAME_SECOND[weapon.weaponNameSecondIndex]);
	juFontDraw(game->assets.fntOverlay, x + 5, y + 20, "$%.2f", weapon.weaponCost);
	y += 20;
	x += 20;
	vk2dRendererSetColourMod(weapon.weaponColourMod);
	if (weapon.weaponType == wt_AssaultRifle) {
		vk2dDrawTextureExt(game->assets.texAssaultRifle, x - 20, y + 30, 3, 3, 0, 0, 0);
		vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
		juFontDraw(game->assets.fntOverlay, x + 10, y + 60, "Damage");
		pnlDrawHealthbar(game, ((weapon.weaponDamage / WEAPON_ASSAULTRIFLE_DAMAGE_MULTIPLIER) - WEAPON_MIN_DAMAGE) / (WEAPON_MAX_DAMAGE - WEAPON_MIN_DAMAGE), VK2D_RED, x + 10, y + 90, 80, 10);
		juFontDraw(game->assets.fntOverlay, x + 10, y + 105, "RPM");
		pnlDrawHealthbar(game, (weapon.weaponBPS - WEAPON_MIN_BPS) / (WEAPON_MAX_BPS - WEAPON_MIN_BPS), VK2D_GREEN, x + 10, y + 135, 80, 10);
	} else if (weapon.weaponType == wt_Sniper) {
		vk2dDrawTextureExt(game->assets.texSniper, x - 20, y + 30, 3, 3, 0, 0, 0);
		vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
		juFontDraw(game->assets.fntOverlay, x + 10, y + 60, "Damage");
		pnlDrawHealthbar(game, ((weapon.weaponDamage / WEAPON_SNIPER_DAMAGE_MULTIPLIER) - WEAPON_MIN_DAMAGE) / (WEAPON_MAX_DAMAGE - WEAPON_MIN_DAMAGE), VK2D_RED, x + 10, y + 90, 80, 10);
	} else if (weapon.weaponType == wt_Shotgun) {
		vk2dDrawTextureExt(game->assets.texShotgun, x - 20, y + 30, 3, 3, 0, 0, 0);
		vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
		juFontDraw(game->assets.fntOverlay, x + 10, y + 60, "Damage");
		pnlDrawHealthbar(game, ((weapon.weaponDamage / WEAPON_SHOTGUN_DAMAGE_MULTIPLIER) - WEAPON_MIN_DAMAGE) / (WEAPON_MAX_DAMAGE - WEAPON_MIN_DAMAGE), VK2D_RED, x + 10, y + 90, 80, 10);
		juFontDraw(game->assets.fntOverlay, x + 10, y + 105, "Spread");
		pnlDrawHealthbar(game, (weapon.weaponPellets - WEAPON_MIN_SPREAD) / (WEAPON_MAX_SPREAD - WEAPON_MIN_SPREAD), VK2D_BLUE, x + 10, y + 135, 80, 10);
	} else if (weapon.weaponType == wt_Sword) {
		vk2dDrawTextureExt(game->assets.texSword, x - 20, y + 30, 3, 3, 0, 0, 0);
		vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
		juFontDraw(game->assets.fntOverlay, x + 10, y + 60, "Damage");
		pnlDrawHealthbar(game, ((weapon.weaponDamage / WEAPON_SWORD_DAMAGE_MULTIPLIER) - WEAPON_MIN_DAMAGE) / (WEAPON_MAX_DAMAGE - WEAPON_MIN_DAMAGE), VK2D_RED, x + 10, y + 90, 80, 10);
	} else if (weapon.weaponType == wt_Pistol) {
		vk2dDrawTextureExt(game->assets.texPistol, x - 20, y + 30, 3, 3, 0, 0, 0);
		vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
		juFontDraw(game->assets.fntOverlay, x + 10, y + 60, "Damage");
		pnlDrawHealthbar(game, ((weapon.weaponDamage / WEAPON_PISTOL_DAMAGE_MULTIPLIER) - WEAPON_MIN_DAMAGE) / (WEAPON_MAX_DAMAGE - WEAPON_MIN_DAMAGE), VK2D_RED, x + 10, y + 90, 80, 10);
	}
}

PNLWeapon pnlGenerateWeapon(PNLRuntime, WeaponType);
TerminalCode pnlUpdateMemorialTerminal(PNLRuntime game) {
	VK2DCamera cam = vk2dRendererGetCamera();
	// Coordinates to start drawing the background - the +3 is to account for the background's frame
	float x = cam.x + (GAME_WIDTH / 2) - (game->assets.bgTerminal->img->width / 2) + 3;
	float y = cam.y + (GAME_HEIGHT / 2) - (game->assets.bgTerminal->img->height / 2) + 3;
	vk2dDrawTexture(game->assets.bgTerminal, x - 3, y - 3);

	if (juSaveKeyExists(game->save, SAVE_HIGHSCORE)) {
		real hs = juSaveGetDouble(game->save, SAVE_HIGHSCORE);
		real dosh = juSaveGetDouble(game->save, SAVE_DOSH);
		real kills = juSaveGetDouble(game->save, SAVE_KILLS);
		const char *planet = juSaveGetString(game->save, SAVE_DEATH_PLANET);
		const char *difficulty = juSaveGetString(game->save, SAVE_DIFFICULTY);
		juFontDraw(game->assets.fntOverlay, x + 10, y - 2, "\n\nHigh score: %.0f fame, $%.0f dosh\nFreedom delivered to: %i aliens\nDied on planet %s\nDistance: %s", hs, dosh, kills, planet, difficulty);
	} else {
		juFontDraw(game->assets.fntOverlay, x + 1, y - 2, "There is no recorded highscore.");
	}
	juFontDraw(game->assets.fntOverlay, x + 3, y + 300 - 75, "Retire if you're out of money or convert %.0f fame to $%.2f dosh", FAME_TO_DOSH_FAME_RATE, FAME_TO_DOSH_DOSH_RATE);
	if (pnlDrawButton(game, game->assets.sprButtonRetire, x + 250 - 127, y + 300 - 43)) {
		pnlSetNotification(game, "Restarted game");
		JUSprite spr = game->player.sprite;
		memcpy(&game->player, &PLAYER_DEFAULT_STATE, sizeof(struct PNLPlayer));
		game->player.sprite = spr;
		game->player.weapon = pnlGenerateWeapon(game, wt_Pistol);
		for (int i = 0; i < STOCK_TRADE_COUNT; i++)
			game->market.stockOwned[i] = 0;
	}
	if (pnlDrawButton(game, game->assets.sprButtonFameToDosh, x + 250 - 127 + 85, y + 300 - 43) && game->player.fame >= FAME_TO_DOSH_FAME_RATE) {
		game->player.fame -= FAME_TO_DOSH_FAME_RATE;
		game->player.dosh += FAME_TO_DOSH_DOSH_RATE;
		pnlSetNotification(game, "Politicans swayed");
		juSoundPlay(game->assets.sndHeavyDrum, false, VOLUME_EFFECT_LEFT, VOLUME_EFFECT_RIGHT);
	}
	if (pnlDrawButton(game, game->assets.sprButtonDoshToFame, x + 250 - 127 + 85 + 85, y + 300 - 43) && pnlPlayerPurchase(game, FAME_TO_DOSH_DOSH_RATE)) {
		game->player.fame += FAME_TO_DOSH_FAME_RATE;
		pnlSetNotification(game, "Politicans swayed");
		juSoundPlay(game->assets.sndHeavyDrum, false, VOLUME_EFFECT_LEFT, VOLUME_EFFECT_RIGHT);
	}

	return tc_NoDraw;
}

void pnlLoadPlanet(PNLRuntime game, int index);
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

		if (pnlDrawButton(game, game->assets.sprButtonLaunch, x + game->assets.bgTerminal->img->width - w - 9, y) && pnlPlayerPurchase(game, game->potentialPlanets[i].doshCost)) {
			pnlLoadPlanet(game, i);
			game->fadeOut = true;
			game->fadeClock = 0;
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

	if (game->tutorialPage % 2 == 0)
		vk2dDrawTexture(game->assets.texTutorial1, x - 4, y - 4);
	else
		vk2dDrawTexture(game->assets.texTutorial2, x - 4, y - 4);

	if (pnlDrawButton(game, game->assets.sprButtonYes, x + 500 - 60, y + 300 - 60))
		game->tutorialPage++;

	return tc_NoDraw;
}

TerminalCode pnlUpdateStocksTerminal(PNLRuntime game) {
	VK2DCamera cam = vk2dRendererGetCamera();
	// Coordinates to start drawing the background - the +3 is to account for the background's frame
	float x = cam.x + (GAME_WIDTH / 2) - (game->assets.bgTerminal->img->width / 2) + 3;
	float y = cam.y + (GAME_HEIGHT / 2) - (game->assets.bgTerminal->img->height / 2) + 3;
	vk2dDrawTexture(game->assets.bgTerminal, x - 3, y - 3);

	// Draw stocks and their info
	float w = game->assets.texStocks[0]->img->width;
	float h = game->assets.texStocks[0]->img->height;
	for (int i = 0; i < STOCK_TRADE_COUNT; i++) {
		float maxCost = (float)STOCK_BASE_PRICE * (1.0f + (float)STOCK_FLUCTUATION[i]);
		float chanceOfGoingUp = (1 - ((float)game->market.stockCosts[i] / (float)maxCost)) * 100.0f;
		vk2dDrawTexture(game->assets.texStocks[i], x + 1, y);
		juFontDraw(game->assets.fntOverlay, x + w + 10, y, "%s | %.0f on hand", STOCK_NAMES[i], (float)game->market.stockOwned[i]);
		juFontDraw(game->assets.fntOverlay, x + w + 10, y + 29, "Market: $%.2f | Chance of Increasing: %0.f%%", (float)game->market.stockCosts[i], chanceOfGoingUp);
		vk2dDrawTexture((game->market.previousCosts[i] < game->market.stockCosts[i] ? game->assets.texUp : game->assets.texDown), x + game->assets.bgTerminal->img->width - w - 9 - 58 - 2 - 30, y);

		if (pnlDrawButton(game, game->assets.sprButtonBuy, x + game->assets.bgTerminal->img->width - w - 9 - 58 - 2, y + 29)) {
			if (pnlPlayerPurchase(game, game->market.stockCosts[i])) {
				game->market.stockOwned[i] += 1;
				pnlSetNotification(game, "Stock sold!");
			}
		}
		if (pnlDrawButton(game, game->assets.sprButtonSell, x + game->assets.bgTerminal->img->width - w - 9, y + 29)) {
			if (game->market.stockOwned[i] > 0) {
				game->market.stockOwned[i] -= 1;
				game->player.dosh += game->market.stockCosts[i];
				pnlSetNotification(game, "Stock sold!");
			}
		}
		if (pnlDrawButton(game, game->assets.sprButtonBuy10, x + game->assets.bgTerminal->img->width - w - 9 - 58 - 2, y)) {
			if (pnlPlayerPurchase(game, game->market.stockCosts[i] * 10)) {
				game->market.stockOwned[i] += 10;
				pnlSetNotification(game, "10 stocks purchased!");
			}
		}
		if (pnlDrawButton(game, game->assets.sprButtonSell10, x + game->assets.bgTerminal->img->width - w - 9, y)) {
			if (game->market.stockOwned[i] >= 10) {
				game->market.stockOwned[i] -= 10;
				game->player.dosh += game->market.stockCosts[i] * 10;
				pnlSetNotification(game, "10 stocks sold!");
			}
		}
		y += h;
	}

	return tc_NoDraw;
}

PNLWeapon pnlGenerateWeapon(PNLRuntime game, WeaponType weaponType);
TerminalCode pnlUpdateWeaponsTerminal(PNLRuntime game) {
	game->weaponThisFrame = true;
	VK2DCamera cam = vk2dRendererGetCamera();
	// Coordinates to start drawing the background - the +3 is to account for the background's frame
	float x = cam.x + (GAME_WIDTH / 2) - (game->assets.bgTerminal->img->width / 2) + 3;
	float y = cam.y + (GAME_HEIGHT / 2) - (game->assets.bgTerminal->img->height / 2) + 3;
	vk2dDrawTexture(game->assets.bgTerminal, x - 3, y - 3);

	for (int i = 0; i < MAX_WEAPONS_AT_RINKYS; i++) {
		pnlDrawWeaponStats(game, game->shop[i], x + 1, y + 1);
		if (pnlDrawButton(game, game->assets.sprButtonPurchase, x + 44, y + 250) && pnlPlayerPurchase(game, game->shop[i].weaponCost)) {
			game->player.weapon = game->shop[i];
			game->shop[i] = pnlGenerateWeapon(game, wt_Any);
			juSoundPlay(game->assets.sndNewGun, false, VOLUME_EFFECT_LEFT, VOLUME_EFFECT_RIGHT);
		}
		x += 500 / MAX_WEAPONS_AT_RINKYS;
	}

	return tc_NoDraw;
}


/********************** Player and other entities **********************/

// Records the new highscore if this score beats the last one - returns true if new highscore
bool pnlRecordHighscore(PNLRuntime game) {
	if (!juSaveKeyExists(game->save, SAVE_HIGHSCORE) || juSaveGetDouble(game->save, SAVE_HIGHSCORE) < game->player.fame) {
		juSaveSetDouble(game->save, SAVE_HIGHSCORE, game->player.fame);
		juSaveSetDouble(game->save, SAVE_DOSH, game->player.dosh);
		juSaveSetDouble(game->save, SAVE_KILLS, (real)game->player.kills);
		juSaveSetString(game->save, SAVE_DEATH_PLANET, PLANET_NAMES[game->planet.spec.planetNameIndex]);
		juSaveSetString(game->save, SAVE_DIFFICULTY, DIFFICULTY_NAMES[game->planet.spec.planetDifficulty - 1]);
		return true;
	}
	return false;
}

// Forward declarations
void pnlDrawWeapon(PNLRuntime game, PNLWeapon wep, float x, float y, float r, float xscale, float yscale);
PNLWeapon pnlGenerateWeapon(PNLRuntime game, WeaponType weaponType);
void pnlCreateBullet(PNLRuntime game, physvec2 pos, real speed, real direction, bool pierce, real damage, VK2DTexture tex);

void _pnlPlayerUpdate(PNLRuntime game, bool drawPlayer) {
	// Handle weapons
	float lookingDir = juPointAngle(game->player.pos.x, game->player.pos.y, game->mouseX, game->mouseY) - (VK2D_PI / 2);

	if (drawPlayer) {
		if (game->player.weapon.weaponType == wt_Shotgun) {
			if (game->player.weapon.cooldown > 0) {
				game->player.weapon.cooldown -= juDelta();
			} else if (game->mouseLPressed) {
				physvec2 pos = {game->player.pos.x + (cos(lookingDir) * WEAPON_BULLET_SPAWN_DISTANCE),
								game->player.pos.y - (sin(lookingDir) * WEAPON_BULLET_SPAWN_DISTANCE)};
				game->player.weapon.cooldown = WEAPON_SHOTGUN_DELAY;
				// Shotguns are guaranteed to fire 1 pellet where they are aimed
				pnlCreateBullet(game, pos, WEAPON_BULLET_SPEED, lookingDir, false, game->player.weapon.weaponDamage,
								game->assets.texBullet);
				for (int i = 0; i < (int) game->player.weapon.weaponPellets - 1; i++)
					pnlCreateBullet(game, pos, WEAPON_BULLET_SPEED,
									lookingDir + sign(randr() - 0.5) * randr() * WEAPON_SHOTGUN_SPREAD_ANGLE, false,
									game->player.weapon.weaponDamage, game->assets.texBullet);
				game->player.velocity.x -= cos(lookingDir) * WEAPON_SHOTGUN_RECOIL;
				game->player.velocity.y += sin(lookingDir) * WEAPON_SHOTGUN_RECOIL;
				juSoundPlay(game->assets.sndShotgun, false, VOLUME_EFFECT_LEFT, VOLUME_EFFECT_RIGHT);
			}
		} else if (game->player.weapon.weaponType == wt_Sniper) {
			if (game->player.weapon.cooldown > 0) {
				game->player.weapon.cooldown -= juDelta();
			} else if (game->mouseLPressed) {
				physvec2 pos = {game->player.pos.x + (cos(lookingDir) * WEAPON_BULLET_SPAWN_DISTANCE),
								game->player.pos.y - (sin(lookingDir) * WEAPON_BULLET_SPAWN_DISTANCE)};
				game->player.weapon.cooldown = WEAPON_SNIPER_DELAY;
				pnlCreateBullet(game, pos, WEAPON_BULLET_SPEED, lookingDir, true, game->player.weapon.weaponDamage,
								game->assets.texBullet);
				game->player.velocity.x -= cos(lookingDir) * WEAPON_SNIPER_RECOIL;
				game->player.velocity.y += sin(lookingDir) * WEAPON_SNIPER_RECOIL;
				juSoundPlay(game->assets.sndSniper, false, VOLUME_EFFECT_LEFT, VOLUME_EFFECT_RIGHT);
			}
		} else if (game->player.weapon.weaponType == wt_AssaultRifle) {
			if (game->player.weapon.cooldown > 0) {
				game->player.weapon.cooldown -= juDelta();
			} else if (game->mouseLHeld) {
				physvec2 pos = {game->player.pos.x + (cos(lookingDir) * WEAPON_BULLET_SPAWN_DISTANCE),
								game->player.pos.y - (sin(lookingDir) * WEAPON_BULLET_SPAWN_DISTANCE)};
				game->player.weapon.cooldown = 1 / game->player.weapon.weaponBPS;
				pnlCreateBullet(game, pos, WEAPON_BULLET_SPEED, lookingDir, false, game->player.weapon.weaponDamage,
								game->assets.texBullet);
				game->player.velocity.x -= cos(lookingDir) * WEAPON_ASSAULTRIFLE_RECOIL;
				game->player.velocity.y += sin(lookingDir) * WEAPON_ASSAULTRIFLE_RECOIL;
				juSoundPlay(game->assets.sndAssaultRifle, false, VOLUME_EFFECT_LEFT, VOLUME_EFFECT_RIGHT);
			}
		} else if (game->player.weapon.weaponType == wt_Pistol) {
			if (game->mouseLPressed) {
				physvec2 pos = {game->player.pos.x + (cos(lookingDir) * WEAPON_BULLET_SPAWN_DISTANCE),
								game->player.pos.y - (sin(lookingDir) * WEAPON_BULLET_SPAWN_DISTANCE)};
				pnlCreateBullet(game, pos, WEAPON_BULLET_SPEED, lookingDir, false, game->player.weapon.weaponDamage,
								game->assets.texBullet);
				game->player.velocity.x -= cos(lookingDir) * WEAPON_PISTOL_RECOIL;
				game->player.velocity.y += sin(lookingDir) * WEAPON_PISTOL_RECOIL;
				juSoundPlay(game->assets.sndPistol, false, VOLUME_EFFECT_LEFT, VOLUME_EFFECT_RIGHT);
			}
		} else if (game->player.weapon.weaponType == wt_Sword) {
			if (game->mouseLPressed) {
				physvec2 pos = {game->player.pos.x + (cos(lookingDir) * WEAPON_BULLET_SPAWN_DISTANCE),
								game->player.pos.y - (sin(lookingDir) * WEAPON_BULLET_SPAWN_DISTANCE)};
				pnlCreateBullet(game, pos, WEAPON_SWORD_BULLET_SPEED, lookingDir, false,
								game->player.weapon.weaponDamage, game->assets.texWhoosh);
				game->player.velocity.x -= cos(lookingDir) * WEAPON_SWORD_RECOIL;
				game->player.velocity.y += sin(lookingDir) * WEAPON_SWORD_RECOIL;
				juSoundPlay(game->assets.sndSword, false, VOLUME_EFFECT_LEFT, VOLUME_EFFECT_RIGHT);
			}
		}
	}

	// Hit cooldown
	game->player.hitcooldown -= juDelta();

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
	game->player.pos.x = clamp(game->player.pos.x, -MAX_MINERAL_SPAWN_DISTANCE, MAX_MINERAL_SPAWN_DISTANCE);
	game->player.pos.y = clamp(game->player.pos.y, -MAX_MINERAL_SPAWN_DISTANCE, MAX_MINERAL_SPAWN_DISTANCE);

	// Draw player
	if (drawPlayer && !(game->player.hitcooldown > 0 && sin((game->time / VK2D_PI) * 4) > 1)) {
		lookingDir += VK2D_PI / 2;
		juSpriteDraw(game->player.sprite, game->player.pos.x, game->player.pos.y);
		pnlDrawWeapon(game, game->player.weapon, game->player.pos.x, game->player.pos.y - 6, sign(lookingDir) == 1 ? -lookingDir + (VK2D_PI / 2) : -lookingDir + (VK2D_PI / 2) - VK2D_PI, sign(lookingDir), 1);
		if (game->onSite) {
			vk2dRendererSetColourMod(VK2D_BLACK);
			vk2dDrawRectangle(game->player.pos.x - (PLAYER_HEALTHBAR_WIDTH / 2), game->player.pos.y - 14 - PLAYER_HEALTHBAR_HEIGHT, PLAYER_HEALTHBAR_WIDTH, PLAYER_HEALTHBAR_HEIGHT);
			vk2dRendererSetColourMod(VK2D_RED);
			vk2dDrawRectangle(game->player.pos.x - (PLAYER_HEALTHBAR_WIDTH / 2) + 1, game->player.pos.y - 13 - PLAYER_HEALTHBAR_HEIGHT, (game->player.hp / PLAYER_MAX_HP) * (PLAYER_HEALTHBAR_WIDTH - 2), PLAYER_HEALTHBAR_HEIGHT - 2);
			vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
		}
	}
}

// Player update - movement and weapons
void pnlPlayerUpdate(PNLRuntime game, bool drawPlayer) {
	if (game->player.hp > 0)
		_pnlPlayerUpdate(game, drawPlayer);
	else if (game->deathCooldown) {
		if (juKeyboardGetKeyPressed(SDL_SCANCODE_SPACE) && !game->fadeOut) {
			game->fadeOut = true;
			game->fadeClock = FADE_IN_DURATION;
		}
	}
}

void pnlSetNotification(PNLRuntime game, const char *string) {
	game->notificationTime = NOTIFICATION_TIME;
	game->notificationMessage = string;
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

// Loads a selected planet spec into the current planet slot
void pnlLoadPlanet(PNLRuntime game, int index) {
	memset(&game->planet, 0, sizeof(struct PNLPlanet));
	game->planet.spec = game->potentialPlanets[index];
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
	real doshMult = pow(DOSH_MULTIPLIER, (real)difficulty);
	real fameMult = pow(FAME_MULTIPLIER, (real)difficulty);

	// Contruct planet specs
	specs.doshCost = (DOSH_PLANET_COST * doshMult) + (DOSH_PLANET_COST_VARIANCE * doshMult * randr());
	specs.fameBonus = (FAME_PER_PLANET * fameMult) + (FAME_VARIANCE * fameMult * randr());
	specs.planetDifficulty = difficulty;
	specs.planetTexIndex = (int)floor(randr() * (real)PLANET_TEXTURE_COUNT);

	// Make unique name
	int chosenName;
	bool nameTaken = true;
	while (nameTaken) {
		chosenName = (int)floor(randr() * (real)PLANET_NAMES_COUNT);
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
	game->weaponLastFrame = game->weaponThisFrame;

	if (block->type == hb_Memorial) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texMemorialTerminal, block->x - (game->assets.texMemorialTerminal->img->width / 2), block->y - (game->assets.texMemorialTerminal->img->height / 2));
		} else if (!game->fadeIn && !game->fadeOut) { // only do terminal things when not fading
			code = pnlUpdateMemorialTerminal(game);
		}
	} else if (block->type == hb_MissionSelect) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texMissionTerminal, block->x - (game->assets.texMissionTerminal->img->width / 2), block->y - (game->assets.texMissionTerminal->img->height / 2));
		} else if (!game->fadeIn && !game->fadeOut) { // only do terminal things when not fading
			code = pnlUpdateMissionSelectTerminal(game);
		}
	} else if (block->type == hb_Help) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texHelpTerminal, block->x - (game->assets.texHelpTerminal->img->width / 2), block->y - (game->assets.texHelpTerminal->img->height / 2));
		} else if (!game->fadeIn && !game->fadeOut) { // only do terminal things when not fading
			code = pnlUpdateHelpTerminal(game);
		}
	} else if (block->type == hb_Stocks) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texStockTerminal, block->x - (game->assets.texStockTerminal->img->width / 2), block->y - (game->assets.texStockTerminal->img->height / 2));
		} else if (!game->fadeIn && !game->fadeOut) { // only do terminal things when not fading
			code = pnlUpdateStocksTerminal(game);
		}
	} else if (block->type == hb_Weapons) {
		if (juPointDistance(game->player.pos.x, game->player.pos.y, block->x, block->y) > IN_RANGE_TERMINAL_DISTANCE) {
			vk2dDrawTexture(game->assets.texWeaponTerminal, block->x - (game->assets.texWeaponTerminal->img->width / 2), block->y - (game->assets.texWeaponTerminal->img->height / 2));
			game->weaponThisFrame = false;
		} else if (!game->fadeIn && !game->fadeOut) { // only do terminal things when not fading
			code = pnlUpdateWeaponsTerminal(game);
		}
	} else {
		game->weaponThisFrame = false;
	}

	if (!game->weaponLastFrame && game->weaponThisFrame)
		juSoundPlay(game->assets.sndShop[(int)floor(randr() * MAX_SHOP_LINES)], false, VOLUME_EFFECT_LEFT, VOLUME_EFFECT_RIGHT);

	return code;
}

// Generates a random weapon, specify wt_Any for a random weapon or wt_whatever for a specific type of weapon
PNLWeapon pnlGenerateWeapon(PNLRuntime game, WeaponType weaponType) {
	PNLWeapon wep = {};

	// Choose random type
	if (weaponType == wt_Any) {
		WeaponType types[] = {wt_AssaultRifle, wt_Shotgun, wt_Sniper, wt_Sword};
		weaponType = types[(int)floor(randr() * 4)];
	}

	// Universal attributes
	wep.weaponColourMod[0] = (float)randr();
	wep.weaponColourMod[1] = (float)randr();
	wep.weaponColourMod[2] = (float)randr();
	wep.weaponColourMod[3] = 1;
	wep.weaponNameFirstIndex = (int)floor(randr() * WEAPON_NAME_FIRST_COUNT);
	wep.weaponNameSecondIndex = (int)floor(randr() * WEAPON_NAME_SECOND_COUNT);
	wep.weaponType = weaponType;
	real bpsPercent = randr();
	wep.weaponBPS = WEAPON_MIN_BPS + ((WEAPON_MAX_BPS - WEAPON_MIN_BPS) * bpsPercent);
	real damagePercent = randr();
	wep.weaponDamage = WEAPON_MIN_DAMAGE + ((WEAPON_MAX_DAMAGE - WEAPON_MIN_DAMAGE) * damagePercent);
	real pelletsPercent = randr();
	wep.weaponPellets = WEAPON_MIN_SPREAD + round((WEAPON_MAX_SPREAD - WEAPON_MIN_SPREAD) * pelletsPercent);

	// Weapon cost is universal even though certain aspects of a weapon are specific to certain weapon types
	real multiplier = 1 + (bpsPercent * WEAPON_COST_BPS_MULTIPLIER) + (damagePercent * WEAPON_COST_DAMAGE_MULTIPLIER) + (pelletsPercent * WEAPON_COST_SPREAD_MULTIPLIER);
	wep.weaponCost = WEAPON_BASE_COST * multiplier;

	// Stuff specific to weapons
	if (weaponType == wt_Pistol) {
		wep.weaponDamage *= WEAPON_PISTOL_DAMAGE_MULTIPLIER;
	} else if (weaponType == wt_Sniper) {
		wep.weaponDamage *= WEAPON_SNIPER_DAMAGE_MULTIPLIER;
	} else if (weaponType == wt_Shotgun) {
		wep.weaponDamage *= WEAPON_SHOTGUN_DAMAGE_MULTIPLIER;
	} else if (weaponType == wt_Sword) {
		wep.weaponDamage *= WEAPON_SWORD_DAMAGE_MULTIPLIER;
	} else if (weaponType == wt_AssaultRifle) {
		wep.weaponDamage *= WEAPON_ASSAULTRIFLE_DAMAGE_MULTIPLIER;
	}

	return wep;
}

void pnlDrawWeapon(PNLRuntime game, PNLWeapon wep, float x, float y, float r, float xscale, float yscale) {
	VK2DTexture tex;
	if (wep.weaponType == wt_Pistol)
		tex = game->assets.texPistol;
	else if (wep.weaponType == wt_AssaultRifle)
		tex = game->assets.texAssaultRifle;
	else if (wep.weaponType == wt_Sword)
		tex = game->assets.texSword;
	else if (wep.weaponType == wt_Shotgun)
		tex = game->assets.texShotgun;
	else
		tex = game->assets.texSniper;
	vk2dRendererSetColourMod(wep.weaponColourMod);
	vk2dRendererDrawTexture(tex, x, y, xscale, yscale, r, 0, tex->img->height / 2, 0, 0, tex->img->width, tex->img->height);
	vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
}

void pnlCreateBullet(PNLRuntime game, physvec2 pos, real speed, real direction, bool pierce, real damage, VK2DTexture tex) {
	PNLBullet *bullet = &game->bullets[game->bulletIndex++ % MAX_BULLETS];
	bullet->active = true;
	bullet->pos = pos;
	bullet->tex = tex;
	bullet->canPierce = pierce;
	bullet->damage = damage;
	bullet->direction = direction;
	bullet->lifetime = 0;
	bullet->velocity = speed;
}

void pnlUpdateBullets(PNLRuntime game) {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (game->bullets[i].active) {
			PNLBullet *b = &game->bullets[i];
			b->pos.x += cos(b->direction) * b->velocity * juDelta();
			b->pos.y -= sin(b->direction) * b->velocity * juDelta();
			b->velocity -= WEAPON_BULLET_DECELERATION * juDelta();
			vec4 c = {1, 1, 1, 1 - (b->lifetime / WEAPON_BULLET_LIFETIME)};
			vk2dRendererSetColourMod(c);
			vk2dRendererDrawTexture(b->tex, b->pos.x - b->tex->img->width / 2, b->pos.y - b->tex->img->height / 2, 1, 1, (VK2D_PI / 2) - b->direction + (VK2D_PI / 2), b->tex->img->width / 2, b->tex->img->height / 2, 0, 0, b->tex->img->width, b->tex->img->height);
			vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);

			b->lifetime += juDelta();
			if (b->lifetime >= WEAPON_BULLET_LIFETIME) {
				b->active = false;
			}

			for (int j = 0; j < MAX_ENEMIES; j++) {
				if (game->planet.enemies[j].active && juPointDistance(game->planet.enemies[j].x, game->planet.enemies[j].y, game->bullets[i].pos.x, game->bullets[i].pos.y) <= ENEMY_HIT_DISTANCE) {
					game->planet.enemies[j].hp -= game->bullets[i].damage;
					if (game->bullets[i].canPierce) {
						game->bullets[i].damage *= 1 - WEAPON_DROPOFF;
					} else {
						game->bullets[i].active = false;
					}
				}
			}
		}
	}
}

void pnlDrawTitleBar(PNLRuntime game) {
	VK2DCamera cam = vk2dRendererGetCamera();
	vk2dRendererSetColourMod(VK2D_BLACK);
	vk2dDrawRectangle(cam.x, cam.y, cam.w, 20);
	vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
	if (game->onSite)
		juFontDraw(game->assets.fntOverlay, cam.x, cam.y - 5, "%s | Dosh: $%.2f | Fame: %.0f | %s | [on-hand/on-ship]", PLANET_NAMES[game->planet.spec.planetNameIndex], (float)game->player.dosh, (float)game->player.fame, VERSION_STRING);
	else
		juFontDraw(game->assets.fntOverlay, cam.x, cam.y - 5, "Home | Dosh: $%.2f | Fame: %.0f | %s", (float)game->player.dosh, (float)game->player.fame, VERSION_STRING);

	// Draw notification at top left (after compass)
	if (game->notificationTime > 0) {
		vec4 cyan = {0, 1, 1, 1};
		game->notificationTime -= juDelta();
		if (game->notificationTime < NOTIFICATION_TIME / 2) {
			cyan[3] = (game->notificationTime) / (NOTIFICATION_TIME / 2);
			cyan[3] = cyan[3] < 0 ? 0 : cyan[3];
		}
		vk2dRendererSetColourMod(cyan);
		juFontDraw(game->assets.fntOverlay, cam.x + 38, cam.y + 18, "%s", game->notificationMessage);
		vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
	}
}

void pnlLoadMineralsIntoShip(PNLRuntime game) {
	for (int i = 0; i < STOCK_TRADE_COUNT; i++) {
		game->planet.inventory.onShipInventory[i] += game->planet.inventory.onHandInventory[i];
		game->planet.inventory.onHandInventory[i] = 0;
	}
	pnlSetNotification(game, "Minerals loaded!");
}

void pnlDrawMineralOverlay(PNLRuntime game) {
	VK2DCamera cam = vk2dRendererGetCamera();
	float x = cam.x;
	float y = cam.y + cam.h - 29;
	vk2dRendererSetColourMod(VK2D_BLACK);
	vk2dDrawRectangle(x, y, cam.w, 29);
	vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
	for (int i = 0; i < STOCK_TRADE_COUNT; i++) {
		vk2dDrawTextureExt(game->assets.texStocks[i], x, y, 0.5, 0.5, 0, 0, 0);
		juFontDraw(game->assets.fntOverlay, x + 35, y, "%i/%i", game->planet.inventory.onHandInventory[i], game->planet.inventory.onShipInventory[i]);
		x += 120;
	}

	vk2dDrawTexture(game->assets.texCompass, cam.x + 4, cam.y + 20 + 4);
	vk2dRendererSetColourMod(VK2D_RED);
	float dir = juPointAngle(game->player.pos.x, game->player.pos.y, 0, 0) - (VK2D_PI / 2);
	vk2dDrawLine(cam.x + 16 + 4, cam.y + 20 + 16 + 4, 4 + cam.x + 16 + (cos(dir) * 13), 4 + cam.y + 16 + 20 - (sin(dir) * 13));
	vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
}

void pnlUpdateMinerals(PNLRuntime game) {
	for (int i = 0; i < MAX_MINERALS; i++) {

		if (game->planet.minerals[i].active) {
			float dist = juPointDistance(game->planet.minerals[i].pos.x, game->planet.minerals[i].pos.y, game->player.pos.x, game->player.pos.y);
			if (dist < GAME_WIDTH) {
				vk2dDrawTextureExt(game->assets.texStocks[game->planet.minerals[i].stockIndex], game->planet.minerals[i].pos.x - 7, game->planet.minerals[i].pos.y - 15 - (sin(game->time + game->planet.minerals[i].randomSeed) * 3), 0.25, 0.25, 0, 0, 0);

				if (dist <= MINERAL_MOVE_RANGE) {
					real angle = juPointAngle(game->planet.minerals[i].pos.x, game->planet.minerals[i].pos.y, game->player.pos.x, game->player.pos.y) - (VK2D_PI / 2);
					game->planet.minerals[i].pos.x += cos(angle) * MINERAL_MOVE_SPEED * juDelta();
					game->planet.minerals[i].pos.y -= sin(angle) * MINERAL_MOVE_SPEED * juDelta();
				}

				if (dist <= MINERAL_PICKUP_RANGE && game->planet.inventory.onHandInventory[game->planet.minerals[i].stockIndex] < MAX_ON_HAND_INVENTORY) {
					game->planet.minerals[i].active = false;
					game->planet.inventory.onHandInventory[game->planet.minerals[i].stockIndex] += MIN_MINERAL_PICKUP + round((real)(MAX_MINERAL_PICKUP - MIN_MINERAL_PICKUP) * randr());
					game->planet.inventory.onHandInventory[game->planet.minerals[i].stockIndex] = clamp(game->planet.inventory.onHandInventory[game->planet.minerals[i].stockIndex], 0, 20);

					pnlSetNotification(game, "Grabbed minerals");
				}
			}
		}
	}
}

void pnlCreateEnemy(PNLRuntime game) {
	PNLEnemy *enemy = NULL;
	for (int i = 0; i < MAX_ENEMIES; i++)
		if (!game->planet.enemies[i].active)
			enemy = &game->planet.enemies[i];

	if (enemy != NULL) { // Creates an enemy with random attributes (check constants at top for ranges)
		enemy->active = true;
		enemy->colour[0] = randr();
		enemy->colour[1] = randr();
		enemy->colour[2] = randr();
		enemy->colour[3] = 1;
		enemy->dosh = (pow(ENEMY_DOSH_MULTIPLIER, game->planet.spec.planetDifficulty) * ENEMY_MIN_DOSH) + (((pow(ENEMY_DOSH_MULTIPLIER, game->planet.spec.planetDifficulty) * ENEMY_MAX_DOSH) - (pow(ENEMY_DOSH_MULTIPLIER, game->planet.spec.planetDifficulty) * ENEMY_MIN_DOSH)) * randr());
		enemy->fame = (pow(ENEMY_FAME_MULTIPLIER, game->planet.spec.planetDifficulty) * ENEMY_MIN_FAME) + (((pow(ENEMY_FAME_MULTIPLIER, game->planet.spec.planetDifficulty) * ENEMY_MAX_FAME) - (pow(ENEMY_FAME_MULTIPLIER, game->planet.spec.planetDifficulty) * ENEMY_MIN_FAME)) * randr());
		enemy->hp = (pow(ENEMY_HP_MULTIPLIER, game->planet.spec.planetDifficulty) * ENEMY_HP) + ((pow(ENEMY_HP_MULTIPLIER, game->planet.spec.planetDifficulty) * ENEMY_HP) * sign(randr() - 0.5) * ENEMY_HP_VARIANCE);
		float angle = randr() * VK2D_PI * 2;
		float distance = MIN_ENEMY_SPAWN_DISTANCE + ((MAX_ENEMY_SPAWN_DISTANCE - MIN_ENEMY_SPAWN_DISTANCE) * randr());
		enemy->x = cos(angle) * distance;
		enemy->y = -sin(angle) * distance;
	}
}

void pnlUpdateEnemies(PNLRuntime game) {
	if (game->planet.enemySpawnDelay > 0) {
		game->planet.enemySpawnDelay -= juDelta();
		if (game->planet.enemySpawnDelay <= 0) {
			game->planet.enemySpawnDelay = game->planet.enemySpawnDelayPrevious * (1 - ENEMY_SPAWN_DELAY_DECREASE);
			game->planet.enemySpawnDelayPrevious = game->planet.enemySpawnDelay;
			pnlCreateEnemy(game);
		}
	}

	// Move enemies towards player and draw
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (game->planet.enemies[i].active) {
			if (game->planet.enemies[i].hp <= 0) { // Kill enemies
				game->planet.enemies[i].active = false;
				game->player.dosh += game->planet.enemies[i].dosh;
				game->player.fame += game->planet.enemies[i].fame;
				game->player.kills += 1;
			} else {
				float angle = juPointAngle(game->planet.enemies[i].x, game->planet.enemies[i].y, game->player.pos.x, game->player.pos.y) - (VK2D_PI / 2);
				float distance = juPointDistance(game->planet.enemies[i].x, game->planet.enemies[i].y, game->player.pos.x, game->player.pos.y);

				if (distance > (float)GAME_WIDTH * 1.5) { // Move double speed when outside player view
					game->planet.enemies[i].x +=
							cos(angle) * ENEMY_SPEED * pow(ENEMY_SPEED_MULTIPLIER, game->planet.spec.planetDifficulty) *
							juDelta() * 4;
					game->planet.enemies[i].y -=
							sin(angle) * ENEMY_SPEED * pow(ENEMY_SPEED_MULTIPLIER, game->planet.spec.planetDifficulty) *
							juDelta() * 4;
				} else {
					game->planet.enemies[i].x +=
							cos(angle) * ENEMY_SPEED * pow(ENEMY_SPEED_MULTIPLIER, game->planet.spec.planetDifficulty) *
							juDelta();
					game->planet.enemies[i].y -=
							sin(angle) * ENEMY_SPEED * pow(ENEMY_SPEED_MULTIPLIER, game->planet.spec.planetDifficulty) *
							juDelta();
				}

				if (distance < ENEMY_HIT_DISTANCE && game->player.hitcooldown <= 0 && !game->fadeOut) {
					real mult = pow(ENEMY_DAMAGE_MULTIPLIER, (real)game->planet.spec.planetDifficulty);
					game->player.hp -= (ENEMY_DAMAGE * mult) + (sign(randr() - 0.5) * ENEMY_DAMAGE_VARIANCE * ENEMY_DAMAGE * randr());
					juSoundPlay(game->assets.sndHit, false, VOLUME_EFFECT_LEFT, VOLUME_EFFECT_RIGHT);
					game->player.hitcooldown = ENEMY_HIT_DELAY;

					if (game->player.hp <= 0) {
						game->highscore = pnlRecordHighscore(game);
						game->deathCooldown = true;
					}
				}

				vk2dRendererSetColourMod(game->planet.enemies[i].colour);
				juSpriteDraw(game->assets.sprEnemy, game->planet.enemies[i].x, game->planet.enemies[i].y);
				vk2dRendererSetColourMod(VK2D_DEFAULT_COLOUR_MOD);
			}
		}
	}
}

/********************** Functions specific to regions **********************/
void pnlInitHome(PNLRuntime game) {
	for (int i = 0; i < GENERATED_PLANET_COUNT; i++)
		game->potentialPlanets[i] = pnlCreatePlanetSpec(game);
	for (int i = 0; i < STOCK_TRADE_COUNT; i++) {
		game->market.previousCosts[i] = game->market.stockCosts[i];
		real mult = weightedChance(0.5) ? -1 : 1; // 50/50 it goes up or down
		game->market.stockCosts[i] = STOCK_BASE_PRICE * (1 + (mult * (STOCK_FLUCTUATION[i] * randr())));
	}
	game->player.hp = PLAYER_MAX_HP;
	game->player.pos.x = PLAYER_DEFAULT_STATE.pos.x;
	game->player.pos.y = PLAYER_DEFAULT_STATE.pos.y;
	game->highscore = false;

	// Make sure that there is at least MINIMUM_WEAPON_DAMAGE_PERCENT damage between all weapons generated for the shop
	real totalWeaponDamagePercent = 0;
	while (totalWeaponDamagePercent < MINIMUM_WEAPON_DAMAGE_PERCENT) {
		totalWeaponDamagePercent = 0;
		for (int i = 0; i < MAX_WEAPONS_AT_RINKYS; i++) {
			game->shop[i] = pnlGenerateWeapon(game, wt_Any);
			totalWeaponDamagePercent += (game->shop[i].weaponDamage - WEAPON_MIN_DAMAGE) / WEAPON_MAX_DAMAGE;
		}
	}

	// Music
	juSoundStopAll();
	if (weightedChance(0.5))
		juSoundPlay(game->assets.sndMusicGoofy, false, VOLUME_MUSIC_LEFT, VOLUME_MUSIC_RIGHT);
	else
		juSoundPlay(game->assets.sndMusicSomber, false, VOLUME_MUSIC_LEFT, VOLUME_MUSIC_RIGHT);
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
	pnlUpdateBullets(game);

	// Overlay
	pnlDrawTitleBar(game);

	// Handle fade
	if (game->fadeOut) {
		if (game->fadeClock >= FADE_IN_DURATION) {
			code = tc_Goto;
			game->fadeOut = false;
		}
		game->fadeClock += juDelta();
	}

	if (code == tc_Goto)
		return ws_Offsite;
	else
		return ws_Home;
}

void pnlQuitHome(PNLRuntime game) {

}

void pnlInitPlanet(PNLRuntime game) {
	game->fadeIn = true;
	game->fadeClock = 0;

	for (int i = 0; i < STOCK_TRADE_COUNT; i++) {
		game->planet.inventory.onShipInventory[i] = 0;
		game->planet.inventory.onHandInventory[i] = 0;
	}
	game->player.pos.x = 150;
	game->player.pos.y = 150;

	// Scatter the minerals all around the place
	for (int i = 0; i < MAX_MINERALS; i++) {
		game->planet.minerals[i].active = true;
		game->planet.minerals[i].pos.x = sign(randr() - 0.5) * (MIN_MINERAL_SPAWN_DISTANCE + ((MAX_MINERAL_SPAWN_DISTANCE - MIN_MINERAL_SPAWN_DISTANCE) * randr()));
		game->planet.minerals[i].pos.y = sign(randr() - 0.5) * (MIN_MINERAL_SPAWN_DISTANCE + ((MAX_MINERAL_SPAWN_DISTANCE - MIN_MINERAL_SPAWN_DISTANCE) * randr()));
		game->planet.minerals[i].stockIndex = (int)floor(randr() * STOCK_TRADE_COUNT);
		game->planet.minerals[i].randomSeed = randr() * 10000;
	}

	// Reset enemy stuff
	game->planet.enemySpawnDelayPrevious = ENEMY_SPAWN_DELAY;
	game->planet.enemySpawnDelay = ENEMY_SPAWN_DELAY;
	memset(game->planet.enemies, 0, sizeof(struct PNLEnemy) * MAX_ENEMIES);

	// Music
	juSoundStopAll();
	juSoundPlay(game->assets.sndMusicMess, true, VOLUME_MUSIC_LEFT, VOLUME_MUSIC_RIGHT);
}

WorldSelection pnlUpdatePlanet(PNLRuntime game) {
	// Draw background and ship
	pnlDrawTiledBackground(game, game->assets.bgOnsite);
	if (pnlDrawButtonExt(game, game->assets.sprButtonShip, 0, 0, juPointDistance(game->player.pos.x, game->player.pos.y, 75/2, 75/2) < IN_RANGE_TERMINAL_DISTANCE * 2) && !game->fadeOut) {
		game->fadeClock = 0;
		game->fadeOut = true;
		pnlLoadMineralsIntoShip(game);
	}

	// Update player bullets minerals and enemies
	pnlUpdateMinerals(game);
	pnlPlayerUpdate(game, true);
	pnlUpdateBullets(game);
	pnlUpdateEnemies(game);
	if (juPointDistance(game->player.pos.x, game->player.pos.y, 39, 39) < MINERAL_DEPOSIT_RANGE) {
		pnlLoadMineralsIntoShip(game);
	}

	// Draw title and the element overlay
	pnlDrawTitleBar(game);
	pnlDrawMineralOverlay(game);

	// Draw death overlay
	if (game->deathCooldown) {
		VK2DCamera cam = vk2dRendererGetCamera();
		// Coordinates to start drawing the background - the +3 is to account for the background's frame
		float x = cam.x + (GAME_WIDTH / 2) - (game->assets.bgTerminal->img->width / 2) + 3;
		float y = cam.y + (GAME_HEIGHT / 2) - (game->assets.bgTerminal->img->height / 2) + 3;
		vk2dDrawTexture((game->highscore ? game->assets.texHighscoreScreen : game->assets.texDeathScreen), x - 3, y - 3);

	}

	// DEBUG
	if (juKeyboardGetKey(SDL_SCANCODE_BACKSPACE)) {
		game->fadeOut = true;
		game->fadeClock = 0;
	}

	// Handle fade in/out
	if (game->fadeIn) {
		if (game->fadeClock >= FADE_IN_DURATION)
			game->fadeIn = false;
		game->fadeClock += juDelta();
	} else if (game->fadeOut) {
		if (game->fadeClock >= FADE_IN_DURATION) {
			// if the player died we need to reset the player
			if (game->deathCooldown) {
				JUSprite spr = game->player.sprite;
				memcpy(&game->player, &PLAYER_DEFAULT_STATE, sizeof(struct PNLPlayer));
				game->player.sprite = spr;
				game->player.weapon = pnlGenerateWeapon(game, wt_Pistol);
				for (int i = 0; i < STOCK_TRADE_COUNT; i++)
					game->market.stockOwned[i] = 0;
			}

			game->fadeOut = false;
			return ws_Home;
		}
		game->fadeClock += juDelta();
	}

	return ws_Offsite;
}

void pnlQuitPlanet(PNLRuntime game) {
	for (int i = 0; i < STOCK_TRADE_COUNT; i++)
		game->market.stockOwned[i] += game->planet.inventory.onShipInventory[i];
	game->player.fame += game->planet.spec.fameBonus;
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
	game->assets.texDown = juLoaderGetTexture(game->loader, "assets/down.png");
	game->assets.texUp = juLoaderGetTexture(game->loader, "assets/up.png");
	game->assets.bgTerminal = juLoaderGetTexture(game->loader, "assets/terminalbg.png");
	game->assets.sprButtonYes = juLoaderGetSprite(game->loader, "assets/yesbutton.png");
	game->assets.texPlanets[0] = juLoaderGetTexture(game->loader, "assets/planet1.png");
	game->assets.texPlanets[1] = juLoaderGetTexture(game->loader, "assets/planet2.png");
	game->assets.texPlanets[2] = juLoaderGetTexture(game->loader, "assets/planet3.png");
	game->assets.texPlanets[3] = juLoaderGetTexture(game->loader, "assets/planet4.png");
	game->assets.texPlanets[4] = juLoaderGetTexture(game->loader, "assets/planet5.png");
	game->assets.texStocks[0] = juLoaderGetTexture(game->loader, "assets/stock1.png");
	game->assets.texStocks[1] = juLoaderGetTexture(game->loader, "assets/stock2.png");
	game->assets.texStocks[2] = juLoaderGetTexture(game->loader, "assets/stock3.png");
	game->assets.texStocks[3] = juLoaderGetTexture(game->loader, "assets/stock4.png");
	game->assets.texStocks[4] = juLoaderGetTexture(game->loader, "assets/stock5.png");
	game->assets.sprButtonLaunch = juLoaderGetSprite(game->loader, "assets/launchbutton.png");
	game->assets.sprButtonBuy = juLoaderGetSprite(game->loader, "assets/buybutton.png");
	game->assets.sprButtonSell = juLoaderGetSprite(game->loader, "assets/sellbutton.png");
	game->assets.sprButtonBuy10 = juLoaderGetSprite(game->loader, "assets/buy10button.png");
	game->assets.sprButtonSell10 = juLoaderGetSprite(game->loader, "assets/sell10button.png");
	game->assets.sprButtonShip = juLoaderGetSprite(game->loader, "assets/shipbutton.png");
	game->assets.sprStars = juLoaderGetSprite(game->loader, "assets/stars.png");
	game->assets.sprButtonRetire = juLoaderGetSprite(game->loader, "assets/retire.png");
	game->assets.sprButtonFameToDosh = juLoaderGetSprite(game->loader, "assets/fametodosh.png");
	game->assets.sprButtonDoshToFame = juLoaderGetSprite(game->loader, "assets/doshtofame.png");
	game->assets.texAssaultRifle = juLoaderGetTexture(game->loader, "assets/assaultrifle.png");
	game->assets.bgOnsite = juLoaderGetTexture(game->loader, "assets/onsite.png");
	game->assets.texPistol = juLoaderGetTexture(game->loader, "assets/pistol.png");
	game->assets.texShotgun = juLoaderGetTexture(game->loader, "assets/shotgun.png");
	game->assets.texSniper = juLoaderGetTexture(game->loader, "assets/sniper.png");
	game->assets.texSword = juLoaderGetTexture(game->loader, "assets/sword.png");
	game->assets.texBullet = juLoaderGetTexture(game->loader, "assets/bullet.png");
	game->assets.texWhoosh = juLoaderGetTexture(game->loader, "assets/whoosh.png");
	game->assets.texCompass = juLoaderGetTexture(game->loader, "assets/compass.png");
	game->assets.texDeathScreen = juLoaderGetTexture(game->loader, "assets/death.png");
	game->assets.texHighscoreScreen = juLoaderGetTexture(game->loader, "assets/deathhighscore.png");
	game->assets.texTutorial1 = juLoaderGetTexture(game->loader, "assets/tutorial1.png");
	game->assets.texTutorial2 = juLoaderGetTexture(game->loader, "assets/tutorial2.png");
	game->assets.sprEnemy = juLoaderGetSprite(game->loader, "assets/enemy.png");
	game->assets.sprButtonPurchase = juLoaderGetSprite(game->loader, "assets/purchase.png");
	game->assets.sndMusicGoofy = juLoaderGetSound(game->loader, "assets/goofytrack.wav");
	game->assets.sndMusicSomber = juLoaderGetSound(game->loader, "assets/sombertrack.wav");
	game->assets.sndNewGun = juLoaderGetSound(game->loader, "assets/newgun.wav");
	game->assets.sndShop[0] = juLoaderGetSound(game->loader, "assets/whatisawenrad.wav");
	game->assets.sndShop[1] = juLoaderGetSound(game->loader, "assets/justdontgethit.wav");
	game->assets.sndShop[2] = juLoaderGetSound(game->loader, "assets/dontturnmypizzainsideout.wav");
	game->assets.sndSword = juLoaderGetSound(game->loader, "assets/sword.wav");
	game->assets.sndPistol = juLoaderGetSound(game->loader, "assets/pistol.wav");
	game->assets.sndShotgun = juLoaderGetSound(game->loader, "assets/shotgun.wav");
	game->assets.sndAssaultRifle = juLoaderGetSound(game->loader, "assets/assaultrifle.wav");
	game->assets.sndSniper = juLoaderGetSound(game->loader, "assets/sniper.wav");
	game->assets.sndHit = juLoaderGetSound(game->loader, "assets/hit.wav");
	game->assets.sndMusicMess = juLoaderGetSound(game->loader, "assets/mess.wav");
	game->assets.sndHeavyDrum = juLoaderGetSound(game->loader, "assets/heavy.wav");
	game->assets.sprEnemy->rotation = 0;

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

	// Load default player state and give a weapon
	memcpy(&game->player, &PLAYER_DEFAULT_STATE, sizeof(struct PNLPlayer));
	game->player.sprite = juLoaderGetSprite(game->loader, "assets/player.png");
	game->player.weapon = pnlGenerateWeapon(game, wt_Pistol);

	// Debug - generate a bunch of random weapons
	/*FILE *file = fopen("weapons/.csv", "w");
	const char *wepnames[] = {
			"Sword",
			"Shotgun",
			"AssaultRifle",
			"Sniper",
   			"Pistol",
	};
	for (int i = 0; i < 100; i++) {
		PNLWeapon w = pnlGenerateWeapon(game, wt_Any);
		fprintf(file, "%s %s,%s,%f damage,%f bps,%f spread,$%f,RGB:%f|%f|%f\n", WEAPON_NAME_FIRST[w.weaponNameFirstIndex], WEAPON_NAME_SECOND[w.weaponNameSecondIndex], wepnames[w.weaponType], w.weaponDamage, w.weaponBPS, w.weaponPellets, w.weaponCost, w.weaponColourMod[0], w.weaponColourMod[1], w.weaponColourMod[2]);
	}
	fclose(file);*/

	pnlInitHome(game);
}

// Called before the rendering begins
void pnlPreFrame(PNLRuntime game) {
	VK2DCamera cam = vk2dRendererGetCamera();

	// Start at the player
	float destX = game->player.pos.x - (GAME_WIDTH / 2);
	float destY = game->player.pos.y - (GAME_HEIGHT / 2);

	// Aim towards mouse
	float dist = juPointDistance(game->player.pos.x, game->player.pos.y, game->mouseX, game->mouseY);
	float angle = juPointAngle(game->player.pos.x, game->player.pos.y, game->mouseX, game->mouseY) - (VK2D_PI / 2);
	destX += cos(angle) * dist * (game->mouseRHeld ? CAMERA_ZOOM_AIM_DISTANCE : CAMERA_ZOOM_DISTANCE);
	destY -= sin(angle) * dist * (game->mouseRHeld ? CAMERA_ZOOM_AIM_DISTANCE : CAMERA_ZOOM_DISTANCE);

	// Set camera
	cam.x += (destX - cam.x) * PHYS_CAMERA_FRICTION * juDelta();
	cam.y += (destY - cam.y) * PHYS_CAMERA_FRICTION * juDelta();
	cam.w = GAME_WIDTH;
	cam.h = GAME_HEIGHT;
	cam.x = round(cam.x);
	cam.y = round(cam.y);
	if (!game->fadeIn && !game->fadeOut) {
		cam.zoom = 1;
		cam.rot = 0;
	} else {
		float percent;
		if (!game->fadeIn)
			percent = 1.0f - (game->fadeClock / FADE_IN_DURATION);
		else
			percent = (float)(game->fadeClock / FADE_IN_DURATION);
		cam.zoom = MAX_ZOOM * percent;
		cam.rot = MAX_ROT * percent;
	}

	vk2dRendererSetCamera(cam);
}

// Called during rendering
void pnlUpdate(PNLRuntime game) {
	if (game->onSite) {
		if (pnlUpdatePlanet(game) == ws_Home) {
			game->onSite = false;
			pnlQuitPlanet(game);
			pnlInitHome(game);
		}
	} else {
		if (pnlUpdateHome(game) == ws_Offsite) {
			game->onSite = true;
			pnlQuitHome(game);
			pnlInitPlanet(game);
		}
	}
	if (!game->mouseRHeld)
		vk2dDrawTexture(game->assets.texCursor, game->mouseX - 4, game->mouseY - 4);
	else
		vk2dDrawTextureExt(game->assets.texCursor, game->mouseX - 8, game->mouseY - 8, 2, 2, 0, 0, 0);
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
	SDL_Window *window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GAME_WIDTH * WINDOW_SCALE, GAME_HEIGHT * WINDOW_SCALE, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
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

	SDL_Surface *icon = SDL_LoadBMP("assets/icon.bmp");
	SDL_SetWindowIcon(window, icon);
	SDL_FreeSurface(icon);

	// Backbuffer
	VK2DTexture backbuffer = vk2dTextureCreate(vk2dRendererGetDevice(), GAME_WIDTH, GAME_HEIGHT);
	int w, h, lw, lh;
	int lastState, state;
	SDL_GetWindowSize(window, &w, &h);
	lw = w;
	lh = h;
	vk2dRendererSetTextureCamera(true);

	// Show loading screen before loading assets
	VK2DTexture texLoading = vk2dTextureLoad("assets/loading.png");
	vk2dRendererStartFrame(VK2D_BLACK);
	vk2dDrawTextureExt(texLoading, 0, 0, WINDOW_SCALE, WINDOW_SCALE, 0, 0, 0);
	vk2dRendererEndFrame();
	vk2dRendererWait();
	vk2dTextureFree(texLoading);

	// Game
	PNLRuntime game = calloc(1, sizeof(struct PNLRuntime));
	game->save = juSaveLoad(SAVE_FILE);
	game->loader = juLoaderCreate(ASSETS, ASSET_COUNT);
	game->ww = w;
	game->wh = h;
	pnlInit(game);

	real time = (real)SDL_GetPerformanceCounter();

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
			game->time += juDelta();
			pnlUpdate(game);
			vk2dRendererSetTarget(VK2D_TARGET_SCREEN);
			vk2dRendererSetViewport(0, 0, w, h);
			cam = vk2dRendererGetCamera();

			// Seriously wacky calculations since the camera is all over the place for figuring out how to draw the game upscaled
			float xscale = (float)w / GAME_WIDTH;
			float yscale = (float)h / GAME_HEIGHT;
			float factor = yscale > xscale ? xscale : yscale;
			float spaceX = (w - (GAME_WIDTH * factor)) / (w / GAME_WIDTH);
			float spaceY = (h - (GAME_HEIGHT * factor)) / (h / GAME_HEIGHT);
			float finalXScale = (GAME_WIDTH - spaceX) / GAME_WIDTH;
			float finalYScale = (GAME_HEIGHT - spaceY) / GAME_HEIGHT;

			vk2dDrawTextureExt(backbuffer, cam.x + (spaceX / 2), cam.y + (spaceY / 2), finalXScale, finalYScale, 0, 0, 0);
			vk2dRendererEndFrame();

			volatile int i;
			while (((real)SDL_GetPerformanceCounter() - time) / (real)SDL_GetPerformanceFrequency() < 1.0f / TARGET_FRAMERATE) {
				i = 0;// lol no
			}
			time = (real)SDL_GetPerformanceCounter();
		}
	}


	// Free assets
	vk2dRendererWait();
	pnlQuit(game);
	juSoundStopAll();
	juLoaderFree(game->loader);
	juSaveStore(game->save, SAVE_FILE);
	// juSaveFree(game->save); // uh oh memory leak?
	free(game);
	vk2dTextureFree(backbuffer);

	// Free
	juQuit();
	vk2dRendererQuit();
	SDL_DestroyWindow(window);
	return 0;
}
