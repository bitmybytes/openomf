#include <stdlib.h>
#include "game/protos/scene.h"
#include "video/video.h"
#include "resources/ids.h"
#include "resources/bk_loader.h"
#include "utils/log.h"
#include "utils/vec.h"
#include "game/game_state.h"

// Some internal functions
void cb_spawn_object(object *parent, int id, vec2i pos, int g, void *userdata);
void cb_destroy_object(object *parent, int id, void *userdata);


// Loads BK file etc.
int scene_create(scene *scene, int scene_id) {
    // Load BK
    if(scene_id == SCENE_NONE || load_bk_file(&scene->bk_data, scene_id)) {
        PERROR("Unable to load BK file %s (%d)!", get_id_name(scene_id), scene_id);
        return 1;
    }
    scene->id = scene_id;

    // Init functions
    scene->userdata = NULL;
    scene->free = NULL;
    scene->event = NULL;
    scene->render = NULL;
    scene->tick = NULL;
    scene->startup = NULL;

    // All done.
    DEBUG("Loaded BK file %s (%d).", get_id_name(scene_id), scene_id);
    return 0;
}

void scene_init(scene *scene) {
    // Init background sprite with palette
    object_create(&scene->background, vec2i_create(0,0), vec2f_create(0,0));
    animation *bgani = create_animation_from_single(sprite_copy(&scene->bk_data.background), vec2i_create(0,0));
    object_set_animation(&scene->background, bgani);
    object_set_animation_owner(&scene->background, OWNER_OBJECT);
    object_select_sprite(&scene->background, 0);
    object_set_palette(&scene->background, bk_get_palette(&scene->bk_data, 0), 0);

    // Bootstrap animations
    iterator it;
    hashmap_iter_begin(&scene->bk_data.infos, &it);
    hashmap_pair *pair = NULL;
    while((pair = iter_next(&it)) != NULL) {
        bk_info *info = (bk_info*)pair->val;
        if(info->load_on_start || info->probability == 1 || scene_startup(scene, info->ani.id)) {
            object obj;
            object_create(&obj, info->ani.start_pos, vec2f_create(0,0));
            object_set_stl(&obj, scene->bk_data.sound_translation_table);
            object_set_palette(&obj, bk_get_palette(&scene->bk_data, 0), 0);
            object_set_animation(&obj, &info->ani);
            object_set_spawn_cb(&obj, cb_spawn_object, (void*)scene);
            object_set_destroy_cb(&obj, cb_destroy_object, (void*)scene);
            game_state_add_object(&obj);
            DEBUG("Scene bootstrap: Animation %d started.", info->ani.id);
        }
    }
}

void scene_set_userdata(scene *scene, void *userdata) {
    scene->userdata = userdata;
}

void* scene_get_userdata(scene *scene) {
    return scene->userdata;
}

int scene_startup(scene *scene, int id) {
    if(scene->startup != NULL) {
        return scene->startup(scene, id);
    }
    return 0;
}

// Return 0 if event was handled here
int scene_event(scene *scene, SDL_Event *event) {
    if(scene->event != NULL) {
        return scene->event(scene, event);
    }
    return 1;
}

void scene_render(scene *scene) {
    object_render_neutral(&scene->background);
    if(scene->render != NULL) {
        scene->render(scene);
    }
}

void scene_tick(scene *scene) {
    if(scene->tick != NULL) {
        scene->tick(scene);
    }
}

void scene_free(scene *scene) {
    if(scene->free != NULL) {
        scene->free(scene);
    }
    bk_free(&scene->bk_data);
    object_free(&scene->background);
}

void scene_set_free_cb(scene *scene, scene_free_cb cbfunc) {
    scene->free = cbfunc;
}

void scene_set_event_cb(scene *scene, scene_event_cb cbfunc) {
    scene->event = cbfunc;
}

void scene_set_render_cb(scene *scene, scene_render_cb cbfunc) {
    scene->render = cbfunc;
}

void scene_set_startup_cb(scene *scene, scene_startup_cb cbfunc) {
    scene->startup = cbfunc;
}

void scene_set_tick_cb(scene *scene, scene_tick_cb cbfunc) {
    scene->tick = cbfunc;
}

int scene_is_valid(int id) {
    switch(id) {
        case SCENE_INTRO:
        case SCENE_MENU:
        case SCENE_ARENA0:
        case SCENE_ARENA1:
        case SCENE_ARENA2:
        case SCENE_ARENA3:
        case SCENE_ARENA4:
        case SCENE_NEWSROOM:
        case SCENE_END:
        case SCENE_END1:
        case SCENE_END2: 
        case SCENE_CREDITS:
        case SCENE_MECHLAB:
        case SCENE_MELEE:
        case SCENE_VS:
        case SCENE_NORTHAM:
        case SCENE_KATUSHAI:
        case SCENE_WAR:
        case SCENE_WORLD:
            return 1;
    }
    return 0;
}

void cb_spawn_object(object *parent, int id, vec2i pos, int g, void *userdata) {
    scene *s = (scene*)userdata;

    // Get next animation
    bk_info *info = bk_get_info(&s->bk_data, id);
    if(info != NULL) {
        object obj;
        object_create(&obj, vec2i_add(pos, info->ani.start_pos), vec2f_create(0,0));
        object_set_stl(&obj, object_get_stl(parent));
        object_set_palette(&obj, object_get_palette(parent), 0);
        object_set_animation(&obj, &info->ani);
        object_set_spawn_cb(&obj, cb_spawn_object, userdata);
        object_set_destroy_cb(&obj, cb_destroy_object, userdata);
        game_state_add_object(&obj);
    }
}

void cb_destroy_object(object *parent, int id, void *userdata) {
    game_state_del_object(id);
}
