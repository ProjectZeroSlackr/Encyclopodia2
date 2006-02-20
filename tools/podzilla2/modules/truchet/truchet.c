/*
 * Truchet - A widget that defrabulates
 *  
 * Copyright (C) 2006 Scott Lawrence
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <stdio.h>
#include <string.h>
#include "pz.h"

typedef struct {
        /* pz meta stuff */
        PzModule * module;
        PzWindow * window;
        PzWidget * widget;

        int timer;      /* internal timer */
	int clicks;	/* number of action clicks */

	char buf[64][64];
	int bw, bh;

        /* for drawing */
        int w, h;
        int fullscreen;

        ttk_color fg;
        ttk_color bg;
        ttk_color border;

} truchet_globals;

static truchet_globals tglob;


static void fill_buffer()
{
	int x,y;
	for( x=0 ; x<tglob.bw ; x++ ) for( y=0 ; y<tglob.bh; y++ ){
		tglob.buf[x][y] = rand();
	}
}

static void twiddle_buffer()
{
	int x,y;
	x = rand() % tglob.bw;
	y = rand() % tglob.bh;
	
	tglob.buf[x][y] = rand();
}

/* this basically just calls the current clock face routine */
void draw_truchet( PzWidget *widget, ttk_surface srf )
{
	int x,y;
	unsigned short slash1[] = {
		0x0080, 0x0040, 0x0020, 0x0010,
		0x0008, 0x0004, 0x0002, 0x0001,
		0x8000, 0x4000, 0x2000, 0x1000,
		0x0800, 0x0400, 0x0200, 0x0100,
	};

	unsigned short slash2[] = {
		0x0100, 0x0200, 0x0400, 0x0800,
		0x1000, 0x2000, 0x4000, 0x8000,
		0x0001, 0x0002, 0x0004, 0x0008,
		0x0010, 0x0020, 0x0040, 0x0080,
	};

	unsigned short curve1[] = {
		0x0180, 0x0180, 0x01c0, 0x01c0,
		0x00e0, 0x00f0, 0x007c, 0xf03f,
		0xfc0f, 0x3e00, 0x0f00, 0x0700,
		0x0380, 0x0380, 0x0180, 0x0180,
	};

	unsigned short curve2[] = {
		0x0180, 0x0180, 0x0380, 0x0380,
		0x0700, 0x0f00, 0x3e00, 0xfc0f,
		0xf03f, 0x003c, 0x00f0, 0x00e0,
		0x01c0, 0x01c0, 0x0180, 0x0180,
	    
	};

	unsigned short *tile1 = curve1;
	unsigned short *tile2 = curve2;

	ttk_gc gc = ttk_new_gc();
	TWindow *pixmap = malloc (sizeof(TWindow)); pixmap->srf = srf;

	/* deal with user input */
	if( tglob.clicks > 3 ) tglob.clicks = 0;

	if( tglob.clicks & 1 ) {
		tglob.bg = ttk_ap_get( "window.bg" )->color;
		tglob.fg = ttk_ap_get( "window.fg" )->color;
	} else {
		tglob.bg = ttk_ap_get( "window.fg" )->color;
		tglob.fg = ttk_ap_get( "window.bg" )->color;
	}

	if( tglob.clicks & 2 ) {
		tile1 = slash1;
		tile2 = slash2;
	}
	tglob.border = ttk_ap_get( "window.border" )->color;


	ttk_fillrect( srf, 0, 0, tglob.w, tglob.h, tglob.bg );
	ttk_gc_set_foreground( gc, tglob.fg );

	for( x=0 ; x<tglob.bw ; x++ ) for( y=0 ; y<tglob.bh ; y++ ) {
		if( tglob.buf[x][y] & 1 ) {
			t_GrBitmap( pixmap, gc, x*16, y*16, 16, 16, tile1 );
		} else {
			t_GrBitmap( pixmap, gc, x*16, y*16, 16, 16, tile2 );
		}
	}


	ttk_free_gc (gc);
	free (pixmap);
}


void cleanup_truchet( void ) 
{
}



int event_truchet( PzEvent *ev )
{
	switch (ev->type) {
	case PZ_EVENT_SCROLL:
		if( ev->arg > 0 ) {
		} else {
		}
		break;

	case PZ_EVENT_BUTTON_DOWN:
		switch( ev->arg ) {
		case( PZ_BUTTON_HOLD ):
			tglob.fullscreen = 1;
			ttk_window_hide_header( tglob.window );
			tglob.w = tglob.window->w;
			tglob.h = tglob.window->h;
			ev->wid->dirty = 1;
		default:
			break;
		}
		break;

	case PZ_EVENT_BUTTON_UP:
		switch( ev->arg ) {
		case( PZ_BUTTON_MENU ):
			pz_set_priority(PZ_PRIORITY_IDLE); 
			pz_close_window (ev->wid->win);
			break;

		case( PZ_BUTTON_HOLD ):
			tglob.fullscreen = 0;
			ttk_window_show_header( tglob.window );
			tglob.w = tglob.window->w;
			tglob.h = tglob.window->h;
			ev->wid->dirty = 1;
			break;

		case( PZ_BUTTON_PLAY ):
			break;

		case( PZ_BUTTON_ACTION ):
			if( !tglob.fullscreen ) tglob.clicks++;
			ev->wid->dirty = 1;
			break;

		default:
			break;
		}
		break;

	case PZ_EVENT_DESTROY:
		cleanup_truchet();
		break;

	case PZ_EVENT_TIMER:
		twiddle_buffer();
		ev->wid->dirty = 1;
		tglob.timer++;
		break;

	default:
		break;
	}
	return 0;
}


/* create the window */
static PzWindow *new_truchet_window_common( )
{
	/* create the window */
	tglob.window = pz_new_window( "Truchet", PZ_WINDOW_NORMAL );

	tglob.w = tglob.window->w;
	tglob.h = tglob.window->h;
	tglob.fullscreen = 0;

	tglob.bw = (ttk_screen->w/16) +1;
	tglob.bh = (ttk_screen->h/16) +1;

	/* create the widget */
	tglob.widget = pz_add_widget( tglob.window, 
				draw_truchet, event_truchet );

	/* 1 second timeout */
	pz_widget_set_timer( tglob.widget, 25 );

	/* we're waaaay more important than anyone else */
	pz_set_priority(PZ_PRIORITY_VITAL); 

	fill_buffer();

	/* done in here! */
	return pz_finish_window( tglob.window );
}


void init_truchet() 
{
	/* internal module name */
	tglob.module = pz_register_module( "truchet", cleanup_truchet );

	pz_menu_add_action("/Extras/Demos/Truchet", new_truchet_window_common );

	tglob.timer = 0;
	tglob.clicks = 0;
}

PZ_MOD_INIT (init_truchet)
