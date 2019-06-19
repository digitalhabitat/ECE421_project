// Doge Pokeball Game
// Michael Miller and Roger Biak
// 12/5/2018

// note this program seems to be run properly only on a specific board
// on other zebboards the display does not output to screen but
// seems to be running the program (LEDs display pushbutton values)
// this issue has not been resolved, is it assume the problem is due to the
// reset_rtl port mapping in vivado
#include <stdio.h>
#include "xil_io.h"
#include "platform.h"
#include <math.h>

#include "data_table.h"
#include "pikachu.h"
#include "background.h"
// for gpio
#include "xparameters.h"
#include "xgpio.h"

//for time keeping
#include "xtime_l.h"

#define BaseAddress 0x43C00000
#define Reg0_Offset 0x00    // data in for RAM
#define Reg1_Offset 0x04    // address for RAM
#define Reg2_Offset 0x08    // Write enable for RAM
#define Reg3_Offset 0x0C	// data out from RAM

#define WordsPerLine 64 // used by set_pix
#define MaxHor 636
#define MaxVer 477
#define MaxBalls 40


void initialize_picture(int color);
void set_pix(int x,int y, int c);
void set_npix(int x,int y,int nx, int c);
void set_block(int width, int height, int Xpos, int Ypos, int color);
void draw_block(int Xpos, int Xpos_next, int Ypos, int Ypos_next, int width, int height,int color, int erase_color);
void move_block(int *Xpos, int *Xpos_next, int *Ypos, int *Ypos_next, int step, char *left, char *left_next, char *up, char *up_next, int height, int width );
void draw_bitmap(int psb_check, uint8_t sprite_table[], int Xpos, int Xpos_temp, int Ypos, int Ypos_temp, int step, int width, int height,int *frame, int size, int erase_color);
void draw_player(sprite *pokemon, int step, int size, int psb_check, int erase_color);

void replace_bitmap(int xpos, int ypos, int width, int height, int size, struct background *scene );

void draw_sprite(int Xpos, int Ypos, uint8_t sprite_table[], int sprite_width, int sprite_height, int size, int flip);

void print_pixel(unsigned char pixeline, int j, int xCord, int yCord, int color2);
void print_pos(unsigned char table[], unsigned char index, int xCordinate, int yCordinate, int colour);
void psh_move(int psb_check, int *Xpos, int *Ypos, int step, int *Xpos_temp, int *Ypos_temp);

void move_sprite(sprite2 *ball);
void draw_ball(int x, int y, int frame, sprite2 *ball);


//
//   (Xpos,Ypos)
//   <---------width---------> ^
//   <                       > |
//   <                       > |height
//   <                       > |
//   <-----------------------> v
//


int main (void){

	sprite2 ball[MaxBalls];

	// Time instantiations
	XTime tStart, tEnd;
	init_platform();
	int sec_count, sec_count_temp;
	int digit0, digit1, digit11, digit2;
	char time[3];
	int life;

	// GPIO instantiations
	XGpio dip;
	XGpio push;
	XGpio led;

	int psb_check, dip_check;

	xil_printf("-- Start of the Program --\r\n");

	XGpio_Initialize(&dip, XPAR_SW_8BIT_DEVICE_ID);
	XGpio_SetDataDirection(&dip, 1, 0xffffffff);

	XGpio_Initialize(&push, XPAR_BTNS_5BIT_DEVICE_ID);
	XGpio_SetDataDirection(&push, 1, 0xffffffff);

	XGpio_Initialize(&led, XPAR_LEDS_8BITS_DEVICE_ID);
	XGpio_SetDataDirection(&led, 1, 0x00);

	int erase_color=0;
	int menu = 1;

 while(1){

	 // start menu ------------------------------------------------------------
	 usleep(16666-137);
	 while(XGpio_DiscreteRead(&push, 1) != 0 );

	 // draw background
	 draw_sprite(0, 0, test.bitmap, 320, 240, 2, 0);
	 draw_sprite(180,200,dodge.bitmap, dodge.width, dodge.height, 1, 0);
	 set_block(280, 30, 200, 330,7);
	 char menu_sel[] = "PRESS ANY BUTTON TO BEGIN";
	 for (int i = 0; i < sizeof(menu_sel); ++i) {
	 	 print_pos(data_table, index_table[menu_sel[i]],200+i*10,330,0);
	 };
	 pika.xpos =  170;
	 pika.ypos =  330;
	 while(menu == 1 ){
		 // loop here
		 psb_check = XGpio_DiscreteRead(&push, 1);
		 if(psb_check != 0){
			 menu = 0;;
		 }
		 draw_player(&pika,pika.step, 1, psb_check, erase_color);
		 usleep(16666-137);
	 }// main menu while loop exits on button press


	 // initialize new game parameters -----------------------------------------
	 // time init
	 XTime_GetTime(&tStart);
	 time[3] = "000";
	 sec_count_temp = 0;

	 // ball init
	 for (int i =0; i<MaxBalls; i++){
	 		ball[i].width = 12;
	 		ball[i].height =12;
	 		ball[i].xpos_next = 50+i*15; // each spawn at a slightly different xpos
	 		ball[i].ypos_next = 130;
	 		ball[i].xpos_temp = 0;
	 		ball[i].ypos_temp = 0;
	 		ball[i].frame = 0;
	 		ball[i].temp = 0;
	 		ball[i].up_next = 1;
	 		ball[i].left_next = 1;
	 		ball[i].left = 1;
	 		ball[i].step = 3; // larger values make the balls move faster
	 		ball[i].size = 1;
	 }

	 // draw background
	 draw_sprite(0, 0, test.bitmap, 320, 240, 2, 0);


	 // life init for new game
	 pika.temp=6;
	 life = pika.temp;
	 draw_sprite(616,80,heart.bitmap,7,7,2,0);
	 draw_sprite(600,80,heart.bitmap,7,7,2,0);
	 draw_sprite(584,80,heart.bitmap,7,7,2,0);

	 // starting new game ------------------------------------------------------
	 while(life>0){


		 // Display Seconds Elapsed
		 XTime_GetTime(&tEnd);
		 sec_count = (int)((tEnd-tStart)/(COUNTS_PER_SECOND));
		 if(sec_count != sec_count_temp){
			 set_block(30,30,30,70, 0);
			 	 digit0 = sec_count%10; // first digit
			 	 digit11 = (sec_count-digit0)%100; // place holder
			 	 digit1 = digit11/10; // second digit
			 	 digit2 = ( (sec_count-digit1-digit0)%1000)/100; // third digit
			 	 time[0]=digit0+'0';
			 	 time[1]=digit1+'0';
			 	 time[2]=digit2+'0';

			 	for (int i = 0; i < 3; ++i) {
			 		print_pos(data_table, index_table[time[i]],50-i*10,70,7);
			 	}
			 sec_count_temp = sec_count;
		 }

		 // Display hearts
		 if(pika.temp != life){
			 if (pika.temp == 5){
				 //set_block(7,7, 616,50,0);
				 draw_sprite(616,80,heart.bitmap+49,7,7,2,0);
			 }
			 else if(pika.temp == 4){
				 //set_block(7,7, 608,50,0);
				 draw_sprite(616,80,heart.bitmap+2*49,7,7,2,0);
			 }
			 else if(pika.temp == 3){
				 //set_block(7,7, 600,50,0);
				 draw_sprite(600,80,heart.bitmap+49,7,7,2,0);
			 }
			 else if(pika.temp == 2){
			 	//set_block(7,7, 600,50,0);
			 	draw_sprite(600,80,heart.bitmap+2*49,7,7,2,0);
			 }
			 else if(pika.temp == 1){
			 	//set_block(7,7, 600,50,0);
			 	draw_sprite(584,80,heart.bitmap+49,7,7,2,0);
			 }
			 else if(pika.temp == 1){
					//set_block(7,7, 600,50,0);
					draw_sprite(584,80,heart.bitmap+2*49,7,7,2,0);
				 }
			 life =  pika.temp;
		 }

		 // Read GPIO
		 psb_check = XGpio_DiscreteRead(&push, 1);
		 dip_check = XGpio_DiscreteRead(&dip, 1);
		 XGpio_DiscreteWrite(&led, 1, psb_check);
		 psh_move(psb_check, &pika.xpos,&pika.ypos, pika.step, &pika.xpos_temp, &pika.ypos_temp);

		// Move and Draw pokeballs
		for (int i=0;i<(sec_count/2)%40;i++){
		move_sprite(&ball[i]);
	    draw_ball(ball[i].xpos_next, ball[i].ypos_next, ball[i].frame, &ball[i]);
		}

		// Draw Player
		draw_player(&pika,pika.step, dip_check+1, psb_check, erase_color);
		usleep(16666-137);

	 }// new game while loop exits on life<=0

	 // Display game over------------------------------------------------------------
	 set_block(90, 30, 250, 300,0);
	 char gameover[] = "GAME OVER";
	 for (int i = 0; i < 9; ++i) {
		 print_pos(data_table, index_table[gameover[i]],250+i*10,300,7);
	 };
	 set_block(380, 30, 130, 330,0);
	 char newgame[] = "PRESS CENTER BUTTON TO GO TO MAIN MENU";
	 for (int i = 0; i < sizeof(newgame); ++i) {
	 	print_pos(data_table, index_table[newgame[i]],130+i*10,330,7);
	 };
	 while(life == 0){
		 // loop here
		 psb_check = XGpio_DiscreteRead(&push, 1);
		 if(psb_check == 1){
			 life = 6;
			 menu = 1;
		 }
	 }// game over loop exits on button press
 }// cycle back to infinite while loop restart game

}//end main
//------------------------------------------------------------------------------
void psh_move(int psb_check, int *Xpos, int *Ypos, int step, int *Xpos_temp, int *Ypos_temp){


	    *Xpos_temp = *Xpos;
		*Ypos_temp = *Ypos;

		//calculate next position
	   if (psb_check == 8){
		  // right
		  *Xpos = *Xpos + step;
	   }
	   else if(psb_check == 4){
		  // left
		   *Xpos = *Xpos - step;
	   }
	   else if(psb_check == 16){
		  // up
		   *Ypos = *Ypos - step;
	    }
	    else if(psb_check == 2){
	      // down
	      *Ypos = *Ypos + step;
	    }
	    else if(psb_check == 6){
	      // down-left
	    	*Xpos = *Xpos - step;
	    	*Ypos = *Ypos + step;
	    }
	    else if(psb_check == 10){
	      // down-right
	    	*Ypos = *Ypos + step;
	      *Xpos = *Xpos + step;
	    }
	    else if(psb_check == 24){
	      // up-right
	    	*Ypos = *Ypos - step;
	      *Xpos = *Xpos + step;
	    }
	    else if(psb_check == 20){
	      // up-left
	    	*Ypos = *Ypos - step;
	    	*Xpos = *Xpos - step;
	    }
}

void draw_sprite(int Xpos, int Ypos, uint8_t sprite_table[], int sprite_width, int sprite_height, int size, int flip){

	// prints image on the screen pixel by pixel using set_block
	if (flip == 0) {
		for (int j=0; j<sprite_height; j++){
			for(int i=0; i<sprite_width; i++){
				// 9 signifies transparency
				set_block(1*size,1*size,Xpos+i*size,Ypos+j*size,sprite_table[j*sprite_width+i]);
			}
		}
	}

	// flip image along horizontal axis
	else {
		for (int j=0; j<sprite_height; j++){
			for(int i=0; i<sprite_width; i++){
				// 9 signifies transparency
				set_block(1*size,1*size,Xpos+i*size,Ypos+j*size,sprite_table[j*sprite_width+(sprite_width-i-1)]);
			}
		}
	}
}

void print_pos(unsigned char table[], unsigned char index, int xCordinate, int yCordinate, int colour)
{
	// used to print characters to the screen
    unsigned int offset = index*32;
    int i;
        for (i=2; i<34; i++) {
        print_pixel(table[offset], i, xCordinate, yCordinate, colour);
        offset++;
        yCordinate++;
        }
}

void print_pixel(unsigned char pixeline, int j, int xCord, int yCord, int color2)
{
	// used by print_pos() to print characters to the screen
	int pixelArray[9] = {0};
	int k;

	if (j%2 == 0) {
		if (pixeline & 0x80) {
			pixelArray[0] = 1;
		}
		if (pixeline & 0x40) {
			pixelArray[1] = 1;
		}
		if (pixeline & 0x20) {
			pixelArray[2] = 1;
		}
		if (pixeline & 0x10) {
			pixelArray[3] = 1;
		}
		if (pixeline & 0x08) {
			pixelArray[4] = 1;
		}
		if (pixeline & 0x04) {
			pixelArray[5] = 1;
		}
		if (pixeline & 0x02) {
			pixelArray[6] = 1;
		}
		if (pixeline & 0x01) {
			pixelArray[7] = 1;
		}
	}

	else {
		if (pixeline & 0x80) {
			pixelArray[8] = 1;
		}
	}
	for (k=0; k<9; k++) {
		if (pixelArray[k] == 1) {
			set_block(1,1,xCord,yCord,color2);
			set_block(1,1,xCord,yCord,color2);
		}
		xCord++;
	}
}
//----------------------------------------------------------------

void move_sprite(sprite2 *ball){

	ball->ypos_temp=ball->ypos_next;
	ball->xpos_temp=ball->xpos_next;

	// frame to offset bitmap array to animate sprite
	ball->frame++;
	if (ball->frame >= 4){
		ball->frame = 0;
	}


	ball->up=ball->up_next;
	ball->left=ball->left_next;
	ball->xpos=ball->xpos_next;
	ball->ypos=ball->ypos_next;

	if (ball->left==0) {
		// does not hit right border
	  if (ball->xpos<=(MaxHor-ball->width-ball->step)) {
       ball->xpos_next=ball->xpos+ball->step;
       ball->left_next=0;
       }else{
    	   // hits right border
       ball->xpos_next= MaxHor-ball->width;
       ball->left_next=1;
	  }
   }else {
	  if (ball->xpos>ball->step) {
	// does not hit left border
       ball->xpos_next=ball->xpos-ball->step;
       ball->left_next=1;
     }else{
    // hits left border
       ball->xpos_next=1;
       ball->left_next=0;
	  }
	}
	if (ball->up==0) {

	  if (ball->ypos<=(MaxVer-ball->height-ball->step)) {
	// does not hit bottom border
       ball->ypos_next=ball->ypos+ball->step;
       ball->up_next=0;
     }else{
    // hits bottom border
       ball->ypos_next=MaxVer-ball->height;
       ball->up_next=1;
	  }
   }else {
	  if (ball->ypos>120+ball->step) {
	// does not hit top border
       ball->ypos_next=ball->ypos-ball->step;
       ball->up_next=1;
     }else{
    	 // hits top border
	    ball->ypos_next=120;
       ball->up_next=0;
	  }
	}

// pikachu collisions
	if ( (ball->left==0) & (ball->up==0) ){
	// ball moving right down
		if(  (ball->xpos>(pika.xpos-ball->width-ball->step)) & (ball->xpos<pika.xpos+2) & (ball->ypos >= pika.ypos) & (ball->ypos < pika.ypos+pika.height)){
		// if hits left side
			ball->xpos_next= pika.xpos-ball->width;
			ball->left_next=1;
			pika.temp--;
		}
		else if((ball->ypos>(pika.ypos-ball->height-ball->step)) & (ball->ypos<pika.ypos+2) & (ball->xpos >= pika.xpos) & (ball->xpos < pika.xpos+pika.width)){
		// if hits top side
			ball->ypos_next=pika.ypos-ball->height;
			ball->up_next=1;
			pika.temp--;
		}

	}else if(ball->left==0 &  ball->up==1){
	// ball moving right up
		//
		if((ball->xpos>(pika.xpos-ball->width-ball->step)) & (ball->xpos<pika.xpos+2) & (ball->ypos >= pika.ypos) & (ball->ypos < pika.ypos+pika.height)){
		// if hits left side  -->|
			ball->xpos_next= pika.xpos-ball->width;
			ball->left_next=1;
			pika.temp--;
		}
		//& (ball->ypos<pika.ypos+pika.height-2) & (ball->xpos >= pika.xpos) & (ball->xpos < pika.xpos+pika.width)
		else if((ball->ypos<(pika.ypos+pika.height+ball->step)) & (ball->ypos>pika.ypos+pika.height-2) & (ball->xpos >= pika.xpos) & (ball->xpos < pika.xpos+pika.width) ){
		// if hits bottom side
		   ball->ypos_next=pika.ypos+pika.height;
	       ball->up_next=0;
	       pika.temp--;
		}
	}else if(ball->left==1 &  ball->up==0){
	// ball moving left down
		if((ball->xpos<(pika.xpos+pika.width)) & (ball->xpos>(pika.xpos+pika.width-10))& (ball->ypos >= pika.ypos) & (ball->ypos < pika.ypos+pika.height) ){
		// if hits right side  |<----
			ball->xpos_next= pika.xpos+pika.width;
			ball->left_next=0;
			pika.temp--;
		}
		else if((ball->ypos>(pika.ypos-ball->height-ball->step)) & (ball->ypos<pika.ypos+2) & (ball->xpos >= pika.xpos) & (ball->xpos < pika.xpos+pika.width)){
		// if hits top side
			ball->ypos_next=pika.ypos-ball->height;
			ball->up_next=1;
			pika.temp--;
		}
	}else if(ball->left==1 &  ball->up==1){
	// ball moving left up
		if( (ball->xpos<(pika.xpos+pika.width)) & (ball->xpos>(pika.xpos+pika.width-10)) & (ball->ypos >= pika.ypos) & (ball->ypos < pika.ypos+pika.height)){
			// if hits right side  |<----
			ball->xpos_next= pika.xpos+pika.width;
			ball->left_next=0;
			pika.temp--;
		}
		else if((ball->ypos<(pika.ypos+pika.height+ball->step)) & (ball->ypos>pika.ypos+pika.height-2) & (ball->xpos >= pika.xpos) & (ball->xpos < pika.xpos+pika.width)){
		// if hits bottom side
		   ball->ypos_next=pika.ypos+pika.height;
	       ball->up_next=0;
	       pika.temp--;
		}
	}
}
void draw_ball(int x, int y, int frame, sprite2 *ball){

int size = ball->size;
int step = ball->step;
	if ((ball->xpos_next>=ball->xpos)&(ball->ypos_next>=ball->ypos) ) {
		//down right
		replace_bitmap(ball->xpos_temp, ball->ypos_temp, ball->width*size, step, size, &test );//down
		replace_bitmap(ball->xpos_temp, ball->ypos_temp,step, ball->height*size, size, &test );//right
	}else if((ball->xpos_next>=ball->xpos)&(ball->ypos_next<ball->ypos)){
		//up right
		replace_bitmap(ball->xpos_temp, ball->ypos_temp+ball->height-step,ball->width, step, size, &test );//up
		replace_bitmap(ball->xpos_temp, ball->ypos_temp,step, ball->height*size, size, &test );//right
	 }else if((ball->xpos_next<ball->xpos)&(ball->ypos_next>=ball->ypos)){
		 //down left
		replace_bitmap(ball->xpos_temp, ball->ypos_temp, ball->width*size, ball->step, size,  &test );//down
		replace_bitmap(ball->xpos_temp+ball->width*size-step, ball->ypos_temp,ball->step, ball->height*size, size, &test );//left
	 }else{
		 //up left
		 replace_bitmap(ball->xpos_temp, ball->ypos_temp+ball->height-step,ball->width, step, size, &test );//up
		 replace_bitmap(ball->xpos_temp+ball->width*size-step, ball->ypos_temp,ball->step, ball->height*size, size, &test );//left

	}
	draw_sprite(x, y, pokeball.bitmap+144*frame, pokeball.width, pokeball.height, pokeball.size, 0);
}

void move_block(int *Xpos, int *Xpos_next, int *Ypos, int *Ypos_next, int step, char *left, char *left_next, char *up, char *up_next, int height, int width ){
	*up=*up_next;
	*left=*left_next;
	*Xpos=*Xpos_next;
	*Ypos=*Ypos_next;

	if (*left==0) {
	  if (*Xpos<=(MaxHor-width-step)) {
       *Xpos_next=*Xpos+step;
       *left_next=0;
     }else{
       *Xpos_next= MaxHor-width;
       *left_next=1;
	  }
   }else {
	  if (*Xpos>step) {
       *Xpos_next=*Xpos-step;
       *left_next=1;
     }else{
       *Xpos_next=1;
       *left_next=0;
	  }
	}
	if (*up==0) {
	  if (*Ypos<=(MaxVer-height-step)) {
       *Ypos_next=*Ypos+step;
       *up_next=0;
     }else{
       *Ypos_next=MaxVer-height;
       *up_next=1;
	  }
   }else {
	  if (*Ypos>step) {
       *Ypos_next=*Ypos-step;
       *up_next=1;
     }else{
	    *Ypos_next=1;
       *up_next=0;
	  }
	}

}


void draw_block(int Xpos, int Xpos_next, int Ypos, int Ypos_next, int width, int height,int color, int erase_color){
// not used in this project
	if ((Xpos_next>=Xpos)&(Ypos_next>=Ypos)) {
		 	  set_block(width,Ypos_next-Ypos, Xpos, Ypos, erase_color);
		       set_block(Xpos_next-Xpos,height-(Ypos_next-Ypos), Xpos, Ypos_next, erase_color);
		     }else if((Xpos_next>=Xpos)&(Ypos_next<Ypos)){
		       set_block(width,Ypos-Ypos_next, Xpos, Ypos_next+height, erase_color);
		       set_block(Xpos_next-Xpos,height-(Ypos-Ypos_next), Xpos, Ypos, erase_color);
		     }else if((Xpos_next<Xpos)&(Ypos_next>=Ypos)){
		       set_block(width,Ypos_next-Ypos, Xpos, Ypos, erase_color);
		       set_block(Xpos-Xpos_next,height-(Ypos_next-Ypos), Xpos_next+width, Ypos_next, erase_color);
		 	}else if((Xpos_next<Xpos)&(Ypos_next<Ypos)){
		 	  set_block(width,Ypos-Ypos_next, Xpos, Ypos_next+height, erase_color);
		       set_block(Xpos-Xpos_next,height-(Ypos-Ypos_next), Xpos_next+width, Ypos, erase_color);
		     }

		 	//draw block at next position
		 	//set_block(width,heigth, Xpos_next, Ypos_next, color);
		     //draw only new parts
		 	if ((Xpos_next>=Xpos)&(Ypos_next>=Ypos)) {
		 	  set_block(width,Ypos_next-Ypos, Xpos_next, Ypos+height, color);
		       set_block(Xpos_next-Xpos,height-(Ypos_next-Ypos), Xpos+width, Ypos_next, color);
		     }else if((Xpos_next>=Xpos)&(Ypos_next<Ypos)){
		       set_block(width,Ypos-Ypos_next, Xpos_next, Ypos_next, color);
		       set_block(Xpos_next-Xpos,height-(Ypos-Ypos_next), Xpos+width, Ypos, color);
		     }else if((Xpos_next<Xpos)&(Ypos_next>=Ypos)){
		       set_block(width,Ypos_next-Ypos, Xpos_next, Ypos+height, color);
		       set_block(Xpos-Xpos_next,height-(Ypos_next-Ypos), Xpos_next, Ypos_next, color);
		 	}else if((Xpos_next<Xpos)&(Ypos_next<Ypos)){
		 	  set_block(width,Ypos-Ypos_next, Xpos_next, Ypos_next, color);
		       set_block(Xpos-Xpos_next,height-(Ypos-Ypos_next), Xpos_next, Ypos, color);
		     }

}
void draw_player(sprite *pokemon, int step, int size, int psb_check, int erase_color){

	// redundant comments of previous code to show similarity to draw_block()
	pokemon->frame = pokemon->frame + 2;
	if (pokemon->frame > 50){
		pokemon->frame = 0;
	}

	if (psb_check == 6) {
		// down left
		//set_block(width, step, pokemon->xpos_temp, pokemon->ypos-step, erase_color);
		//set_block(width-step, height, pokemon->xpos_temp+width-step, pokemon->ypos, erase_color);
		replace_bitmap(pokemon->xpos_temp, pokemon->ypos_temp, pokemon->width*size, step, size, &test );//down
		replace_bitmap(pokemon->xpos_temp+pokemon->width*size-step, pokemon->ypos_temp,step, pokemon->height*size, size, &test );//left
		if (pokemon->frame < 12 || (pokemon->frame > 25 && pokemon->frame< 37)  ){
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+3*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,0);
		}else{
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+4*( pokemon->width*(pokemon->height) ) , pokemon->width, pokemon->height,size,0);
		}

	}else if(psb_check == 20){
		// up left
		//set_block(width, step, pokemon->xpos_temp, pokemon->ypos+height, erase_color);
		//set_block(width-step, height, pokemon->xpos_temp+width-step, pokemon->ypos, erase_color);
		replace_bitmap(pokemon->xpos_temp, pokemon->ypos_temp+pokemon->height*size-step,pokemon->width*size, step, size, &test );//up
		replace_bitmap(pokemon->xpos_temp+pokemon->width*size-step, pokemon->ypos_temp,step, pokemon->height*size, size, &test );//left

		if (pokemon->frame < 12 || (pokemon->frame > 25 && pokemon->frame < 37)  ){
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+3*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,0);
		}else{
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+4*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,0);
		}

	}else if(psb_check == 10){
		// down right
		//set_block(width, step, pokemon->xpos_temp, pokemon->ypos-step, erase_color);
		//set_block(step, height, pokemon->xpos_temp, pokemon->ypos, erase_color);
		replace_bitmap(pokemon->xpos_temp, pokemon->ypos_temp, pokemon->width*size, step, size, &test );//down
		replace_bitmap(pokemon->xpos_temp, pokemon->ypos_temp,step, pokemon->height*size, size, &test );//right
		if (pokemon->frame < 12 || (pokemon->frame > 25 && pokemon->frame < 37)  ){
					draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+3*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,1);
				}else{
					draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+4*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,1);
				}

	}else if(psb_check == 24){
		//up right
		//set_block(width, step, pokemon->xpos_temp, pokemon->ypos+height, erase_color);
		//set_block(step, height, pokemon->xpos_temp, pokemon->ypos, erase_color);

		replace_bitmap(pokemon->xpos_temp, pokemon->ypos_temp+pokemon->height*size-step,pokemon->width*size, step, size, &test );//up
		replace_bitmap(pokemon->xpos_temp, pokemon->ypos_temp,step, pokemon->height*size, size,  &test );//right
		if (pokemon->frame < 12 || (pokemon->frame > 25 && pokemon->frame < 37)  ){
					draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+3*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,1);
				}else{
					draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+4*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,1);
				}
	}else if(psb_check == 16){
		// up
		replace_bitmap(pokemon->xpos_temp, pokemon->ypos_temp+pokemon->height*size-step,pokemon->width*size, step, size,  &test );//up
		//set_block(width, step, pokemon->xpos_temp, pokemon->ypos+height, erase_color);
		draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+2*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,0);
	}
	else if(psb_check == 2){
		// down
		//set_block(width, step, pokemon->xpos_temp, pokemon->ypos-step, erase_color);
		replace_bitmap(pokemon->xpos_temp, pokemon->ypos_temp, pokemon->width*size, step, size,  &test );//down
		if (pokemon->frame < 12 || (pokemon->frame > 25 && pokemon->frame < 37)  ){
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,0);
		}else{
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap, pokemon->width, pokemon->height,size,0);
		}
	}else if(psb_check == 4){
		//left
		//set_block(width-step, height, pokemon->xpos_temp+width-step, pokemon->ypos, erase_color);
		//replace_bitmap(int xpos, int ypos, int width, int height, int size, struct background *scene )
		//replace_bitmap(pokemon->xpos+step, pokemon->ypos_temp, step, pokemon->height, size, &test );
		replace_bitmap(pokemon->xpos_temp+pokemon->width*size-step, pokemon->ypos_temp,step, pokemon->height*size, size,  &test );//left
		if (pokemon->frame < 12 || (pokemon->frame > 25 && pokemon->frame < 37)  ){
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+3*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,0);
		}else{
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+4*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,0);
		}
	}else if(psb_check == 8){
		// right
		//set_block(step, height, pokemon->xpos_temp, pokemon->ypos, erase_color);
		//draw_sprite(pokemon->xpos_temp, pokemon->ypos_temp, background, step, height, 2, 0);
		//draw_sprite(pokemon->xpos_temp, pokemon->ypos_temp, background + (pokemon->xpos_temp), pokemon->height,240, 2, 0); //Stripe Background
		//set_block(2,2,i*2+x0,j*2+y0,scene->bitmap[w1*(y1)+x1+j*w1+i]);
		replace_bitmap(pokemon->xpos_temp, pokemon->ypos_temp,step, pokemon->height*size, size,  &test );//right

		if (pokemon->frame < 12 || (pokemon->frame > 25 && pokemon->frame < 37)  ){
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+3*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,1);
		}else{
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+4*(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,1);
		}
	}else if(psb_check == 0){
		// no movement
		if (pokemon->frame < 25 ){
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap+(pokemon->width*(pokemon->height)), pokemon->width, pokemon->height,size,0);
		}else{
			draw_sprite(pokemon->xpos, pokemon->ypos, pokemon->bitmap, pokemon->width, pokemon->height,size,0);
		}
	}

}
void replace_bitmap(int xpos, int ypos, int width, int height, int size, struct background *scene ){
// this functions draws a specified section of the background
// The *.5 and is necessary due to the background only begin 320x240 pixels (for write speed) begin scaled up for 640x480
// the "scale" data is not fully integrated yet but later versions could fix this to support different size bitmaps
// at the moment this function requires a 320X240 pixel background
	int w0 =640;
	int x0 = xpos;
	int y0 = ypos;
	int w1 = w0/scene->scale;
	int y1 = y0/scene->scale;
	int x1 = x0/scene->scale;
	int scale = scene->scale;

	for (int j=0; j<height*.5; j++){
		for(int i=0; i<width*.5; i++){
		set_block(scale,scale,i*2+x0,j*2+y0,scene->bitmap[w1*(y1)+x1+j*w1+i]);
		}
	}
}

void draw_bitmap(int psb_check, uint8_t sprite_table[], int Xpos, int Xpos_temp, int Ypos, int Ypos_temp, int step, int width, int height,int *frame, int size, int erase_color){

	// the function draw_bitmap is depreciated use draw_sprite()
	width = width*size;
	height = height*size;

	*frame = *frame +2;
	if (*frame > 50){
		*frame = 0;
	}

	if (psb_check == 6) {
		// down left
		set_block(width, step, Xpos_temp, Ypos-step, erase_color);
		set_block(width-step, height, Xpos_temp+width-step, Ypos, erase_color);
		//draw_sprite(Xpos, Ypos, sprite_table, pika_width, pika_height,size,0);
		if (*frame < 12 || (*frame > 25 && *frame < 37)  ){
			draw_sprite(Xpos, Ypos, sprite_table+3*(pika_width*pika_height), pika_width, pika_height,size,0);
		}else{
			draw_sprite(Xpos, Ypos, sprite_table+4*(pika_width*pika_height), pika_width, pika_height,size,0);
		}

	}else if(psb_check == 20){
		// up left
		set_block(width, step, Xpos_temp, Ypos+height, erase_color);
		set_block(width-step, height, Xpos_temp+width-step, Ypos, erase_color);
		if (*frame < 12 || (*frame > 25 && *frame < 37)  ){
			draw_sprite(Xpos, Ypos, sprite_table+3*(pika_width*pika_height), pika_width, pika_height,size,0);
		}else{
			draw_sprite(Xpos, Ypos, sprite_table+4*(pika_width*pika_height), pika_width, pika_height,size,0);
		}

	}else if(psb_check == 10){
		// down right
		set_block(width, step, Xpos_temp, Ypos-step, erase_color);
		set_block(step, height, Xpos_temp, Ypos, erase_color);
		if (*frame < 12 || (*frame > 25 && *frame < 37)  ){
					draw_sprite(Xpos, Ypos, sprite_table+3*(pika_width*pika_height), pika_width, pika_height,size,1);
				}else{
					draw_sprite(Xpos, Ypos, sprite_table+4*(pika_width*pika_height), pika_width, pika_height,size,1);
				}

	}else if(psb_check == 24){
		//up right
		set_block(width, step, Xpos_temp, Ypos+height, erase_color);
		set_block(step, height, Xpos_temp, Ypos, erase_color);
		if (*frame < 12 || (*frame > 25 && *frame < 37)  ){
					draw_sprite(Xpos, Ypos, sprite_table+3*(pika_width*pika_height), pika_width, pika_height,size,1);
				}else{
					draw_sprite(Xpos, Ypos, sprite_table+4*(pika_width*pika_height), pika_width, pika_height,size,1);
				}
	}else if(psb_check == 16){
		// up
		set_block(width, step, Xpos_temp, Ypos+height, erase_color);
		draw_sprite(Xpos, Ypos, sprite_table+2*(pika_width*pika_height), pika_width, pika_height,size,0);
	}
	else if(psb_check == 2){
		// down
		set_block(width, step, Xpos_temp, Ypos-step, erase_color);
		if (*frame < 12 || (*frame > 25 && *frame < 37)  ){
			draw_sprite(Xpos, Ypos, sprite_table+(pika_width*pika_height), pika_width, pika_height,size,0);
		}else{
			draw_sprite(Xpos, Ypos, sprite_table, pika_width, pika_height,size,0);
		}
	}else if(psb_check == 4){
		//left
		set_block(width-step, height, Xpos_temp+width-step, Ypos, erase_color);
		if (*frame < 12 || (*frame > 25 && *frame < 37)  ){
			draw_sprite(Xpos, Ypos, sprite_table+3*(pika_width*pika_height), pika_width, pika_height,size,0);
		}else{
			draw_sprite(Xpos, Ypos, sprite_table+4*(pika_width*pika_height), pika_width, pika_height,size,0);
		}
	}else if(psb_check == 8){
		// right
		set_block(step, height, Xpos_temp, Ypos, erase_color);
		if (*frame < 12 || (*frame > 25 && *frame < 37)  ){
			draw_sprite(Xpos, Ypos, sprite_table+3*(pika_width*pika_height), pika_width, pika_height,size,1);
		}else{
			draw_sprite(Xpos, Ypos, sprite_table+4*(pika_width*pika_height), pika_width, pika_height,size,1);
		}
	}else if(psb_check == 0){
		if (*frame < 25 ){
			draw_sprite(Xpos, Ypos, sprite_table+(pika_width*pika_height), pika_width, pika_height,size,0);
			//*frame=*frame +1;
		}else{
			draw_sprite(Xpos, Ypos, sprite_table, pika_width, pika_height,size,0);
			//*frame=*frame-1;
		}
	}

}

void set_pix(int x,int y, int c){
    volatile unsigned int ramaddr;
	unsigned int word_number=0,packet_nr=0,ram_value=0,last_bit_pixel=0;
	ramaddr = 0;
	//calculate the number of the word where the pixel is stored on one row
	word_number=x/10; // integer division
	//calculate the address of the word containing pixel
	ramaddr=ramaddr+(word_number+(y*WordsPerLine));
	//assign the value of the sram to a temporary value
	Xil_Out32((BaseAddress + Reg2_Offset), 0x0); //bram_we <=0  because we are reading
	Xil_Out32((BaseAddress + Reg1_Offset), ramaddr); //bram_addr
	Xil_Out32((BaseAddress + Reg2_Offset), 0x0); //bram_we <=0  because we are reading,
	Xil_Out32((BaseAddress + Reg1_Offset), ramaddr); //bram_addr
	ram_value=Xil_In32((BaseAddress + Reg3_Offset)); //bram_dout
	//each word contains 10 pixels
	//calculate which position the pixel has in the word
	packet_nr=x-(word_number*10);
	last_bit_pixel=29-(packet_nr*3);
	// clear the bits that are changing -> "000"
	ram_value &=~(0x7<<last_bit_pixel);
	// set the bits
    ram_value |=(c<<last_bit_pixel);
	//write the new sram value to the sram
	Xil_Out32((BaseAddress + Reg2_Offset), 0x1); //bram_we <=1
	Xil_Out32((BaseAddress + Reg0_Offset), ram_value); //bram_din
}

// STUDENT TO MODIFY THIS ROUTINE:
void set_npix(int x,int y,int nx, int c){
    volatile unsigned int ramaddr;
	unsigned int word_number=0,packet_nr=0,ram_value=0,last_bit_pixel=0;
	unsigned int temp = 0;
	int i;
	ramaddr = 0;
	//calculate the number of the word where the pixel is stored on one row
							// integer division
	word_number=x/10;		//word_number = 0-63
	//calculate the address of the word containing pixel
	ramaddr=ramaddr+(word_number+(y*WordsPerLine));

	//assign the value of the sram to a temporary value
														//read the 32-bit data:
	Xil_Out32((BaseAddress + Reg2_Offset), 0x0);		//bram_we <=0 (first time)
	Xil_Out32((BaseAddress + Reg1_Offset), ramaddr); 	//bram_addr   (first time)
	Xil_Out32((BaseAddress + Reg2_Offset), 0x0);		//bram_we <=0 (2nd Flipflop pipeline)
	Xil_Out32((BaseAddress + Reg1_Offset), ramaddr); 	//bram_addr   (2nd Flipflop pipeline)
	ram_value=Xil_In32((BaseAddress + Reg3_Offset));	//bram_dout
	//each word contains 10 pixels
	//calculate which position the pixel has in the word
	packet_nr=x-(word_number*10);
    for(i=0;i<nx;i++){
	  last_bit_pixel=29-(packet_nr*3);
	  // clear the bits that are changing -> "000"
	  ram_value &=~(0x7<<last_bit_pixel); // copypasta from original
	  // set the bits
	  ram_value |=(c<<last_bit_pixel); // copypasta from original
      if (packet_nr<9) {            //check for the last pixel in the byte, if not last:
        packet_nr++;
	  }else{					//if last pixel, move to next 32-bit word
        packet_nr=0;
   		//write the modified 32-bit data:
        Xil_Out32((BaseAddress + Reg2_Offset), 0x1); 			//bram_we <=1
        Xil_Out32((BaseAddress + Reg0_Offset), ram_value); 		//bram_din
		ramaddr++;		//move to next address with color in it
	  														//read the 32-bit data:
		Xil_Out32((BaseAddress + Reg2_Offset), 0x0);		//bram_we <=0 (first time)
	    Xil_Out32((BaseAddress + Reg1_Offset), ramaddr); 	//bram_addr   (first time)
		Xil_Out32((BaseAddress + Reg2_Offset), 0x0);		//bram_we <=0 (2nd Flipflop pipeline)
		Xil_Out32((BaseAddress + Reg1_Offset), ramaddr); 	//bram_addr   (2nd Flipflop pipeline)
		ram_value=Xil_In32((BaseAddress + Reg3_Offset));	//bram_dout
	  }
    }
	//write the new ram value to the ram
	Xil_Out32((BaseAddress + Reg2_Offset), 0x1); //bram_we <=1
	Xil_Out32((BaseAddress + Reg0_Offset), ram_value); //bram_din
}



void initialize_picture(int color){
      volatile unsigned int ramaddr;
  	int i=0;
  	unsigned int pix10color=0;  // unsigned because of the >>3
      pix10color = color<<29 ;
  	for (i=1;i<10;i++){
        pix10color = pix10color | (pix10color>>3) ;
  	}
      print("\r\nstart writing to RAM\n\r");
 	  	putnum(pix10color); //function to output the color number as ASCII on terminal/com port
  	Xil_Out32((BaseAddress + Reg2_Offset), 0x1) ;  //bram_we <=1
  	for(ramaddr=0;ramaddr<=30719;ramaddr++){

  		Xil_Out32((BaseAddress + Reg0_Offset), pix10color) ;  //bram_din
  		Xil_Out32((BaseAddress + Reg1_Offset), ramaddr) ;  //bram_addr
   	}
  }


void set_block(int width, int height, int Xpos, int Ypos, int color){
  int i,j;
    for(j=Ypos;j<Ypos+height;j++){
    	set_npix(Xpos,j,width, color);
    }
}



