/* matrixgl - Cross-platform matrix screensaver
 * Copyright (C) Alexander Zolotov, Eugene Zolotov 2003.
 * Based on matrixgl 1.0 (see http://knoppix.ru/matrixgl.shtml)
 * -------------------------------------------
 * Written By:  Alexander Zolotov  <nightradio@gmail.com> 2003.
 *       and :  Eugene Zolotov     <sentinel@knoppix.ru> 2003.
 * Modified By: Vincent Launchbury <vincent@doublecreations.com> 2008,2009.
 * Modified By: Mustafa Ozgur <mitcore@gmail.com> 2018
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

/* Includes */

#include <stdio.h>   /* Always a good idea. */
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"     /* Prototypes */
#include "matrix1.h"    /* Font data */
#include "matrix2.h"    /* Image data */

#define abs(a) (((a)>0)?(a):(-(a)))

/* Global Variables */
float Z_Off = -89.0f;
unsigned char flare[16]={0,0,0,0,0,180,0}; /* Node flare texture */

//long timer=40;             /* Controls pic fade in/out */
//int mode2=1;               /* Initial speed boost (inits the light table) */
//int pic_mode=1;            /* 1-fade in; 2-fade out (controls pic_fade) */
//int pic_fade=0;            /* Makes all chars lighter/darker */
//int classic=0;             /* classic mode (no 3d) */
//int paused=0;
int num_pics= 2 - 1;         /* # 3d images (0 indexed) */
//GLenum color=GL_GREEN;     /* Color of text */




/* FPS Stats */
time_t last;               /* Tims since last stat was printed */
int showfps=0;
unsigned long sleeper = 0;
float fps=0;
int frames=0;              /* # frames shown since [last] */
int fpoll=2;               /* Print stats every [fpoll] seconds */
float maxfps=32.0;

void initSettings(MGLSettings * inSettings)
{
    
    inSettings->exit_mode=1;                                  //0 - none; 1 - exit mode (explode main text);
    inSettings->texture_add=0;
    inSettings->exit_angle=0;
    inSettings->timer=40;                                    //main timer;
    inSettings->timer2=0;
    inSettings->timer3=0;
    inSettings->mouse_counter=0;
    inSettings->mode2=1;
    inSettings->time2=0;
    inSettings->pic_mode=1;                                   //0 - none; 1 - pic fade in; 2 - pic fade out;
    inSettings->pic_fade=0;
    
    inSettings->paused=0;
    inSettings->classic=0;
}


/* Draw character #num on the screen. */
void draw_char(int mode, long num, float light, float x, float y, float z)
{
    float tx,ty;
    long num2;
    if (mode==1) light/=255;
    num2=num/10;
    ty=(float)num2/7;
    tx=(float)num/10;
    glColor4f(0.9,0.4,0.3,light);
    
    glTexCoord2f(tx, ty); glVertex3f( x, y, z);
    glTexCoord2f(tx+0.1, ty); glVertex3f( x+1, y, z);
    glTexCoord2f(tx+0.1, ty+0.166); glVertex3f( x+1, y-1, z);
    glTexCoord2f(tx, ty+0.166); glVertex3f( x, y-1, z);
}


/* Draw flare around white characters */
void draw_flare(float x,float y,float z)
{
    glColor4f(0.9,0.4,0.3,.75);
    
    glTexCoord2f(0, 0); glVertex3f( x-1, y+1, z);
    glTexCoord2f(0.75, 0); glVertex3f( x+2, y+1, z);
    glTexCoord2f(0.75, 0.75); glVertex3f( x+2, y-2, z);
    glTexCoord2f(0, 0.75); glVertex3f( x-1, y-2, z);
}

/* Draw green text on screen */
void draw_text1(MGLSettings * inSettings)
{
    float x,y;
    long p=0,b=0;
    int c,c_pic;
    
    if(!inSettings->paused && inSettings->pic_mode==1 && (inSettings->pic_fade+=3)>255) inSettings->pic_fade=255;
    if(!inSettings->paused && inSettings->pic_mode==2 && (inSettings->pic_fade-=3)<0) inSettings->pic_fade=0;
    for(y=_text_y;y>-_text_y;y--)
    {
        for(x=-_text_x;x<_text_x;x++)
        {
            if (x>-_rtext_x-1 && x<_rtext_x )
            {
                /* Main text */
                c=inSettings->text_light[p]-(inSettings->text[p]>>1);
                c+=inSettings->pic_fade;if(c>255) c=255;
                /* 3D Image offsets */
                c_pic=pic[b+inSettings->pic_offset]-(255-inSettings->pic_fade);
                if(c_pic<0) {c_pic=0;} c-=c_pic;if(c<0) {c=0;}
                
                if(!inSettings->paused) inSettings->bump_pic[p]=(float)(255-c_pic)/32;
                if(c>10 && inSettings->text[p]) draw_char(1, inSettings->text[p]+1,c,x,y, inSettings->bump_pic[p]);
                b++;
            } else {
                c=inSettings->text_light[p]-(inSettings->text[p]>>1);c+=inSettings->pic_fade;if(c>255) c=255;
                if (c>10 && inSettings->text[p]) draw_char(1, inSettings->text[p]+1, c,x,y,8);
            }
            p++;
        }
    }
}

/* Draw white characters and flares for each column */
void draw_text2(int mode,MGLSettings * inSettings)
{
    float x,y;
    long p=0;
    for(y=_text_y;y>-_text_y;y--){
        for(x=-_text_x;x<_text_x;x++){
            if(inSettings->text_light[p]>128 && inSettings->text_light[p+inSettings->text_x] < 10) {
                /* White character */
                if (!mode) {
                    draw_char(2, inSettings->text[p]+1,0.5,x,y, ((x>-_rtext_x-1 && x<_rtext_x ) ? inSettings->bump_pic[p] : 8));
                    /* Flare */
                } else {
                    draw_flare(x,y, ((x>-_rtext_x-1 && x<_rtext_x ) ? inSettings->bump_pic[p] : 8));
                }
            }
            
            p++;
        }
    }
}

int scroll(int mode,MGLSettings * inSettings)
{
    int a, s=0;
    
    if (inSettings->paused)
    {
        return mode;
    }
    
    
    for(a=(inSettings->text_x*text_y)+inSettings->text_x-1;a>inSettings->text_x-1;a--)
    {
        if(inSettings->speed[s])
        {
            inSettings->text_light[a]=inSettings->text_light[a-inSettings->text_x];
        }
        
        s++;if(s>=inSettings->text_x) s=0;
    }
    
    for(s=0,a=(inSettings->text_x*text_y)+inSettings->text_x-1;a>inSettings->text_x-1;a--)
    {
        if(inSettings->speed[s]>1)
        {
            inSettings->text_light[a]=inSettings->text_light[a-inSettings->text_x];
        }
        
        s++;if(s>=inSettings->text_x) s=0;
    }
    
    if(inSettings->timer2&1)
    {
        if(inSettings->timer==0 && !inSettings->classic)
        {
            inSettings->pic_mode=1;  /* pic fade in */
        }
        
        if(inSettings->timer>10)
        {
            inSettings->mode2=0;
            
            mode=0;
        } /* Initialization */
        
        if(inSettings->timer>140 && inSettings->timer<145 && !inSettings->classic)
        {
            inSettings->pic_mode=2; /* pic fade out */
        }
        
        if (inSettings->timer > 140 && inSettings->pic_offset==(num_pics+1)*(rtext_x*text_y))
        {
            inSettings->pic_offset+=rtext_x*text_y; /* Go from 'knoppix.ru' -> 'DC' */
            
            inSettings->timer=70;
            
            inSettings->pic_mode=1; /* Make DC dissapear quickly */
        }
        
        if(inSettings->timer>210)
        {
            inSettings->timer=-1;  /* back to start */
            
            inSettings->pic_offset+=rtext_x*text_y; /* Next pic */
            
            if(inSettings->pic_offset>(rtext_x*text_y*(num_pics))) inSettings->pic_offset=0;
        }
        
        inSettings->timer++;
        
        for(a=(inSettings->text_x*text_y)+inSettings->text_x-1;a>inSettings->text_x-1;a--)
        {
            inSettings->text_light[a]=inSettings->text_light[a-inSettings->text_x];
        }
        
        /* Clear top line in light table */
        for(a=1;a<inSettings->text_x;a++) inSettings->text_light[a]=253;
        
        for(s=0,a=(inSettings->text_x*text_y)/2; a<(inSettings->text_x*text_y); a++)
        {
            /* Make black bugs in top line */
            if(inSettings->text_light[a]==255)
                inSettings->text_light[s]=inSettings->text_light[s+inSettings->text_x]>>1;
            
            s++;if(s>=inSettings->text_x) s=0;
        }
    }
    
    inSettings->timer2++;
    
    return mode;
}


int make_change(MGLSettings * inSettings)
{
    unsigned int r=rand()&0xFFFF;
    if (inSettings->paused) {
        return 0;
    }
    r >>= 3;
    if(r < (inSettings->text_x*text_y))
    {
        inSettings->text[r]+=133; /* random character changes) */
    }
    r = rand() & 0xFFFF;
    r >>= 7;
    if(r < inSettings->text_x && inSettings->text_light[r] != 0)
    {
        inSettings->text_light[r]=255; /* white nodes */
    }
    
    if(inSettings->mode2)
    {
        return scroll(inSettings->mode2,inSettings);
    }
    
    return 1;
}


int cbRenderScene(MGLSettings * inSettings)
{  
    int tResult;
    
    glBindTexture(GL_TEXTURE_2D,1);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(0.0f,0.0f,Z_Off);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBegin(GL_QUADS);
    draw_text1(inSettings);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D,2);
    glBegin(GL_QUADS);
    draw_text2(0,inSettings);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D,3);
    glBegin(GL_QUADS);
    draw_text2(1,inSettings);
    glEnd();
    
    tResult=make_change(inSettings);
    
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    
    return tResult;
}


void cbResizeScene(int Width, int Height)
{
    glViewport(0, 0, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,200.0f);
//    GLKMatrix4MakePerspective(45.0f, (GLfloat)Width/(GLfloat)Height, 0.1f, 200.0f);
//    glMultMatrix( GLKMatrix4MakePerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,200.0f ).m ); // << .m is the GLfloat* you are accessing

    glMatrixMode(GL_MODELVIEW);
}


void ourInit(int Width,int Height,MGLSettings * inSettings) 
{
    /* Set up column speeds and character mappings */
    long a;
    int i,s;
    
    cbResizeScene(Width,Height);
    
    inSettings->text_x = ceil(text_y * ((float)Width/Height));
    
    inSettings->pic_offset=0;
    //inSettings->text_x*text_y*7;
    
    initSettings(inSettings);
    
    /* Initializations */
    if (inSettings->text_x % 2 == 1) {inSettings->text_x++;}
    if (inSettings->text_x < rtext_x) {inSettings->text_x=rtext_x; /* Sanity check */}
    inSettings->speed = tmalloc(inSettings->text_x);
    inSettings->text= tmalloc(inSettings->text_x*(text_y+1));
    inSettings->text_light = tmalloc(inSettings->text_x*(text_y+1));
    inSettings->bump_pic = tmalloc(sizeof(float) * (inSettings->text_x*(text_y+1)));
    memset(inSettings->text_light, 253, inSettings->text_x*(text_y+1));
    /* End allocations */
    
    /* Init the light tables */
    inSettings->mode2=0;
    for (i=0; i<500;i++) {
        make_change(inSettings);
        
        for(a=(inSettings->text_x*text_y)+inSettings->text_x-1;a>inSettings->text_x;a--)
            {
                inSettings->text_light[a]=inSettings->text_light[a-inSettings->text_x];
            }
        
        for(a=1;a<inSettings->text_x;a++) {inSettings->text_light[a]=253; /* Clear top line */}
        
        for(s=0,a=(inSettings->text_x*text_y)/2; a<(inSettings->text_x*text_y); a++){
            if(inSettings->text_light[a]==255) {inSettings->text_light[s]=inSettings->text_light[s+inSettings->text_x]>>1;}
            s++;if(s>=inSettings->text_x) {s=0;}
        }
    }
    inSettings->mode2=1;
    
    

    for(a=0;a<(inSettings->text_x*text_y);a++) {inSettings->text[a] = rand() & 63;}
    for(a=0;a<inSettings->text_x;a++) {
        inSettings->speed[a] = rand() & 1;
        if (!a && inSettings->speed[a]==inSettings->speed[a-1])
        {
            inSettings->speed[a]=2; /* Collisions goto speed 3 */
        }
    }
    
    /* Make Textures */
    glPixelTransferf(GL_GREEN_SCALE, 1.15f); /* Give green a bit of a boost */
    
//    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 512, 256, inSettings->color, GL_RGBA, GL_UNSIGNED_BYTE, (void *)font);
//    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,1);
       gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA8, 512, 256, inSettings->color, GL_UNSIGNED_BYTE, (void *)font);
    
//    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA8, 512, 256, GL_LUMINANCE, GL_RGBA, GL_UNSIGNED_BYTE, (void *)font);
//    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,2);
    gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA8, 512, 256, GL_LUMINANCE, GL_UNSIGNED_BYTE, (void *)font);

//    glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA8, 4, 4, GL_LUMINANCE, GL_RGBA, GL_UNSIGNED_BYTE, (void *)font);
//    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,3);
    gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA8, 4, 4, GL_LUMINANCE, GL_UNSIGNED_BYTE, (void *)flare);
    
    /* Some pretty standard settings for wrapping and filtering. */
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glShadeModel(GL_SMOOTH);
    
    /* A handy trick -- have surface material mirror the color. */
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
}

/* malloc w/ error checking  */
void *tmalloc(size_t n)
{
    void *p = malloc(n);
    if (!p && n != 0)
        {
            exit(EXIT_FAILURE);
        }
    return p;
}

