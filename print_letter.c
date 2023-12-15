#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Include Pico libraries
#include "pico/stdlib.h"
#include "pico/multicore.h"
// Include hardware libraries
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
// Include protothreads
#include "pt_cornell_rp2040_v1.h"

// PWM wrap value and clock divide value
// For a CPU rate of 125 MHz, this gives
// a PWM frequency of 1 kHz.
#define WRAPVAL 10000
#define CLKDIV 250.0f

// Variable to hold PWM slice number
uint slice_num ;

// direction and step pin for two motors
#define direction1 10
#define step1 11
#define direction2 12
#define step2 13

int step_360 =200;
float delta1 = 0;
float delta2 = 0;
int s1 = 0;
int s2 = 0;
uint32_t motorInterval1 = 1000;
uint32_t motorInterval2 = 1000;
int d1 = 0;
int d2 = 0;

int step = 0;

struct Point {
    float x;
    float y;
};
struct Point points[40];

// PWM duty cycle
volatile int control ;
volatile int old_control ;

// Top Direction
void up(int font)
{
    //int n =step;
    gpio_put(direction1, 0);
    gpio_put(direction2, 0);
    for(int i=0;i<=font;i++) {
        gpio_put(step1, 1);
        gpio_put(step2, 1);
        sleep_ms(2);
        gpio_put(step1, 0);
        gpio_put(step2, 0);
        sleep_ms(2);
    }
    sleep_ms(10);
}

void down(int font)
{
    gpio_put(direction1, 1);
    gpio_put(direction2, 1);
    for(int i=0;i<=font;i++) {
        gpio_put(step1, 1);
        gpio_put(step2, 1);
        sleep_ms(2);
        gpio_put(step1, 0);
        gpio_put(step2, 0);
        sleep_ms(2);
    }
    sleep_ms(10);
}


void left(int font)
{
    gpio_put(direction1, 1);
    gpio_put(direction2, 0);
    for(int i=0;i<=font;i++) {
        gpio_put(step1, 1);
        gpio_put(step2, 1);
        sleep_ms(2);
        gpio_put(step1, 0);
        gpio_put(step2, 0);
        sleep_ms(2);
    }
    sleep_ms(10);
 
}


void right(int font)
{
    gpio_put(direction1, 0);
    gpio_put(direction2, 1);
    for(int i=0;i<=font;i++) {
        gpio_put(step1, 1);
        gpio_put(step2, 1);
        sleep_ms(2);
        gpio_put(step1, 0);
        gpio_put(step2, 0);
        sleep_ms(2);
    }
    sleep_ms(10);
    
}

void ne(int font)
{
    gpio_put(direction1, 0);
    for(int i=0;i<=font;i++) {
        gpio_put(step1, 1);
        sleep_ms(2);
        gpio_put(step1, 0);
        sleep_ms(2);
    }
    sleep_ms(10);
}

void sw(int font)
{
    gpio_put(direction1, 1);
    for(int i=0;i<=font;i++) {
        gpio_put(step1, 1);
        sleep_ms(2);
        gpio_put(step1, 0);
        sleep_ms(2);
    }
    sleep_ms(10);
}

void nw(int font)
{
    gpio_put(direction2, 0);
    for(int i=0;i<=font;i++) {
        gpio_put(step2, 1);
        sleep_ms(2);
        gpio_put(step2, 0);
        sleep_ms(2);
    }
    sleep_ms(10);
}

void se(int font)
{
    gpio_put(direction2, 1);
    for(int i=0;i<=font;i++) {
        gpio_put(step2, 1);
        sleep_ms(2);
        gpio_put(step2, 0);
        sleep_ms(2);
    }
    sleep_ms(10);
}


void drawA(int n)
{
    ne(n);
    se(n);
    nw(n/2);
    left(n/3);
}
void drawB(int n)
{
    up(n);
    right(n/2);
    down(n/2);
    left(n/2);
    right(n/2);
    down(n/2);
    left(n/2);
}
void drawC(int n)
{
   up(n);
   right(n/2);
   left(n/2);
   down(n);
   right(n/2);
   right(n/2);
}
void drawD(int n)
{
   up(n);
   right(n/2);
   down(n);
   left(n/2);
}
void drawE(int n)
{
   up(n);
   right(n/2);
   left(n/2);
   down(n/2);
   right(n/2);
   left(n/2);
   down(n/2);
   left(n/2);
   left(n/2);
}
void drawF(int n)
{
    up(n);
    right(n/2);
    left(n/2);
    down(n/2);
    right(n/2);
}
void drawG(int n)
{
    left(n/2);
    down(n);
    right(n/2);
    up(n/2);
    left(n/3);
}
void drawH(int n)
{
    up(n);
    down(n/2);
    right(n/2);
    up(n/2);
    down(n);
    right(n/2);
}
void drawI(int n)
{
    up(n);
}

void drawJ(int n)
{
    right(n/2);
    up(n);
   
}
void drawK(int n)
{
    sw(n/2);
    up(n/2);
    down(n);
    up(n/2);
    right(n/2);
    down(n/2);
}
void drawL(int n)
{
    up(n);
    down(n);
    right(n/2);
}
void drawM(int n)
{
    up(n);
    se(n/2);
    ne(n/2);
    down(n);
}
void drawN(int n)
{
    up(n);
    se(2*n);
    up(n);
    down(n);
    right(n/2);
}
void drawO(int n)
{
    up(n);
    right(n/2);
    down(n);
    left(n/2);
    right(n);
}
void drawP(int n)
{
    up(n);
    right(n/2);
    down(n/2);
    left(n/2);
}
void drawQ(int n)
{
    up(n);
    right(n/2);
    down(n);
    left(n/2);
    right(n/2);
    nw(n/2);
    se(n);
}
void drawR(int n)
{
    up(n);
    right(n/2);
    down(n/2);
    left(n/2);
    se(n);
    right(n/2);
}
void drawS(int n)
{
    right(n/2);
    up(n/2);
    left(n/2);
    up(n/2);
    right(n/2);
}
void drawT(int n)
{
    up(n);
    left(n/4);
    right(n/2);
    left(n/4);
    down(n);
    right(n/2);

}
void drawU(int n)
{
    up(n);
   down(n);
   right(n/2);
   up(n);
   down(n);
   right(n/2);
}
void drawV(int n)
{
    se(n);
    ne(n);
    
}
void drawW(int n)
{
    down(n);
    ne(n/2);
    se(n/2);
    up(n);
}
void drawX(int n)
{
    ne(n);
    sw(n/2);
    se(n/2);
    nw(n);
}
void drawY(int n)
{
    up(n/2);
   nw(n/2);
   se(n/2);
   ne(n/2); 
}
void drawZ(int n)
{
    left(n/2);
    ne(n);
    left(n/2);
}

// User input thread. User can change draw speed
static PT_THREAD (protothread_serial(struct pt *pt))
{
    PT_BEGIN(pt) ;
    static char classifier ;
    static char char_in ;
    static int font_size_in ;
    while(1) {
    
    
    sprintf(pt_serial_out_buffer, "input font size: ");
    serial_write ;
        // spawn a thread to do the non-blocking serial read
        serial_read ;
        // convert input string to number
        sscanf(pt_serial_in_buffer,"%d", &step) ;
        {
        
            sprintf(pt_serial_out_buffer, "input a letter: ");
            serial_write ;
            // spawn a thread to do the non-blocking serial read
            serial_read ;
            // convert input string to number
            sscanf(pt_serial_in_buffer,"%c", &classifier) ;

            // num_independents = test_in ;
            if (classifier=='a' || classifier == 'A') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawA(step);
                }
                if (classifier=='B' || classifier == 'b') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawB(step);
                }
                if (classifier=='C' || classifier == 'c') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawC(step);
                }
                if (classifier=='D' || classifier == 'd') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawD(step);
                }
                if (classifier=='e' || classifier == 'E') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawE(step);
                }
                if (classifier=='F' || classifier == 'f') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawF(step);
                }
                if (classifier=='G' || classifier == 'g') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawG(step);
                }
                if (classifier=='h' || classifier == 'H') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawH(step);
                }
                if (classifier=='I' || classifier == 'i') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawI(step);
                }
                if (classifier=='j' || classifier == 'J') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawJ(step);
                }
                            if (classifier=='k' || classifier == 'K') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawK(step);
                }
                            if (classifier=='l' || classifier == 'L') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawL(step);
                }
                            if (classifier=='m' || classifier == 'M') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawM(step);
                }
                            if (classifier=='n' || classifier == 'N') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawN(step);
                }
                            if (classifier=='o' || classifier == 'O') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawO(step);
                }
                            if (classifier=='p' || classifier == 'P') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawP(step);
                }
                            if (classifier=='q' || classifier == 'Q') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawQ(step);
                }
                            if (classifier=='r' || classifier == 'R') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawR(step);
                }
                            if (classifier=='s' || classifier == 'S') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawS(step);
                }
                            if (classifier=='t' || classifier == 'T') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawT(step);
                }
                            if (classifier=='u' || classifier == 'U') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawU(step);
                }
                            if (classifier=='v' || classifier == 'V') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawV(step);
                }
                            if (classifier=='w' || classifier == 'W') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawW(step);
                }
                            if (classifier=='x' || classifier == 'X') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawX(step);
                }
                            if (classifier=='y' || classifier == 'Y') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawY(step);
                }
                            if (classifier=='z' || classifier == 'Z') {
                // serial_write ;
                // serial_read ;
                // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                //pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                drawZ(step);
                            }
                if (classifier=='0'){
                  // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1000);
                    sleep_ms(1000);
                    right(step);
                    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                }
                    if (classifier=='1'){
                 // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1000);
                    sleep_ms(1000);
                    left(6*step);
                    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);
                }
            
            }
        }
    PT_END(pt) ;
}

// Main (runs on core 0)
int main() {
    // Initialize stdio
    stdio_init_all();

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////// PWM CONFIGURATION ////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    // Tell GPIO 5 that it is allocated to the PWM
    gpio_set_function(5, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 5 (it's slice 2)
    slice_num = pwm_gpio_to_slice_num(5);

    // This section configures the period of the PWM signals
    pwm_set_wrap(slice_num, WRAPVAL) ;
    pwm_set_clkdiv(slice_num, CLKDIV) ;

    // Start the channel
    pwm_set_mask_enabled((1u << slice_num));

    gpio_init(direction1);
    gpio_init(step1);
    gpio_init(direction2);
    gpio_init(step2);
    // Configure the motor pin as an output
    gpio_set_dir(direction1, GPIO_OUT);
    gpio_set_dir(step1, GPIO_OUT);
    gpio_set_dir(direction2, GPIO_OUT);
    gpio_set_dir(step2, GPIO_OUT);

    // start core 0
    pt_add_thread(protothread_serial) ;
    pt_schedule_start ;                                                                                                             
}








