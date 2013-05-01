#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "utils/log.h"
#include "game/text/text.h"
#include "audio/music.h"
#include "game/scene.h"
#include "game/scenes/mainmenu.h"

font font_small;

int menu_init(scene *scene) {
    if(!music_playing()) {
        music_play("resources/MENU.PSM");
    }
    font_create(&font_small);
    if(font_load(&font_small, "resources/CHARSMAL.DAT", FONT_SMALL)) {
        PERROR("Error while loading small font!");
        font_free(&font_small);
        return 1;
    }
    return 0;
}

void menu_deinit(scene *scene) {
    font_free(&font_small);
}

void menu_tick(scene *scene) {

}

int menu_event(scene *scene, SDL_Event *event) {
    return 1;
}

void menu_render(scene *scene) {
    int line = 50;
    font_render(&font_small, "Just", 200, line, 50, 205, 50);
    line += 7;
    font_render(&font_small, "Testing", 200, line, 50, 205, 50);
    line += 7;
    font_render(&font_small, "the font", 200, line, 50, 205, 50);
    line += 7;
    font_render(&font_small, "renderer", 200, line, 50, 205, 50);
}

void menu_load(scene *scene) {
    scene->event = menu_event;
    scene->render = menu_render;
    scene->init = menu_init;
    scene->deinit = menu_deinit;
    scene->tick = menu_tick;
}
