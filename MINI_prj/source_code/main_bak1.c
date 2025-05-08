#if 0

#include "device_driver.h"
/* 디버깅용 전처리기 */
#define DEBUG			(1)
#define TEST			(1)

#define LCDW			(320)
#define LCDH			(240)
#define X_MIN	 		(0)
#define X_MAX	 		(LCDW - 1)
#define Y_MIN	 		(0)
#define Y_MAX	 		(LCDH - 1)

#define TIMER_PERIOD	(10)
#define RIGHT			(1)
#define LEFT			(-1)
#define HOME			(0)
#define SCHOOL			(1)
#define UP			    (-1)
#define DOWN			(1)

#define POLICE_STEP		(10)
#define POLICE_SIZE_X	(20)
#define POLICE_SIZE_Y	(20)
#define PLAYER_STEP		(10)
#define PLAYER_SIZE_X	(10)
#define PLAYER_SIZE_Y	(10)
#define MINERAL_SIZE_X	(20)
#define MINERAL_SIZE_Y	(20)

#define BACK_COLOR		(5)
#define POLICE_COLOR	(0)
#define PLAYER_COLOR	(4)
#define GOLD_COLOR		(1)
#define EMERALD_COLOR	(2)
#define DIAMOND_COLOR	(3)

#define MAX_HP			(3)
#define MIN_HP			(0)

#define GAME_OVER		(1)
#define TOUCH_GOLD		(2)
#define TOUCH_EMERALD	(4)
#define TOUCH_DIA		(8)

typedef struct
{
	int x,y;
	int w,h;
	int ci;
	int dir_x, dir_y;
	int hp;
}HUMAN;

typedef struct
{
	int x,y;
	int w,h;
	int ci;
}MINERAL;

static HUMAN player;
static HUMAN police;

static MINERAL dia;
static MINERAL emerald;
static MINERAL gold;

// 함수 선언
static void Draw_Object_Restore(MINERAL * obj);

static int score;
static int stage;
static unsigned short color[] = {RED, YELLOW, GREEN, BLUE, WHITE, BLACK};

#if 0
static int Check_Collision(void)
{
	// 사람 충돌 여부
	int col = 0;

	if((police.x >= player.x) && ((player.x + PLAYER_STEP) >= police.x)) col |= 1<<0;
	else if((police.x < player.x) && ((police.x + POLICE_STEP) >= player.x)) col |= 1<<0;
	
	if((police.y >= player.y) && ((player.y + PLAYER_STEP) >= police.y)) col |= 1<<1;
	else if((police.y < player.y) && ((police.y + POLICE_STEP) >= player.y)) col |= 1<<1;

	if(col == 3)
	{
		Uart_Printf("SCORE = %d\n", score);	
		return GAME_OVER;
	}

	// 금과 플레이어 접촉 여부
	int col_g = 0;

	if((gold.x >= player.x) && ((player.x + PLAYER_STEP) >= gold.x)) col_g |= 1<<0;
	else if((gold.x + MINERAL_SIZE_X > player.x) && (gold.x <= (player.x + PLAYER_STEP))) col_g |= 1<<0;
	
	if((gold.y >= player.y) && ((player.y + PLAYER_STEP) >= gold.y)) col_g |= 1<<1;
	else if((gold.y + MINERAL_SIZE_Y > player.y) && (gold.y <= (player.y + PLAYER_STEP))) col_g |= 1<<1;

	if(col_g == 3)
	{
		Draw_Object_Restore(&gold);
		return TOUCH_GOLD;
	}

	// 에메랄드과 플레이어 접촉 여부
	int col_e = 0;

	if((emerald.x >= player.x) && ((player.x + PLAYER_STEP) >= emerald.x)) col_e |= 1<<0;
	else if((emerald.x + MINERAL_SIZE_X > player.x) && (emerald.x <= (player.x + PLAYER_STEP))) col_e |= 1<<0;
	
	if((emerald.y >= player.y) && ((player.y + PLAYER_STEP) >= emerald.y)) col_e |= 1<<1;
	else if((emerald.y + MINERAL_SIZE_Y > player.y) && (emerald.y <= (player.y + PLAYER_STEP))) col_e |= 1<<1;

	if(col_e == 3)
	{	
		Draw_Object_Restore(&emerald);
		return TOUCH_EMERALD;
	}

	// 다이아몬드와 플레이어 접촉 여부
	int col_d = 0;

	if((dia.x >= player.x) && ((player.x + PLAYER_STEP) >= dia.x)) col_d |= 1<<0;
	else if((dia.x + MINERAL_SIZE_X > player.x) && (dia.x <= (player.x + PLAYER_STEP))) col_d |= 1<<0;
	
	if((dia.y >= player.y) && ((player.y + PLAYER_STEP) >= dia.y)) col_d |= 1<<1;
	else if((dia.y + MINERAL_SIZE_Y > player.y) && (dia.y <= (player.y + PLAYER_STEP))) col_d |= 1<<1;

	if(col_d == 3)
	{	
		Draw_Object_Restore(&dia);
		return TOUCH_DIA;
	}

	return 0;
}
#endif

#if TEST
/* 각 픽셀 간 충돌여부 판단 */

static int Check_Police_Collision(void)
{
	int col = 0;

	if ((police.x >= player.x) && ((player.x + PLAYER_STEP) >= police.x)) col |= 1 << 0;
	else if ((police.x < player.x) && ((police.x + POLICE_STEP) >= player.x)) col |= 1 << 0;

	if ((police.y >= player.y) && ((player.y + PLAYER_STEP) >= police.y)) col |= 1 << 1;
	else if ((police.y < player.y) && ((police.y + POLICE_STEP) >= player.y)) col |= 1 << 1;

	if (col == 3)
	{
		Uart_Printf("SCORE = %d\n", score);
		return GAME_OVER;
	}
	return 0;
}

static int Check_Mineral_Collision(MINERAL *m, int mineral_type)
{
	int col = 0;

	if ((m->x >= player.x) && ((player.x + PLAYER_STEP) >= m->x)) col |= 1 << 0;
	else if ((m->x + m->w > player.x) && (m->x <= (player.x + PLAYER_STEP))) col |= 1 << 0;

	if ((m->y >= player.y) && ((player.y + PLAYER_STEP) >= m->y)) col |= 1 << 1;
	else if ((m->y + m->h > player.y) && (m->y <= (player.y + PLAYER_STEP))) col |= 1 << 1;

	if (col == 3)
	{
		Draw_Object_Restore(m);
		return mineral_type;
	}

	return 0;
}

static int Check_Collision(void)
{
	int result;

	result = Check_Police_Collision();
	if (result) return result;

	result = Check_Mineral_Collision(&gold, TOUCH_GOLD);
	if (result) return result;

	result = Check_Mineral_Collision(&emerald, TOUCH_EMERALD);
	if (result) return result;

	result = Check_Mineral_Collision(&dia, TOUCH_DIA);
	if (result) return result;

	return 0;
}
#endif


static int Police_Move(void)
{
	police.x += POLICE_STEP * police.dir_x;
    police.y += POLICE_STEP * police.dir_y;
	if((police.x + police.w >= X_MAX) || (police.x <= X_MIN)) police.dir_x = -police.dir_x;
    if((police.y + police.h >= Y_MAX) || (police.y <= Y_MIN)) police.dir_y = -police.dir_y;
	return Check_Collision();
}


static void k0(void)
{
	if(player.y > Y_MIN) player.y -= PLAYER_STEP;
}

static void k1(void)
{
	if(player.y + player.h < Y_MAX) player.y += PLAYER_STEP;
}

static void k2(void)
{
	if(player.x > X_MIN) player.x -= PLAYER_STEP;
}

static void k3(void)
{
	if(player.x + player.w < X_MAX) player.x += PLAYER_STEP;
}

static int Player_Move(int k)
{
	// UP(0), DOWN(1), LEFT(2), RIGHT(3)
	static void (*key_func[])(void) = {k0, k1, k2, k3};
	if(k <= 3) key_func[k]();
	return Check_Collision();
}

static void Game_Init(void)
{
	score = 0;
	stage = 1;
	Lcd_Clr_Screen();
	player.x = 150; player.y = 220; player.w = PLAYER_SIZE_X; player.h = PLAYER_SIZE_Y; player.ci = PLAYER_COLOR; player.dir_x = RIGHT; player.dir_y = UP; player.hp = MAX_HP;
	police.x = 0; police.y = 110; police.w = POLICE_SIZE_X; police.h = POLICE_SIZE_Y; police.ci = POLICE_COLOR; police.dir_x = RIGHT; police.dir_y = UP; police.hp = MAX_HP;
	Lcd_Draw_Box(player.x, player.y, player.w, player.h, color[player.ci]);
	Lcd_Draw_Box(police.x, police.y, police.w, police.h, color[police.ci]);
}

static void Draw_Object_H(HUMAN * obj)
{
	Lcd_Draw_Box(obj->x, obj->y, obj->w, obj->h, color[obj->ci]);
}

static void Draw_Object_M(MINERAL * obj)
{
	Lcd_Draw_Box(obj->x, obj->y, obj->w, obj->h, color[obj->ci]);
}

static void Draw_Object_Restore(MINERAL * obj)
{
	Lcd_Draw_Box(obj->x, obj->y, obj->w, obj->h, color[obj->ci]);
}

static void Destroy_MINERAL(MINERAL * obj)
{
	obj->ci = 5;
	Draw_Object_M(obj);
	obj->x = 0;
	obj->y = 0;
	obj->w = 0;
	obj->h = 0;
}

// 광물 랜덤 생성
void Generate_Mineral(MINERAL *m, int color_index, int seed)
{

	srand(seed);

    m->w = MINERAL_SIZE_X;
    m->h = MINERAL_SIZE_Y;
    m->ci = color_index;

    m->x = rand() % (LCDW - m->w);
    m->y = rand() % (LCDH - m->h);
}

// 광물을 생성하고 그려줌
void Generate_And_Draw_Mineral(MINERAL *m, int color)
{
    int seed;

    Adc_Cds_Init();
    Adc_Start();
    while (!Adc_Get_Status());
    seed = Adc_Get_Data();

    Generate_Mineral(m, color, seed);
    Draw_Object_M(m);
}

extern volatile int TIM2_expired;
extern volatile int cnt_time;
extern volatile int TIM4_expired;
extern volatile int USART1_rx_ready;
extern volatile int USART1_rx_data;
extern volatile int Jog_key_in;
extern volatile int Jog_key;

void System_Init(void)
{
	Clock_Init();
	LED_Init();
	Key_Poll_Init();
	Uart1_Init(115200);

	SCB->VTOR = 0x08003000;
	SCB->SHCSR = 7<<16;
}

#define DIPLAY_MODE		3

void Main(void)
{
	// 내가 만든 플래그
	int stage = 1;
	int next_stage = 0;

	System_Init();
	Uart_Printf("Street Froggy\n");

	Lcd_Init(DIPLAY_MODE);


	Jog_Poll_Init();
	Jog_ISR_Enable(1);
	Uart1_RX_Interrupt_Enable(1);


	for(;;)
	{
		Game_Init();
		TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD*10);
		Lcd_Printf(0,0,BLUE,WHITE,2,2,"%d", score);
#if 1
		//광물 생성
		Generate_And_Draw_Mineral(&emerald, EMERALD_COLOR);
 	   	Generate_And_Draw_Mineral(&gold, GOLD_COLOR);
   	 	Generate_And_Draw_Mineral(&dia, DIAMOND_COLOR);
#endif


		int touching_gold = 0;
		int touching_emerald = 0;
		int touching_diamond = 0;
		

		for(;;)
		{
			int game_over = 0;
			int check_state = 0;

			if(Jog_key_in) 
			{
				player.ci = BACK_COLOR;
				Draw_Object_H(&player);
				check_state = Player_Move(Jog_key);

				//현재 접촉 상태
				int now_touching_gold = 0;
        		int now_touching_emerald = 0;
        		int now_touching_diamond = 0;

				switch (check_state)
				{
					case GAME_OVER:
						game_over = 1; 
						break;
					case TOUCH_GOLD:
						now_touching_gold = 1;
						if (!touching_gold) {
							#if DEBUG
							TIM2_Repeat_Interrupt_Enable_time(1, 1000);
							#endif
							Uart1_Printf("MINING GOLD\n");
						}
						break;
					case TOUCH_EMERALD:
						now_touching_emerald = 1;
						if (!touching_emerald) {
							#if DEBUG
							TIM2_Repeat_Interrupt_Enable_time(1, 1000);
							#endif
							Uart1_Printf("MINING EMERALD\n");
						}
						break;
					case TOUCH_DIA:
						now_touching_diamond = 1;
						if (!touching_diamond) {
							#if DEBUG
							TIM2_Repeat_Interrupt_Enable_time(1, 1000);
							#endif
							Uart1_Printf("MINING DIAMOND\n");
						}
						break;
					default:
						break;
				}

				// 이번 프레임 접촉 상태 저장
				touching_gold = now_touching_gold;
				touching_emerald = now_touching_emerald;
				touching_diamond = now_touching_diamond;
				
				player.ci = PLAYER_COLOR;
				Draw_Object_H(&player);
				Jog_key_in = 0;
			}
			#if DEBUG
			if(TIM2_expired)
			{
				switch (cnt_time)
				{
					case 2: 
					{
						if(touching_gold)
						{
							Destroy_MINERAL(&gold); 
							score += 1;
							Uart1_Printf("Mine Gold! +1point\n");
							Uart1_Printf("Score: %d\n", score);
							TIM2_Stop();
						}
						break;
					}

					case 4:
					{
						if(touching_emerald)
						{
							Destroy_MINERAL(&emerald); 
							score += 2;
							Uart1_Printf("Mine Emerald! +2point\n");
							Uart1_Printf("Score: %d\n", score);
							TIM2_Stop();
						}
						break;
					}

					case 6:
					{
						if(touching_diamond)
						{
							Destroy_MINERAL(&dia); 
							score += 3;
							Uart1_Printf("Mine Diamond! +3point\n");
							Uart1_Printf("Score: %d\n", score);
							TIM2_Stop();
						}
						break;
					}
					default:
						break;
				}
				TIM2_expired = 0;
			}
			#endif
			if(TIM4_expired) 
			{
				police.ci = BACK_COLOR;
				Draw_Object_H(&police);
				check_state = Police_Move();
				if(check_state == GAME_OVER) game_over = 1;
				police.ci = POLICE_COLOR;
				Draw_Object_H(&police);
				#if 1
				Draw_Object_Restore(&dia);
				Draw_Object_Restore(&gold);
				Draw_Object_Restore(&emerald);
				#endif
				TIM4_expired = 0;
			}

			if(game_over)
			{
				TIM4_Repeat_Interrupt_Enable(0, 0);
				Uart_Printf("Game Over, Please press any key to continue.\n");
				Jog_Wait_Key_Pressed();
				Jog_Wait_Key_Released();
				Uart_Printf("Game Start\n");
				score = 0;
				stage = 1;
				next_stage = 0;
				cnt_time = 0;
				break;
			}

			#if DEBUG
			if(stage)
			{
				if((stage == 1) && (score == 6))
				{
					next_stage = 1;
				}
			}
			#endif

			#if DEBUG
			if(next_stage)
			{
				
				TIM4_Repeat_Interrupt_Enable(0, 0);
				Uart_Printf("Congratulations! total Score = %d\n", score);
				Uart_Printf("Stage %d Clear\n", stage);
				Jog_Wait_Key_Pressed();
				Jog_Wait_Key_Released();
				Uart_Printf("Game Start\n");
				score = 0;
				stage ++;
				next_stage = 0;
				cnt_time = 0;
				break;
			}
			#endif
		}
	}
}



#endif