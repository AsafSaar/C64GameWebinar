/*
 * gfx.h - C64 drawing for Tank's Disk Rescue.
 *
 * Draws the state held in `room` (room.h). It never changes game state.
 */
#ifndef GFX_H
#define GFX_H

/* Set up the VIC-II, build the character set and sprites. Call once. */
void gfx_init(void);

/* Draw the whole room, both sprites and the HUD from scratch. */
void gfx_draw_room(void);

/* Redraw one tile from the room grid. */
void gfx_draw_tile(unsigned char col, unsigned char row);

/* Put Tank's sprite on his tile, even if he has just been crushed. */
void gfx_place_tank(void);

/* Put Tank's sprite on his tile, or hide it once he is crushed. */
void gfx_update_tank(void);

/* Hide the disk sprite once it is collected. */
void gfx_update_disk(void);

/* Animate the crate falls recorded by the last room_move(). */
void gfx_animate_falls(void);

/* Redraw the HUD on screen row 24. */
void gfx_draw_hud(void);

/* Wait for the next frame (vertical blank). */
void gfx_wait_frame(void);

#endif
