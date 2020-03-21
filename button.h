/* File Description: this file provide interfaces of button.
 * Before Using,you should achieve four functions o platform.
 * 	A: btn_malloc() Apply for memory space.
 * 	B: btn_free() 	release memory space.
 * 	C: demo_get_gpio_press() detecting hardware pin levels.
 *  D: demo_button_conf()	config pin.
 * How to use:
 * First: Create button by called button_create.
 * Second: register function handle of button event by called btn_event_register.
 * At last: cyclle called the btn_main_cycle() and btn_event_handl() by order.
 */


#ifndef __BUTTON_H__
#define __BUTTON_H__

#define NULL				(0)
#define ON					1
#define OFF					0
#define MAX_NAME			16									//max leng of button name

#define MAIN_LOOP			10									//cycle detection gpio status time(ms)

#define CLICK_INTERVAL 		500 / (MAIN_LOOP * 2)					//continuous trigger interval time
#define DEBOUNCE_TIME		20	/ (MAIN_LOOP * 2)								//debounce time (ms)

#define LONG_TRIG_TIME 		(3000 / (MAIN_LOOP * 2))			//long press time (3s)
#define SHORT_TRIG_TIME		(100 / (MAIN_LOOP * 2))				//short press time (100ms)	

enum btn_event{
	NO_PRESS = 0,
	PRESS_DOWN,
	PRESS_UP,	
	SIGNLE_CLICK,	
	LONG_PRESS,
	num_of_event,
	ALL_EVENT,
};

struct btn_info{
	char name[MAX_NAME];

	char enable;							//reserve
	
	char port;								//gpio port
	
	char trig_level;						//effective level

	void (*event_handle[num_of_event])();	//handle button event function
	
	int last_status;						//button last stutas
	
	int cur_status;							//button current stutas
	
	int btn_cnt;							//
	
	int btn_cycle_cnt;						//reserve

	int btn_trig_event;
	
	struct btn_info *next;
};

//----------------------------------------------------------------------------------
#define DEBUG					ON		//log switch
#if DEBUG
#define DEBUG_PRINT(format,...) do{ \
		printf(format"\n",##__VA_ARGS__); \
		}while(0);
#else
#define DEBUG_PRINT(format,...)

#endif
//-----------------------------------------function---------------------------------
void btn_main_cycle();

void Initqueue();

void button_create(const char * name,unsigned char port,unsigned char level);

void button_delete_by_name(const char * name);

static void button_add(struct btn_info *btn);

static void demo_button_conf(int port,char level);

void Print_Btn_Info();

void btn_event_handl();

void btn_event_register(const char *name,enum btn_event event,void (*handle)());

#endif
