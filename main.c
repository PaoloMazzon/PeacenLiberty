#define SDL_MAIN_HANDLED
#include <JamUtil.h>
#include <SDL2/SDL.h>

/********************** Typedefs **********************/
typedef long double real;

/********************** Constants **********************/
const int GAME_WIDTH = 600;
const int GAME_HEIGHT = 400;
const char *GAME_TITLE = "Peace & Liberty";

/********************** Struct **********************/
typedef struct PNLRuntime {

} *PNLRuntime;

/********************** Game functions **********************/
void pnlInit(PNLRuntime game) {

}

void pnlUpdate(PNLRuntime game) {

}

void pnlQuit(PNLRuntime game) {

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
	free(game);
	vk2dTextureFree(backbuffer);

	// Free
	juQuit();
	vk2dRendererQuit();
	SDL_DestroyWindow(window);
	return 0;
}
