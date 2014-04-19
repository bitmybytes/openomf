#include <stdlib.h>
#include <SDL2/SDL.h>
#include "utils/log.h"
#include "utils/random.h"
#include "video/video.h"
#include "resources/ids.h"
#include "game/text/text.h"
#include "game/text/languages.h"
#include "game/protos/scene.h"
#include "game/scenes/vs.h"
#include "game/menu/menu_background.h"
#include "game/game_state.h"
#include "controller/controller.h"
#include "controller/keyboard.h"

void cb_vs_spawn_object(object *parent, int id, vec2i pos, int g, void *userdata);
void cb_vs_destroy_object(object *parent, int id, void *userdata);

typedef struct vs_local_t {
    object player1_portrait;
    object player2_portrait;
    object player1_har;
    object player2_har;
    surface arena_select_bg;
    object arena_select;
    int arena;
} vs_local;

void cb_vs_spawn_object(object *parent, int id, vec2i pos, int g, void *userdata) {
    scene *s = (scene*)userdata;

    // Get next animation
    bk_info *info = bk_get_info(&s->bk_data, id);
    if(info != NULL) {
        object *obj = malloc(sizeof(object));
        object_create(obj, parent->gs, vec2i_add(pos, vec2f_to_i(parent->pos)), vec2f_create(0,0));
        object_set_stl(obj, object_get_stl(parent));
        object_set_animation(obj, &info->ani);
        object_set_spawn_cb(obj, cb_vs_spawn_object, userdata);
        object_set_destroy_cb(obj, cb_vs_destroy_object, userdata);
        game_state_add_object(parent->gs, obj, RENDER_LAYER_MIDDLE);
    }
}

void cb_vs_destroy_object(object *parent, int id, void *userdata) {
    game_state_del_animation(parent->gs, id);
}

void vs_free(scene *scene) {
    vs_local *local = scene_get_userdata(scene);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    surface_free(&local->arena_select_bg);
    object_free(&local->player1_portrait);
    object_free(&local->player2_portrait);
    object_free(&local->player1_har);
    object_free(&local->player2_har);
    if (player2->selectable) {
        object_free(&local->arena_select);
    }
    free(local);
}

void vs_handle_action(scene *scene, int action) {
    vs_local *local = scene_get_userdata(scene);
    switch (action) {
        case ACT_KICK:
        case ACT_PUNCH:
            game_state_set_next(scene->gs, SCENE_ARENA0+local->arena);
            break;
        case ACT_UP:
        case ACT_LEFT:
            if(game_state_get_player(scene->gs, 1)->selectable) {
                local->arena--;
                if (local->arena < 0) {
                    local->arena =4;
                }
                object_select_sprite(&local->arena_select, local->arena);
            }
            break;
        case ACT_DOWN:
        case ACT_RIGHT:
            if(game_state_get_player(scene->gs, 1)->selectable) {
                local->arena++;
                if (local->arena > 4) {
                    local->arena = 0;
                }
                object_select_sprite(&local->arena_select, local->arena);
            }
            break;
    }
}

void vs_tick(scene *scene) {
    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *i = NULL;
    // Handle extra controller inputs
    i = player1->ctrl->extra_events;
    if (i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                vs_handle_action(scene, i->event_data.action);
            } else if (i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
                return;
            }
        } while((i = i->next));
    }
}

int vs_event(scene *scene, SDL_Event *event) {
    ctrl_event *p1=NULL, *i;
    game_player *player1 = game_state_get_player(scene->gs, 0);
    controller_event(player1->ctrl, event, &p1);
    i = p1;
    if (i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if (i->event_data.action == ACT_ESC) {
                    game_state_set_next(scene->gs, SCENE_MELEE);
                    return 1;
                } else {
                    vs_handle_action(scene, i->event_data.action);
                }
            } else if (i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
                return 1;
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
    return 1;
}

void vs_render(scene *scene) {
    vs_local *local = scene_get_userdata(scene);

    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    // player 1 HAR
    object_render(&local->player1_har);

    // player 2 HAR
    object_render(&local->player2_har);

    // player 1 portrait
    object_render(&local->player1_portrait);

    // player 2 portrait
    object_render(&local->player2_portrait);


    if (player2->selectable) {
        // arena selection
        video_render_sprite(&local->arena_select_bg, 55, 150, BLEND_ALPHA, 0);

        object_render(&local->arena_select);

        // arena name
        font_render_wrapped(&font_small, lang_get(56+local->arena), 56+72, 153, (211-72)-4, COLOR_GREEN);

        // arena description
        font_render_wrapped(&font_small, lang_get(66+local->arena), 56+72, 160, (211-72)-4, COLOR_GREEN);

    } else {
        font_render_wrapped(&font_small, lang_get(749+(11*player1->pilot_id)+player2->pilot_id), 59, 160, 150, COLOR_YELLOW);
        font_render_wrapped(&font_small, lang_get(870+(11*player2->pilot_id)+player1->pilot_id), 320-(59+150), 180, 150, COLOR_YELLOW);
    }
}

int vs_create(scene *scene) {
    // Init local data
    vs_local *local = malloc(sizeof(vs_local));
    scene_set_userdata(scene, local);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    animation *ani;

    palette *mpal = video_get_base_palette();
    palette_set_player_color(mpal, 0, player1->colors[2], 0);
    palette_set_player_color(mpal, 0, player1->colors[1], 1);
    palette_set_player_color(mpal, 0, player1->colors[0], 2);
    palette_set_player_color(mpal, 1, player2->colors[2], 0);
    palette_set_player_color(mpal, 1, player2->colors[1], 1);
    palette_set_player_color(mpal, 1, player2->colors[0], 2);
    video_force_pal_refresh();

    // HAR
    ani = &bk_get_info(&scene->bk_data, 5)->ani;
    object_create(&local->player1_har, scene->gs, vec2i_create(160,0), vec2f_create(0, 0));
    object_set_animation(&local->player1_har, ani);
    object_select_sprite(&local->player1_har, player1->har_id - HAR_JAGUAR);

    object_create(&local->player2_har, scene->gs, vec2i_create(160,0), vec2f_create(0, 0));
    object_set_animation(&local->player2_har, ani);
    object_select_sprite(&local->player2_har, player2->har_id - HAR_JAGUAR);
    object_set_direction(&local->player2_har, OBJECT_FACE_LEFT);
    object_set_pal_offset(&local->player2_har, 48);

    // PLAYER
    ani = &bk_get_info(&scene->bk_data, 4)->ani;
    object_create(&local->player1_portrait, scene->gs, vec2i_create(-10,150), vec2f_create(0, 0));
    object_set_animation(&local->player1_portrait, ani);
    object_select_sprite(&local->player1_portrait, player1->pilot_id);

    object_create(&local->player2_portrait, scene->gs, vec2i_create(330,150), vec2f_create(0, 0));
    object_set_animation(&local->player2_portrait, ani);
    object_select_sprite(&local->player2_portrait, player2->pilot_id);
    object_set_direction(&local->player2_portrait, OBJECT_FACE_LEFT);

    // clone the left side of the background image
    // Note! We are touching the scene-wide background surface!
    surface_sub(&scene->bk_data.background, // DST Surface
                &scene->bk_data.background, // SRC Surface
                160, 0, // DST
                0, 0, // SRC
                160, 200, // Size
                SUB_METHOD_MIRROR); // Flip the right side horizontally

    if (player2->selectable) {
        // player1 gets to choose, start at arena 0
        local->arena = 0;
    } else {
        // pick a random arena for 1 player mode
        local->arena = rand_int(5); // srand was done in melee
    }

    // Arena
    if(player2->selectable) {
        ani = &bk_get_info(&scene->bk_data, 3)->ani;
        object_create(&local->arena_select, scene->gs, vec2i_create(59,155), vec2f_create(0, 0));
        object_set_animation(&local->arena_select, ani);
        object_select_sprite(&local->arena_select, local->arena);
    }

    
    // SCIENTIST
    object *o_scientist = malloc(sizeof(object));
    ani = &bk_get_info(&scene->bk_data, 8)->ani;
    object_create(o_scientist, scene->gs, vec2i_create(280,118), vec2f_create(0, 0));
    object_set_animation(o_scientist, ani);
    object_select_sprite(o_scientist, 0);
    object_set_direction(o_scientist, OBJECT_FACE_LEFT);
    game_state_add_object(scene->gs, o_scientist, RENDER_LAYER_MIDDLE);

    // WELDER
    object *o_welder = malloc(sizeof(object));
    ani = &bk_get_info(&scene->bk_data, 7)->ani;
    object_create(o_welder, scene->gs, vec2i_create(90,80), vec2f_create(0, 0));
    object_set_animation(o_welder, ani);
    object_select_sprite(o_welder, 0);
    object_set_spawn_cb(o_welder, cb_vs_spawn_object, (void*)scene);
    object_set_destroy_cb(o_welder, cb_vs_destroy_object, (void*)scene);
    game_state_add_object(scene->gs, o_welder, RENDER_LAYER_MIDDLE);

    // GANTRIES
    object *o_gantry_a = malloc(sizeof(object));
    ani = &bk_get_info(&scene->bk_data, 11)->ani;
    object_create(o_gantry_a, scene->gs, vec2i_create(0,0), vec2f_create(0, 0));
    object_set_animation(o_gantry_a, ani);
    object_select_sprite(o_gantry_a, 0);
    game_state_add_object(scene->gs, o_gantry_a, RENDER_LAYER_TOP);

    object *o_gantry_b = malloc(sizeof(object));
    object_create(o_gantry_b, scene->gs, vec2i_create(320,0), vec2f_create(0, 0));
    object_set_animation(o_gantry_b, ani);
    object_select_sprite(o_gantry_b, 0);
    object_set_direction(o_gantry_b, OBJECT_FACE_LEFT);
    game_state_add_object(scene->gs, o_gantry_b, RENDER_LAYER_TOP);

    // Background tex
    menu_background2_create(&local->arena_select_bg, 211, 50);

    // Callbacks
    scene_set_render_cb(scene, vs_render);
    scene_set_event_cb(scene, vs_event);
    scene_set_tick_cb(scene, vs_tick);
    scene_set_free_cb(scene, vs_free);

    // Pick renderer
    video_select_renderer(VIDEO_RENDERER_HW);

    return 0;
}
