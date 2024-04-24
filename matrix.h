/* matrixgl - Cross-platform matrix screensaver
 * Copyright (C) Alexander Zolotov, Eugene Zolotov 2003. 
 * Based on matrixgl 1.0 (see http://knoppix.ru/matrixgl.shtml)
 * -------------------------------------------
 * Written By:  Alexander Zolotov  <nightradio@gmail.com> 2003.
 *       and :  Eugene Zolotov     <sentinel@knoppix.ru> 2003.
 * Modified By: Vincent Launchbury <vincent@doublecreations.com> 2008,2009.
 * -------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  US
 */
 
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
//#include <GLKit/GLKit.h>
#include <time.h>

#define FRAME_RATE_SAMPLES 30

#define rtext_x 120
#define _rtext_x rtext_x/2
#define text_y 70
#define _text_y text_y/2
#define _text_x inSettings->text_x/2
 
extern float Z_Off;
extern int num_pics;

typedef struct MGLSettings
{
    int text_x;
	unsigned char * speed;
    unsigned char * text;
    unsigned char * text_light; //255 - white; 254 - none; 
	float * bump_pic;
    
	GLenum color;
	
	int classic;
	int paused;
	
    int pic_offset;
    int exit_mode;                                  //0 - none; 1 - exit mode (explode main text);
    float texture_add;
    float exit_angle;
    long timer;                                    //main timer;
    long timer2,timer3,mouse_counter;
    int mode2;
    time_t  time2;
    int pic_mode;                                   //0 - none; 1 - pic fade in; 2 - pic fade out;
    int pic_fade;
} MGLSettings;

void initSettings(MGLSettings * inSettings);

int cbRenderScene(MGLSettings * inSettings);

int make_change(MGLSettings * inSettings);

void make_text(void);
void draw_char(int mode, long num, float light, float x, float y, float z);
void draw_flare(float x,float y,float z);
void draw_text1(MGLSettings * inSettings);
void draw_text2(int mode,MGLSettings * inSettings);
int scroll(int mode,MGLSettings * inSettings);


void MouseFunc(int x, int y);
void cbKeyPressed(unsigned char key, int x, int y);
void ourBuildTextures(void);
void cbResizeScene(int Width, int Height);
void ourInit(int Width,int Height,MGLSettings * inSettings);
void load_texture(void);
void *tmalloc(size_t n);
#ifdef NIX_MODE
char get_ascii_keycode(XEvent *ev);
int htoi(char *str);
#endif

#define clamp(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
