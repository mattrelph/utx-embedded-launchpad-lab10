// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))

#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_PUR_R        (*((volatile unsigned long *)0x40024510))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_LOCK_R       (*((volatile unsigned long *)0x40024520))
#define GPIO_PORTE_CR_R         (*((volatile unsigned long *)0x40024524))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))

#define GPIO_PORTB_DATA_R       (*((volatile unsigned long *)0x400053FC))
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_PUR_R        (*((volatile unsigned long *)0x40005510))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_LOCK_R       (*((volatile unsigned long *)0x40005520))
#define GPIO_PORTB_CR_R         (*((volatile unsigned long *)0x40005524))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))
	
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))

#define NVIC_ST_CTRL_R          (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R        (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R       (*((volatile unsigned long *)0xE000E018))

#define INPUT_WEST_PE0    		  (*((volatile unsigned long *)0x40024004))	//PE0 - West Car Sensor
#define INPUT_SOUTH_PE1     		(*((volatile unsigned long *)0x40024008))	//PE1 - South Car Sensor
#define INPUT_WALK_PE2       	(*((volatile unsigned long *)0x40024010)) //PE2 - Pedestrian Sensor
	
#define OUTPUT_WALK_PF1    		  (*((volatile unsigned long *)0x40025008))	//PF1 - Pedestrian Red LED
#define OUTPUT_WALK_PF3     		(*((volatile unsigned long *)0x40025020))	//PF3 - Pedestrian Green LED

#define OUTPUT_SOUTH_GREEN_PB0    		  (*((volatile unsigned long *)0x40005004))	//PB0 - South Light Green LED
#define OUTPUT_SOUTH_YELLOW_PB1     		(*((volatile unsigned long *)0x40005008))	//PB1 - South Light Yellow LED
#define OUTPUT_SOUTH_RED_PB2       			(*((volatile unsigned long *)0x40005010)) //PB2 - South Light Red LED

#define OUTPUT_WEST_GREEN_PB3    		  (*((volatile unsigned long *)0x40005020))	//PB3 - West Light Green LED
#define OUTPUT_WEST_YELLOW_PB4     		(*((volatile unsigned long *)0x40005040))	//PB4 - West Light Yellow LED
#define OUTPUT_WEST_RED_PB5       		(*((volatile unsigned long *)0x40005080)) //PB5	 - West Light Red LED

#define SENSOR  (*((volatile unsigned long *)0x4002401C)) //PE0-PE2
#define CAR_LIGHT   (*((volatile unsigned long *)0x400050FC)) //PB0-PE5
#define WALK_LIGHT   (*((volatile unsigned long *)0x40025028)) //PF1,PF3


#define goWest 0
#define yieldWest 1
#define goSouth 2
#define yieldSouth 3
#define goWalk 4
#define stopWalk 5
#define flashWalkA 6
#define flashWalkB 7
#define flashWalkC 8




// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

void SysTick_Init(void);		//Init SysTick clock
void SysTick_Wait(unsigned long delay);
void SysTick_Wait10ms(unsigned long delay); //Wait 10 ms * delay

void PortB_Init(void);	//Init Port B
void PortE_Init(void);	//Init Port E
void PortF_Init(void);	//Init Port F


struct State {
  unsigned long carOut;  // 6-bit pattern to output
  unsigned long walkOut;  // 2-bit pattern to output	
  unsigned long Time; // delay in 10ms units 
  unsigned long Next[9];}; // next state for inputs 0,1,2,3

typedef const struct State STyp;
STyp FSM[9]=	{
{0x0C,0x02,500,{goWest,goWest,yieldWest,yieldWest,yieldWest,yieldWest,yieldWest,yieldWest}},
{0x14,0x02,500,{stopWalk,stopWalk,stopWalk,goSouth,stopWalk,goWalk,stopWalk,goSouth}},
{0x21,0x02,500,{goSouth,yieldSouth,goSouth,yieldSouth,yieldSouth,yieldSouth,yieldSouth,yieldSouth}},
{0x22,0x02,500,{stopWalk,stopWalk,stopWalk,goWest,stopWalk,stopWalk,goWalk,goWalk}},
{0x24,0x08,500,{goWalk,flashWalkA,flashWalkA,flashWalkA,goWalk,flashWalkA,flashWalkA,flashWalkA}},
{0x24,0x02,500,{stopWalk,goWest,goSouth,goWest,goWalk,goWest,goSouth,goWest}},
{0x24,0x00,500,{flashWalkB,flashWalkB,flashWalkB,flashWalkB,flashWalkB,flashWalkB,flashWalkB,flashWalkB}},
{0x24,0x02,500,{flashWalkC,flashWalkC,flashWalkC,flashWalkC,flashWalkC,flashWalkC,flashWalkC,flashWalkC}},
{0x24,0x00,500,{stopWalk,stopWalk,stopWalk,stopWalk,stopWalk,stopWalk,stopWalk,stopWalk}}
};
	
unsigned long S;  // index to the current state 
unsigned long Input; 

// ***** 3. Subroutines Section *****

int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz


	SysTick_Init();	//Init Systick Clock
  PortB_Init(); 	//Init Port B (PB0-PB5 = both RYG traffic lights)
	PortE_Init();		//Init Port E (PE0-PE2 = Car / pedestrian detectors)
	PortF_Init();		//Init Port F (PF1, PF2 = Pedestrian lights)
	S = stopWalk;
  EnableInterrupts();
  while(1){
  //Set lights
	CAR_LIGHT = FSM[S].carOut;  
	WALK_LIGHT = FSM[S].walkOut;  	
		
	//Wait
	SysTick_Wait10ms(FSM[S].Time);	
		
	//Check Inputs
	Input = SENSOR; 	
	//Go to appropriate state
	S = FSM[S].Next[Input]; 
		
  }
}


void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x1F;          // 5) PF4-PF0 output
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x00;          // no pull-up on PF4-PF0
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0
}

void PortE_Init(void)
	{ 
		volatile unsigned long delay;
		SYSCTL_RCGC2_R |= 0x00000010;     // 1) E clock 
		delay = SYSCTL_RCGC2_R;           // delay   
		GPIO_PORTE_LOCK_R = 0x4C4F434B;   // 2) unlock PortE PE0  
		GPIO_PORTE_CR_R = 0x07;           // allow changes to PE2-0       
		GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog function
		GPIO_PORTE_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
		GPIO_PORTE_DIR_R = 0x00;          // 5) PE2-PE0 input
		GPIO_PORTE_AFSEL_R = 0x00;        // 6) no alternate function
		GPIO_PORTE_PUR_R = 0x00;          // enable pullup resistors on PE2-PE0       
		GPIO_PORTE_DEN_R = 0x07;          // 7) enable digital pins PE2-PE0       
		
}


void PortB_Init(void)
	{ 
		volatile unsigned long delay;
		SYSCTL_RCGC2_R |= 0x00000002;     // 1) B clock 
		delay = SYSCTL_RCGC2_R;           // delay   
		GPIO_PORTB_LOCK_R = 0x4C4F434B;   // 2) unlock PortB PB0  
		GPIO_PORTB_CR_R = 0x3F;           // allow changes to PB5-0       
		GPIO_PORTB_AMSEL_R = 0x00;        // 3) disable analog function
		GPIO_PORTB_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
		GPIO_PORTB_DIR_R = 0x3F;          // 5) PB5-PB0 are output   
		GPIO_PORTB_AFSEL_R = 0x00;        // 6) no alternate function
		GPIO_PORTB_PUR_R = 0x00;          // enable pullup resistors on PB5-PB0       
		GPIO_PORTB_DEN_R = 0x3F;          // 7) enable digital pins PB5-PB0       
		
}


// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = 0x00FFFFFF;        // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it             
  NVIC_ST_CTRL_R = 0x00000005;          // enable SysTick with core clock
}


// Time delay using busy wait.
// The delay parameter is in units of the core clock. 
void SysTick_Wait(unsigned long delay){
  volatile unsigned long elapsedTime;
  unsigned long startTime = NVIC_ST_CURRENT_R;
  do{
    elapsedTime = (startTime-NVIC_ST_CURRENT_R)&0x00FFFFFF;
  }
  while(elapsedTime <= delay);
}

// Time delay using busy wait.
// This assumes 16 MHz system clock.
void SysTick_Wait10ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    //SysTick_Wait(1600000);  // wait 10ms (assumes 16 MHz clock)
		SysTick_Wait(160000);  // wait 10ms (assumes 16 MHz clock)
  }
}


