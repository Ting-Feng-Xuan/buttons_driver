/* FileName: button.c
 * Description: this file is something about buttons function 
 * Author: TingFengXuan
 * Time: 2020/3/18
 */
#include "stdlib.h"
#include "button.h"

struct btn_info *head = NULL;

struct btn_info *btn_queue = NULL;

void enqueue(struct btn_info *btn)
{
	struct btn_info *q = (struct btn_info *)btn_malloc(sizeof(struct btn_info));
	struct btn_info *p = NULL;
	if(q == NULL)
	{
		printf("enqueue failed!\n");
		return;
	}
	memcpy(q,btn,sizeof(struct btn_info));
	
	btn->btn_trig_event = NO_PRESS;		//clear button event mark.
	
	if(NULL == btn_queue)
	{
		btn_queue = q;
		btn_queue->next = NULL;
	}
	else
	{
		p = btn_queue;
		while(p->next)
		{
			p = p->next;
		}
		p->next = q;
		q->next = NULL;
	}

	
}
struct btn_info * dequeue()
{
	struct btn_info *btn = NULL;
	if(btn_queue)
	{
		btn = btn_queue;
		btn_queue = btn->next;
		btn->next = NULL;
		return btn;
	}
	return NULL;
}
void *btn_malloc(unsigned int size)
{
	return tls_mem_alloc(size); 
}
void btn_free(void *mem)
{
	if(mem)
	{
		tls_mem_free(mem);
	}
}
void button_create(const char * name,unsigned char port,unsigned char level)
{
	if(!name || strlen(name) > MAX_NAME)
	{
		printf("button name is invalid!\n");
		return;
	}
	struct btn_info *p = head;
	while(p)
	{
		if(strcmp(p->name,name) == 0)
		{
			printf("button: %s has bean existed!\n",p->name);
			return;
		}
		p = p->next;
	}
	struct btn_info *btn = (struct btn_info *)btn_malloc(sizeof(struct btn_info));
	if(btn == NULL)
	{
		printf("button Create failed memory not enough!\n");
		return;
	}
	memset(btn,0,sizeof(struct btn_info));
	
	strcpy(btn->name,name);
	btn->port = port;
	btn->trig_level = level;
	
	button_add(btn);
	demo_button_conf(port,level)；
	return;
}
static bool demo_get_gpio_press(int port,char level)
{
	return(tls_gpio_read(port) == level);
}

static void demo_button_conf(int port,char level)
{
	if(level)
	{
		tls_gpio_cfg(port, WM_GPIO_DIR_INPUT, WM_GPIO_ATTR_FLOATING);
	}
	else 
	{
		tls_gpio_cfg(port, WM_GPIO_DIR_INPUT, WM_GPIO_ATTR_PULLHIGH);
	}
	
}
static void button_add(struct btn_info *btn)
{
	struct btn_info *pbtn = head->next;
	
	if(NULL == head)
	{
		head = btn;
	}
	else
	{
		btn->next = head;
		head = btn;
	}
}

void button_delete_by_name(const char * name)
{
	struct btn_info *btn = head;
	if(btn == NULL)
	{
		printf("There are no buttons!\n");
		return;
	}
	if(strcmp(btn->name,name) == 0)//if it is head node.
	{
		head = head->next;
		btn_free(btn);
		DEBUG_PRINT("%s delete success!",name);
		return;
	}
	while(btn->next)
	{
		
		if(strcmp(btn->next->name,name) == 0)
			break;
		btn = btn->next;
	}
	if(btn->next)
	{
		struct btn_info *p = btn->next;
		btn->next = p->next;
		p->next = NULL;
		btn_free(p);
		DEBUG_PRINT("%s delete success!",name);
	}
	else 
	{
		DEBUG_PRINT("%s not found!",name);
	}
}

void Print_Btn_Info()
{
	struct btn_info *btn = head;
	if(btn == NULL)
	{
		printf("there are not button!\n");
		return;
	}
	printf("--------Button Info:\n");
	while(btn)
	{
		printf("btn->name: %s\n"
				"btn->port: %d\n"
				"btn->level: %d\n"
				"btn->enable: %d\n",
				btn->name,
				btn->port,
				btn->trig_level,
				btn->enable

			);
		printf("----------------------\n");

		btn = btn->next;
	}
}

static void check_btn_status(struct btn_info *btn)
{
	if(NULL == btn)
	{
		DEBUG_PRINT("%s():btn is NULL!\n",__FUNCTION__);
		return;
	}

	if(demo_get_gpio_press(btn->port,btn->trig_level))
	{		
		if((++btn->btn_cnt)  >= DEBOUNCE_TIME)
		{
			btn->cur_status	= PRESS_DOWN;
		}
	}
	else
	{
		if((++btn->btn_cnt)  >= DEBOUNCE_TIME)
		{
			btn->cur_status	= PRESS_UP;
		}
	}
	switch(btn->last_status)
	{
		case NO_PRESS:

			if(btn->cur_status == PRESS_DOWN)		
			{
				btn->last_status = PRESS_DOWN;
			}
		break;
		case PRESS_UP:

			if(btn->cur_status == PRESS_DOWN)
			{
				btn->last_status = PRESS_DOWN;
		
			}
			else
			{
				btn->last_status = NO_PRESS;
			}
		break;
		case PRESS_DOWN:
			
			if(btn->cur_status == PRESS_UP)
			{
				if(btn->btn_cnt < LONG_TRIG_TIME)
				{
					btn->btn_trig_event = SIGNLE_CLICK;
					btn->btn_cnt = 0;
				}
				else
				{
					btn->btn_trig_event = LONG_PRESS;
					btn->btn_cnt = 0;
				}
				
			}
			else if(btn->cur_status == PRESS_DOWN)
			{
				btn->last_status = PRESS_DOWN;
				if(btn->btn_cnt >= LONG_TRIG_TIME)
				{
					btn->btn_trig_event = LONG_PRESS;
					btn->btn_cnt = 0;
				
				}
			}
			btn->last_status = btn->cur_status;
		break;

	}
	
}

void btn_event_handl()
{

	struct btn_info *btn = NULL;
	while(btn = dequeue(&btn_queue))
	{
		if(btn->event_handle[btn->btn_trig_event] != NULL)
		{
			
			btn->event_handle[btn->btn_trig_event]();
			btn->btn_trig_event = NO_PRESS;
			
			btn_free(btn);
			btn = NULL;
		}
	}
}

void btn_main_cycle()
{
	struct btn_info *btn = head;
	struct btn_info *q = NULL;
	if(NULL != btn)
	{
		while(btn)
		{
			check_btn_status(btn);
			
			if(btn->btn_trig_event != NO_PRESS && btn->event_handle[btn->btn_trig_event] != NULL)
			{
				enqueue(btn);
			}
			btn = btn->next;
		}
	}
	else
	{
		DEBUG_PRINT("%s():there no button!\n",__FUNCTION__);
		return;
	}
}

void btn_event_register(const char *name,enum btn_event event,void (*handle)())
{
	if(!handle || event < NO_PRESS || event >= num_of_event)
	{
		printf("argument is  invalid! \n"
				"name: %s handle %p event: %d\n",name,handle,event);
		return;
	}
	struct btn_info *btn = head;
	if(NULL == btn)
	{
		printf("there are no any button!\n");
		return;
	}
	/*if name is not given,it will set all buttons */
	if(name == NULL)
	{
		while(btn)
		{
			if(event == ALL_EVENT)
			{
				for(int i = 0; i < num_of_event; i++)
					btn->event_handle[i] = handle;
			}
			else
			{
				btn->event_handle[event] = handle;
			}
			btn = btn->next;
		}
	}
	else
	{
		while(btn)
		{
			if(strcmp(name,btn->name) == 0)
			{
				break;
			}
			btn = btn->next;
		}
		if(!btn)
		{
			printf("button: %s not found!\n",name);
			return;
		}
		/*if event i ALL_EVENT,it will set all event handle*/
		if(event == ALL_EVENT)
		{
			for(int i = 0; i < num_of_event; i++)
				btn->event_handle[i] = handle;
		}
		else
		{
			btn->event_handle[event] = handle;
		}
	}
}

