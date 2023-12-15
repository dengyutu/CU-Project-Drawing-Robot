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

int step_360 = 200; //number of steps per rotation
float delta1 = 0;
float delta2 = 0;
int s1 = 0; //total number of steps for motor 1
int s2 = 0; //total number of steps for motor 2
uint32_t motorInterval1 = 1000; //wait time for motor 1
uint32_t motorInterval2 = 1000; //wait time for motor 2
int d1 = 0; //direction for motor 1
int d2 = 0; //direction for motor 2

int step = 0;

struct Point {
    float x;
    float y;
};
struct Point points[40];

// PWM duty cycle
volatile int control ;
volatile int old_control ;

// control second stepper motor
void core1_main(){
    gpio_put(direction2, d2);
    for(int i=0;i<s2;i++) {
        gpio_put(step2, 1);
        sleep_us(motorInterval2);
        gpio_put(step2, 0);
        sleep_us(motorInterval2);
    }
    gpio_put(step2, 0);
}

void draw_xy(float startx, float starty, float endx, float endy){
    float deltax = endx - startx;
    float deltay = endy - starty;
    delta2 = (deltax + deltay) / sqrtf(2);
    delta1 = (deltax - deltay) / sqrtf(2);
    s1 = (int)(fabs(delta1) / 28 * step_360);
    s2 = (int)(fabs(delta2) / 28 * step_360);

    if(s1>=s2){
        if(s2==0){
            motorInterval2 = 1000;
            motorInterval1 = 1000;
        }
        else {
        motorInterval2 = (int)(1000*s1/s2);
        motorInterval1 = 1000;
        }
    }
    else if(s1<s2){
        if(s1==0){
            motorInterval2 = 1000;
            motorInterval1 = 1000;
        }
        else {
        motorInterval1 = (int)(1000*s2/s1);
        motorInterval2 = 1000;
        }
    }
    
    if (delta1 > 0){
        d1 = 1;}
    else{
        d1 = 0;}

    if (delta2 > 0){
        d2 = 1;}
    else{
        d2 = 0;}

    printf("delta1 is %f\n",delta1);
    printf("delta2 is %f\n",delta2);
    printf("step1 is %d\n",s1);
    printf("step2 is %d\n",s2);
    printf("I1 is %d\n",motorInterval1);
    printf("I2 is %d\n",motorInterval2);
    printf("D1 is %d\n",d1);
    printf("D2 is %d\n",d2);

    if(s2>0){
    multicore_reset_core1();
    multicore_launch_core1(&core1_main);
    }
    if(s1>0){
    gpio_put(direction1, d1);
    for(int i=0;i<s1;i++) {
        gpio_put(step1, 1);
        sleep_us(motorInterval1);
        gpio_put(step1, 0);
        sleep_us(motorInterval1);
    }
    gpio_put(step1, 0);
    }

    if(s1==0){
    sleep_us(2*s2*motorInterval2);
    }
    sleep_ms(50);
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
                if (classifier=='0'){
                  // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
                    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1000);
                }
                    if (classifier=='1'){
                 // convert input string to number
                sscanf(pt_serial_in_buffer,"%c", &char_in) ;
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
    gpio_init(MS1);
    gpio_init(MS2);
    gpio_init(MS3);
    // Configure the motor pin as an output
    gpio_set_dir(direction1, GPIO_OUT);
    gpio_set_dir(step1, GPIO_OUT);
    gpio_set_dir(direction2, GPIO_OUT);
    gpio_set_dir(step2, GPIO_OUT);

    for(int i=0;i<41;i++){
        points[i].x = 30*cos(2*M_PI*i/40);
        points[i].y = -30*sin((2*M_PI*i/40));
    }

    pwm_set_chan_level(slice_num, PWM_CHAN_B, 900);
    sleep_ms(1000);

    for(int i=1;i<41;i++){
        printf("The %dth newX is %f\n",i, points[i].x);
        printf("The %dth newY is %f\n",i, points[i].y);
        draw_xy(points[i-1].x,points[i-1].y,points[i].x,points[i].y);
    }

    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1300);

        // start core 0
    pt_add_thread(protothread_serial) ;
    pt_schedule_start ;
                                                                                                                                          
}








