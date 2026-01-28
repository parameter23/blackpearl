#include <stdio.h>
#include "invaders.h"
#include "framebuffer4.h"
#include "sprite.h"
#include "joystick.h"
#include "sfx.h"
#include "text.h"
#include "invader_anim.h"
#include "explosion_sprites.h"
#include "title_screen.h"
#include "music.h"

#define PLAYER_Y   (FB4_HEIGHT - 20)
#define PLAYER_W    10
#define PLAYER_H     6

#define MAX_BULLETS        6
#define MAX_ENEMY_BULLETS  3
#define MAX_EXPLOSIONS     8

#define INV_W       8
#define INV_H       8
#define INV_ROWS    4
#define INV_COLS    8

#define STAR_COUNT_NEAR  20
#define STAR_COUNT_FAR   30

typedef struct {
    int x, y;
} Star;

static Star stars_near[STAR_COUNT_NEAR];
static Star stars_far[STAR_COUNT_FAR];

typedef struct {
    int x, y;
    uint8_t active;
} Bullet;

typedef struct {
    int x, y;
    uint8_t active;
} EnemyBullet;

typedef struct {
    int x, y;
    uint8_t alive;
    uint8_t type;
    uint8_t hit_flash;   // farbiges Trefferblinken
} Invader;

typedef struct {
    int x, y;
    uint8_t frame;
    uint8_t active;
    uint16_t timer;
} Explosion;

typedef enum {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_LEVEL_COMPLETE,
    STATE_GAME_OVER
} GameState;

/* ---------------------------------------------------------
 * Globale Variablen
 * --------------------------------------------------------- */
static int player_x = FB4_WIDTH/2;
static int shoot_cooldown = 0;

static Bullet bullets[MAX_BULLETS];
static EnemyBullet enemy_bullets[MAX_ENEMY_BULLETS];
static Explosion explosions[MAX_EXPLOSIONS];
static Invader inv[INV_ROWS][INV_COLS];

static int inv_dx = 2;
static int inv_timer = 0;
static int score = 0;
static int lives = 3;
int level = 1;

static GameState game_state = STATE_TITLE;

/* ---------------------------------------------------------
 * Pseudozufall
 * --------------------------------------------------------- */
static uint32_t rnd(void)
{
    static uint32_t r = 0xCAFEBABE;
    r = r * 1664525 + 1013904223;
    return r;
}

/* ---------------------------------------------------------
 * Explosion erzeugen
 * --------------------------------------------------------- */
static void spawn_explosion(int x, int y)
{
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) {
            explosions[i].active = 1;
            explosions[i].x = x;
            explosions[i].y = y;
            explosions[i].frame = 0;
            explosions[i].timer = 0;
            return;
        }
    }
}

/* ---------------------------------------------------------
 * Explosionen updaten
 * --------------------------------------------------------- */
static void update_explosions(void)
{
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) continue;

        explosions[i].timer++;

        if (explosions[i].timer > 60) {
            explosions[i].timer = 0;
            explosions[i].frame++;

            if (explosions[i].frame >= 4) {
                explosions[i].active = 0;
            }
        }
    }
}

/* ---------------------------------------------------------
 * Explosionen zeichnen
 * --------------------------------------------------------- */
static void draw_explosions(void)
{
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) continue;

        const Sprite4 *spr = NULL;

        switch (explosions[i].frame) {
        case 0: spr = &explosion_frame1; break;
        case 1: spr = &explosion_frame2; break;
        case 2: spr = &explosion_frame3; break;
        case 3: spr = &explosion_frame4; break;
        }

        sprite4_draw_trans(explosions[i].x, explosions[i].y, spr, 0);
    }
}

/* ---------------------------------------------------------
 * Level aufbauen
 * --------------------------------------------------------- */
static void build_level(void)
{
    for (int r = 0; r < INV_ROWS; r++) {
        for (int c = 0; c < INV_COLS; c++) {

            inv[r][c].alive = 1;
            inv[r][c].type = r % 3;
            inv[r][c].hit_flash = 0;

            inv[r][c].x = 30 + c * (INV_W + 6 - level);
            inv[r][c].y = 20 + r * (INV_H + 4);
        }
    }

    inv_dx = 2 + level;
    inv_timer = 0;
}

/* ---------------------------------------------------------
 * Spiel komplett neu starten
 * --------------------------------------------------------- */
void invaders_init(void)
{
    title_screen_init();
    invader_anim_init();
    music_init();

    game_state = STATE_TITLE;

    player_x = FB4_WIDTH/2;
    shoot_cooldown = 0;

    for (int i = 0; i < MAX_BULLETS; i++)
        bullets[i].active = 0;

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
        enemy_bullets[i].active = 0;

    for (int i = 0; i < MAX_EXPLOSIONS; i++)
        explosions[i].active = 0;

    score = 0;
    lives = 3;
    level = 1;

    build_level();

    /* Parallax-Sterne initialisieren */
    for (int i = 0; i < STAR_COUNT_NEAR; i++) {
        stars_near[i].x = rnd() % FB4_WIDTH;
        stars_near[i].y = rnd() % FB4_HEIGHT;
    }
    for (int i = 0; i < STAR_COUNT_FAR; i++) {
        stars_far[i].x = rnd() % FB4_WIDTH;
        stars_far[i].y = rnd() % FB4_HEIGHT;
    }

}

/* ---------------------------------------------------------
 * Spieler-Schuss
 * --------------------------------------------------------- */
static void spawn_bullet(int x, int y)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = 1;
            bullets[i].x = x;
            bullets[i].y = y;
            sfx_play(SFX_BEEP);
            return;
        }
    }
}

/* ---------------------------------------------------------
 * Gegner-Schuss
 * --------------------------------------------------------- */
static void enemy_spawn_bullet(int x, int y)
{
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) {
            enemy_bullets[i].active = 1;
            enemy_bullets[i].x = x;
            enemy_bullets[i].y = y;
            sfx_play(SFX_LASER);
            return;
        }
    }
}

static void enemy_try_shoot(void)
{
    int col = (rnd() >> 16) % INV_COLS;

    for (int r = INV_ROWS - 1; r >= 0; r--) {
        if (inv[r][col].alive) {
            enemy_spawn_bullet(inv[r][col].x + INV_W/2,
                               inv[r][col].y + INV_H + 2);
            return;
        }
    }
}

/* ---------------------------------------------------------
 * Spieler verliert ein Leben
 * --------------------------------------------------------- */
static void lose_life(void)
{
    lives--;
    sfx_play(SFX_NOISE_SHORT);

    if (lives <= 0) {
        game_state = STATE_GAME_OVER;
        music_stop();
        return;
    }

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
        enemy_bullets[i].active = 0;

    for (int i = 0; i < MAX_BULLETS; i++)
        bullets[i].active = 0;

    player_x = FB4_WIDTH/2;
}

/* ---------------------------------------------------------
 * Prüfen, ob Level beendet ist
 * --------------------------------------------------------- */
static uint8_t all_invaders_dead(void)
{
    for (int r = 0; r < INV_ROWS; r++)
        for (int c = 0; c < INV_COLS; c++)
            if (inv[r][c].alive)
                return 0;
    return 1;
}

/* ---------------------------------------------------------
 * Update
 * --------------------------------------------------------- */
void invaders_update(void)
{
    /* Musik tick */
    music_update_1ms();

    /* Animation tick */
    invader_anim_update();

    /* Parallax-Hintergrund bewegen */
    for (int i = 0; i < STAR_COUNT_NEAR; i++) {
        stars_near[i].y += 2;  // schneller
        if (stars_near[i].y >= FB4_HEIGHT) {
            stars_near[i].y = 0;
            stars_near[i].x = rnd() % FB4_WIDTH;
        }
    }

    for (int i = 0; i < STAR_COUNT_FAR; i++) {
        stars_far[i].y += 1;  // langsamer
        if (stars_far[i].y >= FB4_HEIGHT) {
            stars_far[i].y = 0;
            stars_far[i].x = rnd() % FB4_WIDTH;
         }
    }


    /* -------------------- TITLE SCREEN -------------------- */
    if (game_state == STATE_TITLE) {
        title_screen_update();
        if (!title_screen_active) {
            game_state = STATE_PLAYING;
            music_play();
        }
        return;
    }

    JoystickState js = joystick_update();

    /* -------------------- GAME OVER -------------------- */
    if (game_state == STATE_GAME_OVER) {
        if (js.pressed & JS_BTN) {
            invaders_init();
        }
        return;
    }

    /* -------------------- LEVEL COMPLETE -------------------- */
    if (game_state == STATE_LEVEL_COMPLETE) {
        if (js.pressed & JS_BTN) {
            level++;
            build_level();
            game_state = STATE_PLAYING;
        }
        return;
    }

    /* -------------------- PLAYING -------------------- */

    /* Spieler bewegen */
    /* Analoge Steuerung */
    uint16_t ax = joystick_get_adc_x();   // 0–4095

    int center = 2048;
    int deadzone = 200;

    int dx = ax - center;

    /* Bewegung nach rechts */
    if (dx > deadzone) {
        int speed = (dx - deadzone) / 200;   // feinfühlig
        if (speed > 6) speed = 6;
        player_x += speed;
    }
    /* Bewegung nach links */
    else if (dx < -deadzone) {
        int speed = (-dx - deadzone) / 200;
        if (speed > 6) speed = 6;
        player_x -= speed;
    }

    if (player_x < 0) player_x = 0;
    if (player_x > FB4_WIDTH - PLAYER_W) player_x = FB4_WIDTH - PLAYER_W;

    /* Schießen */
    if (shoot_cooldown > 0) shoot_cooldown--;

    if ((js.pressed & JS_BTN) && shoot_cooldown == 0) {
        spawn_bullet(player_x + PLAYER_W/2, PLAYER_Y - 2);
        shoot_cooldown = 12;
    }

    /* Spieler-Bullets bewegen */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        bullets[i].y -= 5;
        if (bullets[i].y < 0)
            bullets[i].active = 0;
    }

    /* Gegner bewegen */
    inv_timer++;
    int anim_speed = 120 - (level * 10);
    if (anim_speed < 40) anim_speed = 40;

    if (inv_timer >= 20 - level) {
        inv_timer = 0;

        int hit_edge = 0;

        for (int r = 0; r < INV_ROWS; r++) {
            for (int c = 0; c < INV_COLS; c++) {
                if (!inv[r][c].alive) continue;

                inv[r][c].x += inv_dx;

                if (inv[r][c].x < 10 || inv[r][c].x > FB4_WIDTH - 20)
                    hit_edge = 1;
            }
        }

        if (hit_edge) {
            inv_dx = -inv_dx;
            for (int r = 0; r < INV_ROWS; r++)
                for (int c = 0; c < INV_COLS; c++)
                    inv[r][c].y += 6;
        }
    }

    /* Kollisionen Spieler → Gegner */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        for (int r = 0; r < INV_ROWS; r++) {
            for (int c = 0; c < INV_COLS; c++) {

                if (!inv[r][c].alive) continue;

                int bx = bullets[i].x;
                int by = bullets[i].y;

                int ix = inv[r][c].x;
                int iy = inv[r][c].y;

                if (bx > ix && bx < ix + INV_W &&
                    by > iy && by < iy + INV_H) {

                    inv[r][c].alive = 0;
                    inv[r][c].hit_flash = 6;   // farbiges Trefferblinken
                    bullets[i].active = 0;
                    score += 10 * level;

                    spawn_explosion(ix, iy);
                    sfx_play(SFX_EXPLOSION);
                }
            }
        }
    }

    /* Gegner schießen */
    if ((rnd() % (40 - level*2)) == 0)
        enemy_try_shoot();

    /* Gegner-Bullets bewegen */
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) continue;

        enemy_bullets[i].y += 4 + level;

        if (enemy_bullets[i].y > FB4_HEIGHT)
            enemy_bullets[i].active = 0;

        /* Kollision mit Spieler */
        if (enemy_bullets[i].y > PLAYER_Y &&
            enemy_bullets[i].x > player_x &&
            enemy_bullets[i].x < player_x + PLAYER_W) {

            enemy_bullets[i].active = 0;
            lose_life();
        }
    }

    update_explosions();

    /* Trefferblinken runterzählen */
    for (int r = 0; r < INV_ROWS; r++) {
        for (int c = 0; c < INV_COLS; c++) {
            if (inv[r][c].hit_flash > 0)
                inv[r][c].hit_flash--;
        }
    }

    /* Level abgeschlossen? */
    if (all_invaders_dead()) {
        game_state = STATE_LEVEL_COMPLETE;
    }
}

/* ---------------------------------------------------------
 * Rendering
 * --------------------------------------------------------- */
void invaders_render(void)
{
    /* TITLE SCREEN */
    if (game_state == STATE_TITLE) {
        title_screen_render();
        return;
    }

    fb4_clear(0);

    /* Parallax-Hintergrund */
    for (int i = 0; i < STAR_COUNT_FAR; i++) {
        fb4_set_pixel(stars_far[i].x, stars_far[i].y, 4);  // dunkler
    }

    for (int i = 0; i < STAR_COUNT_NEAR; i++) {
        fb4_set_pixel(stars_near[i].x, stars_near[i].y, 15); // hell
    }


    /* Spieler */
    fb4_fill_rect(player_x, PLAYER_Y, PLAYER_W, PLAYER_H, 2);

    /* Spieler-Bullets */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active)
            fb4_fill_rect(bullets[i].x, bullets[i].y, 2, 4, 1);
    }

    /* Gegner */
    for (int r = 0; r < INV_ROWS; r++) {
        for (int c = 0; c < INV_COLS; c++) {
            if (!inv[r][c].alive) continue;

            /* farbiges Trefferblinken */
            if (inv[r][c].hit_flash > 0) {
                fb4_fill_rect(inv[r][c].x, inv[r][c].y, INV_W, INV_H, 14);
                continue;
            }

            const Sprite4 *spr = invader_anim_get(inv[r][c].type, anim_frame);
            sprite4_draw_trans(inv[r][c].x, inv[r][c].y, spr, 0);
        }
    }

    /* Gegner-Bullets */
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemy_bullets[i].active)
            fb4_fill_rect(enemy_bullets[i].x, enemy_bullets[i].y, 2, 4, 5);
    }

    /* Explosionen */
    draw_explosions();

    /* Score */
    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE: %d", score);
    draw_text_fb_trans(4, 4, buf, 1);

    /* Lives */
    snprintf(buf, sizeof(buf), "LIVES: %d", lives);
    draw_text_fb_trans(FB4_WIDTH - 70, 4, buf, 1);

    /* Level */
    snprintf(buf, sizeof(buf), "LEVEL: %d", level);
    draw_text_fb_trans(FB4_WIDTH/2 - 20, 4, buf, 1);

    /* Level Complete Screen */
    if (game_state == STATE_LEVEL_COMPLETE) {
        draw_text_fb_trans(FB4_WIDTH/2 - 40, FB4_HEIGHT/2 - 10, "LEVEL COMPLETE", 1);
        draw_text_fb_trans(FB4_WIDTH/2 - 60, FB4_HEIGHT/2 + 10, "PRESS BUTTON TO CONTINUE", 1);
    }

    /* Game Over Screen */
    if (game_state == STATE_GAME_OVER) {
        draw_text_fb_trans(FB4_WIDTH/2 - 30, FB4_HEIGHT/2 - 10, "GAME OVER", 1);
        draw_text_fb_trans(FB4_WIDTH/2 - 60, FB4_HEIGHT/2 + 10, "PRESS BUTTON TO RESTART", 1);
    }

    fb4_flush_dma();
}
