#ifndef _SETTINGS_H
#define _SETTINGS_H

typedef enum fight_mode_t {
    FIGHT_MODE_NORMAL, 
    FIGHT_MODE_HYPER
} fight_mode;

typedef enum difficulty_t {
    PUNCHING_BAG, 
    ROOKIE, 
    VETERAN, 
    WORLD_CLASS,
    DEADLY,
    CHAMPION
} difficulty;

typedef struct settings_sound_t {
    int sound_on;
    int music_on;
    int stereo_on;
    int stereo_reversed;
} settings_sound;

typedef struct settings_video_t {
    int screen_w;
    int screen_h;
} settings_video;

typedef struct settings_gameplay_t {
    int speed;
    fight_mode fight_mode;
    int power1;
    int power2;
    int hazards_on;
    difficulty difficulty;
    int rounds;
} settings_gameplay;

typedef struct settings_t {
    settings_video video;
    settings_sound sound;
    settings_gameplay gameplay;
} settings;

int settings_init(settings *s);
void settings_free(settings *s);

void settings_load(settings *s);
void settings_save(settings *s);

#endif // _SETTINGS_H

