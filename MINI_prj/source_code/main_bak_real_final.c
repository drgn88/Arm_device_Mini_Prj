#if 0
#if 1
// 진짜 찐찐 최종본


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

#define POLICE_STEP			(10)
#define POLICE_SIZE_X_S1	(20)
#define POLICE_SIZE_Y_S1	(20)
#define POLICE_SIZE_X_S2	(15)
#define POLICE_SIZE_Y_S2	(15)
#define POLICE_SIZE_X_S3	(17)
#define POLICE_SIZE_Y_S3	(17)
#define PLAYER_STEP			(10)
#define PLAYER_SIZE_X		(10)
#define PLAYER_SIZE_Y		(10)
#define MINERAL_SIZE_X		(20)
#define MINERAL_SIZE_Y		(20)
#define OBS_SIZE_X			(20)
#define OBS_SIZE_Y			(50)
#define OBS_STEP			(20)


#define BACK_COLOR		(5)
#define POLICE_COLOR	(0)
#define PLAYER_COLOR	(4)
#define OBS_COLOR		(0)
#define GOLD_COLOR		(1)
#define EMERALD_COLOR	(2)
#define DIAMOND_COLOR	(3)

#define MAX_HP			(3)
#define MIN_HP			(0)

#define GAME_OVER		(1)
#define TOUCH_GOLD		(2)
#define TOUCH_EMERALD	(4)
#define TOUCH_DIA		(8)

//음악 관련
#define BASE  (500) //msec

typedef struct
{
	int x,y;
	int w,h;
	int ci;
	int dir_x, dir_y;
}HUMAN;

typedef struct
{
	int x,y;
	int w,h;
	int ci;
	int dir_x, dir_y;
}OBSTACLE;

typedef struct
{
	int x,y;
	int w,h;
	int ci;
}MINERAL;

static HUMAN player;
static HUMAN police1;
static HUMAN police2;
static HUMAN police3;
static OBSTACLE fall;
static OBSTACLE fall2;

static MINERAL dia;
static MINERAL emerald;
static MINERAL gold;

// 함수 선언
static void Draw_Object_Restore(MINERAL * obj);
static void Draw_Object_O(OBSTACLE * obj);
static void Destroy_OBS(OBSTACLE * obj);

static int score;
static int stage;
static int player_hp;
static int fall_timer;        // TIM4 프레임 누적 카운터
static int fall_target_time;  // 다음 종유석 등장까지 기다릴 시간 (ms 단위 or 틱 단위)
static int fall_active;       // 현재 종유석이 화면에 존재 중인지
static unsigned short color[] = {RED, YELLOW, GREEN, BLUE, WHITE, BLACK};

//외부 변수 선언
extern volatile int TIM2_expired;
extern volatile int cnt_time;
extern volatile int TIM4_expired;
extern volatile int USART1_rx_ready;
extern volatile int USART1_rx_data;
extern volatile int Jog_key_in;
extern volatile int Jog_key;
extern volatile int SYSTICK_expired;
extern volatile int tone_duration_ms;

#if TEST
/* 각 픽셀 간 충돌여부 판단 */

static int Check_Police_Collision(int stage)
{
	static int check_collision = 0;
	int col = 0;

	int step1 = 0;
	int step2 = 0;
	int step3 = 0;
	switch (stage)
	{
		case 1: step1 = POLICE_STEP; step2 = 0; step3 = 0; break;
		case 2: step1 = POLICE_STEP + 5; step2 = POLICE_STEP + 5; step3 = 0; break;
		case 3: step1 = POLICE_STEP + 10; step2 = POLICE_STEP + 10; step3 = 10; break;;
		default: step1 = 0; step2= 0; step3 = 0; break;
	}

	//police 1 Check
	if ((police1.x >= player.x) && ((player.x + PLAYER_STEP) >= police1.x)) col |= 1 << 0;
	else if ((police1.x < player.x) && ((police1.x + step1) >= player.x)) col |= 1 << 0;

	if ((police1.y >= player.y) && ((player.y + PLAYER_STEP) >= police1.y)) col |= 1 << 1;
	else if ((police1.y < player.y) && ((police1.y + step1) >= player.y)) col |= 1 << 1;

	//police 2 Check
	if ((police2.x >= player.x) && ((player.x + PLAYER_STEP) >= police2.x)) col |= 1 << 0;
	else if ((police2.x < player.x) && ((police2.x + step2) >= player.x)) col |= 1 << 0;

	if ((police2.y >= player.y) && ((player.y + PLAYER_STEP) >= police2.y)) col |= 1 << 1;
	else if ((police2.y < player.y) && ((police2.y + step2) >= player.y)) col |= 1 << 1;

	//police 3 Check
	if ((police3.x >= player.x) && ((player.x + PLAYER_STEP) >= police3.x)) col |= 1 << 0;
	else if ((police3.x < player.x) && ((police3.x + step3) >= player.x)) col |= 1 << 0;

	if ((police3.y >= player.y) && ((player.y + PLAYER_STEP) >= police3.y)) col |= 1 << 1;
	else if ((police3.y < player.y) && ((police3.y + step3) >= player.y)) col |= 1 << 1;

	if (col == 3)
	{
		if(!check_collision)
		{
			Uart_Printf("Crash!!\n", score);
			player_hp--;

			if(!player_hp)
			{
				return GAME_OVER;
			}

			check_collision = 1;
		}
	}
	else
	{
		check_collision = 0;
	}
	return 0;
}

static int Check_OBS_Collision(int stage)
{
	static int check_obs_collision = 0;
	int col = 0;

	if ((fall.x >= player.x) && ((player.x + PLAYER_STEP) >= fall.x)) col |= 1 << 0;
	else if ((fall.x < player.x) && ((fall.x + OBS_STEP) >= player.x)) col |= 1 << 0;

	if ((fall.y >= player.y) && ((player.y + PLAYER_STEP) >= fall.y)) col |= 1 << 1;
	else if ((fall.y < player.y) && ((fall.y + OBS_STEP) >= player.y)) col |= 1 << 1;

	if ((fall2.x >= player.x) && ((player.x + PLAYER_STEP) >= fall2.x)) col |= 1 << 0;
	else if ((fall2.x < player.x) && ((fall2.x + OBS_STEP) >= player.x)) col |= 1 << 0;

	if ((fall2.y >= player.y) && ((player.y + PLAYER_STEP) >= fall2.y)) col |= 1 << 1;
	else if ((fall2.y < player.y) && ((fall2.y + OBS_STEP) >= player.y)) col |= 1 << 1;

	if (col == 3)
	{
		if(!check_obs_collision)
		{
			Uart_Printf("Crash!!\n", score);
			player_hp--;

			if(!player_hp)
			{
				return GAME_OVER;
			}

			check_obs_collision = 1;
		}
	}
	else
	{
		check_obs_collision = 0;
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

static int Check_Collision(int stage)
{
	int result;

	result = Check_Police_Collision(stage);
	if (result) return result;

	result = Check_OBS_Collision(stage);
	if(result) return result;

	result = Check_Mineral_Collision(&gold, TOUCH_GOLD);
	if (result) return result;

	result = Check_Mineral_Collision(&emerald, TOUCH_EMERALD);
	if (result) return result;

	result = Check_Mineral_Collision(&dia, TOUCH_DIA);
	if (result) return result;

	return 0;
}
#endif

#if 0
static int Police_Move(void)
{
	police1.x += POLICE_STEP * police1.dir_x;
    police1.y += POLICE_STEP * police1.dir_y;
	if((police1.x + police1.w >= X_MAX) || (police1.x <= X_MIN)) police1.dir_x = -police1.dir_x;
    if((police1.y + police1.h >= Y_MAX) || (police1.y <= Y_MIN)) police1.dir_y = -police1.dir_y;
	return Check_Collision();
}
#endif

#if 1
static int Police_Move(int stage)
{
	int step1 = 0;
	int step2 = 0;
	int step3 = 0;
	switch (stage)
	{
		case 1: step1 = POLICE_STEP; step2 = 0; step3 = 0; break;
		case 2: step1 = POLICE_STEP + 5; step2 = POLICE_STEP + 5; step3 = 0; break;
		case 3: step1 = POLICE_STEP + 10; step2 = POLICE_STEP + 10; step3 = 10; break;;
		default: step1 = 0; step2= 0; step3 = 0; break;
	}

	police1.x += step1 * police1.dir_x;
    police1.y += step1 * police1.dir_y;
	if((police1.x + police1.w >= X_MAX) || (police1.x <= X_MIN)) police1.dir_x = -police1.dir_x;
    if((police1.y + police1.h >= Y_MAX) || (police1.y <= Y_MIN)) police1.dir_y = -police1.dir_y;

	police2.x += step2 * police2.dir_x;
    police2.y += step2 * police2.dir_y;
	if((police2.x + police2.w >= X_MAX) || (police2.x <= X_MIN)) police2.dir_x = -police2.dir_x;
    if((police2.y + police2.h >= Y_MAX) || (police2.y <= Y_MIN)) police2.dir_y = -police2.dir_y;

	police3.x += step3 * police3.dir_x;
    police3.y += step3 * police3.dir_y;
	if((police3.x + police3.w >= X_MAX) || (police3.x <= X_MIN)) police3.dir_x = -police3.dir_x;
    if((police3.y + police3.h >= Y_MAX) || (police3.y <= Y_MIN)) police3.dir_y = -police3.dir_y;
	return Check_Collision(stage);
}
#endif

#if 1
static int OBS_Move(int stage)
{
    fall.y += OBS_STEP * fall.dir_y;
    if(fall.y + fall.h >= Y_MAX)
	{
		Destroy_OBS(&fall);
	}

	fall2.y += OBS_STEP * fall2.dir_y;
    if(fall2.y + fall2.h >= Y_MAX)
	{
		Destroy_OBS(&fall2);
	}
	return Check_Collision(stage);
}
#endif

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

static int Player_Move(int k, int stage)
{
	// UP(0), DOWN(1), LEFT(2), RIGHT(3)
	static void (*key_func[])(void) = {k0, k1, k2, k3};
	if(k <= 3) key_func[k]();
	return Check_Collision(stage);
}

static void Draw_Object_H(HUMAN * obj)
{
	Lcd_Draw_Box(obj->x, obj->y, obj->w, obj->h, color[obj->ci]);
}

static void Draw_Object_M(MINERAL * obj)
{
	Lcd_Draw_Box(obj->x, obj->y, obj->w, obj->h, color[obj->ci]);
}

static void Draw_Object_O(OBSTACLE * obj)
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

static void Destroy_OBS(OBSTACLE * obj)
{
	obj->ci = 5;
	Draw_Object_O(obj);
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

// 종유석 생성 함수
void Generate_Fall(void)
{
    fall.x = rand() % (LCDW - OBS_SIZE_X);
    fall.y = 0;
    fall.w = OBS_SIZE_X;
    fall.h = OBS_SIZE_Y;
    fall.ci = OBS_COLOR;
    fall.dir_y = DOWN;

    fall_active = 1;  // 현재 종유석 활성화
    Draw_Object_O(&fall);
}

void Generate_Fall2(void)
{
    fall2.x = rand() % (LCDW - OBS_SIZE_X);
    fall2.y = 0;
    fall2.w = OBS_SIZE_X;
    fall2.h = OBS_SIZE_Y;
    fall2.ci = OBS_COLOR;
    fall2.dir_y = DOWN;

    fall_active = 1;  // 현재 종유석 활성화
    Draw_Object_O(&fall);
}

#if TEST
static void Game_Init_S1(void)
{
	player_hp = 3;
	cnt_time = 0;
	score = 0;
	Lcd_Clr_Screen();
	player.x = 150; player.y = 220; player.w = PLAYER_SIZE_X; player.h = PLAYER_SIZE_Y; player.ci = PLAYER_COLOR; player.dir_x = RIGHT; player.dir_y = UP;
	police1.x = 0; police1.y = 110; police1.w = POLICE_SIZE_X_S1; police1.h = POLICE_SIZE_Y_S1; police1.ci = POLICE_COLOR; police1.dir_x = RIGHT; police1.dir_y = UP;
	police2.x = 0; police2.y = 0; police2.w = 1; police2.h = 1; police2.ci = BACK_COLOR; police2.dir_x = RIGHT; police2.dir_y = UP;
	Lcd_Draw_Box(player.x, player.y, player.w, player.h, color[player.ci]);
	Lcd_Draw_Box(police1.x, police1.y, police1.w, police1.h, color[police1.ci]);

#if 1
	//광물 생성
	Generate_And_Draw_Mineral(&emerald, EMERALD_COLOR);
	Generate_And_Draw_Mineral(&gold, GOLD_COLOR);
	Generate_And_Draw_Mineral(&dia, DIAMOND_COLOR);
#endif
}

static void Game_Init_S2(void)
{
	player_hp = 3;
	score = 0;
	cnt_time = 0;
	Lcd_Clr_Screen();
	player.x = 150; player.y = 220; player.w = PLAYER_SIZE_X; player.h = PLAYER_SIZE_Y; player.ci = PLAYER_COLOR; player.dir_x = RIGHT; player.dir_y = UP;
	police1.x = 0; police1.y = 110; police1.w = POLICE_SIZE_X_S2; police1.h = POLICE_SIZE_Y_S2; police1.ci = POLICE_COLOR; police1.dir_x = RIGHT; police1.dir_y = UP;
	police2.x = 300; police2.y = 110; police2.w = POLICE_SIZE_X_S2; police2.h = POLICE_SIZE_Y_S2; police2.ci = POLICE_COLOR; police2.dir_x = RIGHT; police2.dir_y = UP;
	Lcd_Draw_Box(player.x, player.y, player.w, player.h, color[player.ci]);
	Lcd_Draw_Box(police1.x, police1.y, police1.w, police1.h, color[police1.ci]);
	Lcd_Draw_Box(police2.x, police2.y, police2.w, police2.h, color[police2.ci]);

#if 1
	//광물 생성
	Generate_And_Draw_Mineral(&emerald, EMERALD_COLOR);
	Generate_And_Draw_Mineral(&gold, GOLD_COLOR);
	Generate_And_Draw_Mineral(&dia, DIAMOND_COLOR);
#endif
}

static void Game_Init_S3(void)
{
	player_hp = 3;
	score = 0;
	cnt_time = 0;
	#if TEST
	fall_active = 0;
	fall_timer = 0;
	fall_target_time = 200 + (rand() % 301); // 2~5초 랜덤 초기값 
	#endif
	Lcd_Clr_Screen();
	player.x = 150; player.y = 220; player.w = PLAYER_SIZE_X; player.h = PLAYER_SIZE_Y; player.ci = PLAYER_COLOR; player.dir_x = RIGHT; player.dir_y = UP;
	police1.x = 0; police1.y = 110; police1.w = POLICE_SIZE_X_S3; police1.h = POLICE_SIZE_Y_S3; police1.ci = POLICE_COLOR; police1.dir_x = RIGHT; police1.dir_y = UP;
	police2.x = 300; police2.y = 110; police2.w = POLICE_SIZE_X_S3; police2.h = POLICE_SIZE_Y_S3; police2.ci = POLICE_COLOR; police2.dir_x = RIGHT; police2.dir_y = UP;
	police3.x = 170; police3.y = 60; police3.w = POLICE_SIZE_X_S3; police3.h = POLICE_SIZE_Y_S3; police3.ci = POLICE_COLOR; police3.dir_x = RIGHT; police3.dir_y = UP;
	fall.x = 0; fall.y = 0; fall.w = OBS_SIZE_X; fall.h = OBS_SIZE_Y; fall.ci = OBS_COLOR; fall.dir_x = RIGHT; fall.dir_y = DOWN;
	fall2.x = 0; fall2.y = 0; fall2.w = OBS_SIZE_X; fall2.h = OBS_SIZE_Y; fall2.ci = OBS_COLOR; fall2.dir_x = RIGHT; fall2.dir_y = DOWN;
	Lcd_Draw_Box(player.x, player.y, player.w, player.h, color[player.ci]);
	Lcd_Draw_Box(police1.x, police1.y, police1.w, police1.h, color[police1.ci]);
	Lcd_Draw_Box(police2.x, police2.y, police2.w, police2.h, color[police2.ci]);
	Lcd_Draw_Box(police3.x, police3.y, police3.w, police3.h, color[police3.ci]);

#if 1
	//광물 생성
	Generate_And_Draw_Mineral(&emerald, EMERALD_COLOR);
	Generate_And_Draw_Mineral(&gold, GOLD_COLOR);
	Generate_And_Draw_Mineral(&dia, DIAMOND_COLOR);
#endif
}
#endif

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
	int next_stage = 0;
	int game_clear = 0;
	int pol_col1;
	int pol_col2;
	int pol_col3;
	stage = 1;


	System_Init();
	SysTick_Init_ms(1);
	TIM3_Out_Init();

	Uart_Printf("DOKI DOKI MINING GAME\n");

	Lcd_Init(DIPLAY_MODE);

	Jog_Poll_Init();
	Jog_ISR_Enable(1);
	Uart1_RX_Interrupt_Enable(1);

	//음악 세팅
	#if TEST
	const static unsigned short tone_value[] = {261,277,293,311,329,349,369,391,415,440,466,493,523,554,587,622,659,698,739,783,830,880,932,987};
	enum key{C1, C1_, D1, D1_, E1, F1, F1_, G1, G1_, A1, A1_, B1, C2, C2_, D2, D2_, E2, F2, F2_, G2, G2_, A2, A2_, B2};
	enum note{N16=BASE/4, N8=BASE/2, N4=BASE, N2=BASE*2, N1=BASE*4};
	const int song_base[][2] = {
		{C2, N8}, {E2, N8}, {G2, N8}, {C2, N8},
		{C2, N8}, {E2, N8}, {G2, N8}, {C2, N8},
		{D2, N8}, {F2, N8}, {A2, N8}, {D2, N8},
		{C2, N4}, {B1, N4}
	};
	const int song_clear[][2] = {
		{C1, N4}, {E1, N4}, {G1, N4}, {C2, N4},
		{G1, N8}, {C2, N8}, {E2, N4},
		{G2, N4}, {C2, N4}, {C2, N2}
	};
	const int song_over[][2] = {
		{C2, N4}, {B1, N4}, {A1, N4},
		{G1, N8}, {E1, N8}, {D1, N4},
		{C1, N2}
	};
	const int song_perfect[][2] = {
		{C1, N8}, {E1, N8}, {G1, N8},
		{C2, N4},
		{B1, N8}, {G1, N8}, {C2, N8},
		{C2, N2}
	};

	const char * note_name[] = {"C1", "C1#", "D1", "D1#", "E1", "F1", "F1#", "G1", "G1#", "A1", "A1#", "B1", "C2", "C2#", "D2", "D2#", "E2", "F2", "F2#", "G2", "G2#", "A2", "A2#", "B2"};

	// 현재 노래
	const int (*current_song)[2] = song_base;
	int current_song_len = sizeof(song_base) / sizeof(song_base[0]);
	int song_index = 0;


	#endif

	for(;;)
	{
		#if TEST
		if(stage == 1)
		{
			Game_Init_S1();
			TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD*10);
			Lcd_Printf(300,0,BLUE,WHITE,2,2,"%d", score);
		}
		else if(stage == 2)
		{
			Game_Init_S2();
			TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD*10);
			Lcd_Printf(300,0,BLUE,WHITE,2,2,"%d", score);
		}
		else if(stage == 3)
		{
			Game_Init_S3();
			TIM4_Repeat_Interrupt_Enable(1, TIMER_PERIOD*10);
			Lcd_Printf(300,0,BLUE,WHITE,2,2,"%d", score);
		}
		#endif
#if 0
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
			game_clear = 0;

			#if TEST
			// TIM3와 Systick을 이용한 배경음
			if(SYSTICK_expired)
			{
				SYSTICK_expired = 0;

				TIM3_Out_Stop();

				TIM3_Out_Freq_Generation(tone_value[current_song[song_index][0]]);

				tone_duration_ms = current_song[song_index][1];

				if (song_index == current_song_len - 1)
					song_index = 0;
				else
					song_index++;
			}
			#endif


			if(Jog_key_in) 
			{
				player.ci = BACK_COLOR;
				Draw_Object_H(&player);
				check_state = Player_Move(Jog_key, stage);

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
							cnt_time = 0;
							TIM2_Repeat_Interrupt_Enable_time(1, 1000);
							#endif
							Uart1_Printf("MINING GOLD\n");
						}
						break;
					case TOUCH_EMERALD:
						now_touching_emerald = 1;
						if (!touching_emerald) {
							#if DEBUG
							cnt_time = 0;
							TIM2_Repeat_Interrupt_Enable_time(1, 1000);
							#endif
							Uart1_Printf("MINING EMERALD\n");
						}
						break;
					case TOUCH_DIA:
						now_touching_diamond = 1;
						if (!touching_diamond) {
							#if DEBUG
							cnt_time = 0;
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
				#if 1
				switch (cnt_time)
				{
					case 1: 
					{
						if(touching_gold)
						{
							Destroy_MINERAL(&gold); 
							score += 1;
							cnt_time = 0;
							Lcd_Printf(300,0,BLUE,WHITE,2,2,"%d", score);
							Uart1_Printf("Mine Gold! +1point\n");
							Uart1_Printf("Score: %d\n", score);
							TIM2_Stop();
						}
						break;
					}

					case 3:
					{
						if(touching_emerald)
						{
							Destroy_MINERAL(&emerald); 
							score += 2;
							cnt_time = 0;
							Lcd_Printf(300,0,BLUE,WHITE,2,2,"%d", score);
							Uart1_Printf("Mine Emerald! +2point\n");
							Uart1_Printf("Score: %d\n", score);
							TIM2_Stop();
						}
						break;
					}

					case 5:
					{
						if(touching_diamond)
						{
							Destroy_MINERAL(&dia); 
							cnt_time = 0;
							score += 3;
							Lcd_Printf(300,0,BLUE,WHITE,2,2,"%d", score);
							Uart1_Printf("Mine Diamond! +3point\n");
							Uart1_Printf("Score: %d\n", score);
							TIM2_Stop();
						}
						break;
					}
					default:
						break;
				}
				#endif
				#if 0
				if(cnt_time == 1)
				{
					if(touching_gold)
					{
						Destroy_MINERAL(&gold);
						score += 1;
						cnt_time = 0;
						Uart1_Printf("Mine Gold! +1point\n");
						Uart1_Printf("Score: %d\n", score);
						Lcd_Printf(300,0,BLUE,WHITE,2,2,"%d", score);
						TIM2_Stop();
					}
					else if(touching_emerald)
					{
						Destroy_MINERAL(&emerald); 
						score += 2;
						cnt_time = 0;
						Uart1_Printf("Mine Emerald! +2point\n");
						Uart1_Printf("Score: %d\n", score);
						Lcd_Printf(300,0,BLUE,WHITE,2,2,"%d", score);
						TIM2_Stop();
					}
					else if(touching_diamond)
					{
						Destroy_MINERAL(&dia); 
						score += 3;
						cnt_time = 0;
						Uart1_Printf("Mine Diamond! +3point\n");
						Uart1_Printf("Score: %d\n", score);
						Lcd_Printf(300,0,BLUE,WHITE,2,2,"%d", score);
						TIM2_Stop();
					}
				}
				#endif
				
				TIM2_expired = 0;
			}
			#endif
			if(TIM4_expired) 
			{
				#if TEST
				switch (stage)
				{
					case 1: 
						pol_col1 = POLICE_COLOR; pol_col2 = BACK_COLOR; pol_col3 = BACK_COLOR; break;
					case 2:
						pol_col1 = POLICE_COLOR; pol_col2 = POLICE_COLOR; pol_col3 = BACK_COLOR; break;
					case 3:
						pol_col1 = POLICE_COLOR; pol_col2 = POLICE_COLOR; pol_col3 = POLICE_COLOR; break;
					default:
						pol_col1 = BACK_COLOR; pol_col2 = BACK_COLOR; pol_col3 = BACK_COLOR; break;
				}
				#endif

				#if TEST
				if (stage == 3)
				{
					// 종유석이 이미 떨어지고 있는 경우
					if (fall_active)
					{
						fall.ci = BACK_COLOR;
						fall2.ci = BACK_COLOR;
						Draw_Object_O(&fall); // 이전 위치 지우기
						Draw_Object_O(&fall2); // 이전 위치 지우기
						check_state = OBS_Move(stage); // Y 위치 증가 + 충돌 확인
						if(check_state == GAME_OVER) game_over = 1;
						fall.ci = OBS_COLOR;
						fall2.ci = OBS_COLOR;
						Draw_Object_O(&fall); // 새 위치 그림
						Draw_Object_O(&fall2); // 새 위치 그림

						if (check_state == GAME_OVER) game_over = 1;

						// 종유석이 사라졌다면 타이머 리셋
						if (fall.y + fall.h == 0)
						{
							fall_active = 0;
							fall_timer = 0;
							fall_target_time = 200 + (rand() % 301);  // 2000~3000ms
						}

						if (fall2.y + fall2.h == 0)
						{
							fall_active = 0;
							fall_timer = 0;
							fall_target_time = 200 + (rand() % 301);  // 2000~3000ms
						}
					}
					else
					{
						// 종유석 비활성 상태일 때 대기 시간 누적
						fall_timer += TIMER_PERIOD;  // TIM4 인터럽트 주기만큼 증가
						Uart_Printf("fall_timer = %d, target = %d\n", fall_timer, fall_target_time);

						if (fall_timer >= fall_target_time)
						{
							Generate_Fall(); // 새 종유석 생성
							Generate_Fall2(); // 새 종유석 생성
						}
					}
				}
				#endif
				police1.ci = BACK_COLOR;
				police2.ci = BACK_COLOR;
				police3.ci = BACK_COLOR;
				Draw_Object_H(&police1);
				Draw_Object_H(&police2);
				Draw_Object_H(&police3);
				//check_state = Police_Move();
				check_state = Police_Move(stage);
				if(check_state == GAME_OVER) game_over = 1;
				police1.ci = pol_col1;
				police2.ci = pol_col2;
				police3.ci = pol_col3;
				Draw_Object_H(&police1);
				Draw_Object_H(&police2);
				Draw_Object_H(&police3);
				#if 1
				Draw_Object_Restore(&dia);
				Draw_Object_Restore(&gold);
				Draw_Object_Restore(&emerald);
				#endif
				
				TIM4_expired = 0;
			}

			// 검증을 위한 game over 주석 
			#if 1
			if(game_over)
			{
				TIM2_Stop();
				TIM3_Out_Stop();
				TIM4_Repeat_Interrupt_Enable(0, 0);
				Lcd_Printf(300,0,RED,WHITE,2,2,"%c", 'X');

				//game over 노래 재생
				current_song = song_over;
				current_song_len = sizeof(song_over) / sizeof(song_over[0]);
				song_index = 0;

				while(current_song == song_over)
				{
					if(SYSTICK_expired)
					{
						SYSTICK_expired = 0;

						TIM3_Out_Stop();

						TIM3_Out_Freq_Generation(tone_value[current_song[song_index][0]]);

						tone_duration_ms = current_song[song_index][1];

						if (song_index == current_song_len - 1)
						{
							song_index = 0;
							current_song = 0;
						}
						else
							song_index++;
					}
				}
				TIM3_Out_Stop();

				Uart_Printf("Game Over, Please press any key to continue.\n");
				Jog_Wait_Key_Pressed();
				Jog_Wait_Key_Released();
				Uart_Printf("Game Start\n");

				//기본 노래로 복귀
				current_song = song_base;
				current_song_len = sizeof(song_base) / sizeof(song_base[0]);
				song_index = 0;
				score = 0;
				stage = 1;
				next_stage = 0;
				cnt_time = 0;
				break;
			}
			#endif

			#if DEBUG
			if(stage)
			{
				if((stage == 1) && (score == 6))
				{
					next_stage = 1;
				}
				else if((stage == 2) && (score == 6))
				{
					next_stage = 1;
				}
				else if((stage == 3) && (score == 6))
				{
					game_clear = 1;
				}
			}
			#endif

			#if DEBUG
			if(next_stage)
			{
				TIM2_Stop();
				TIM3_Out_Stop();
				TIM4_Repeat_Interrupt_Enable(0, 0);
				

				//clear 노래
				current_song = song_clear;
				current_song_len = sizeof(song_clear) / sizeof(song_clear[0]);
				song_index = 0;

				while(current_song == song_clear)
				{
					if(SYSTICK_expired)
					{
						SYSTICK_expired = 0;

						TIM3_Out_Stop();

						TIM3_Out_Freq_Generation(tone_value[current_song[song_index][0]]);

						tone_duration_ms = current_song[song_index][1];

						if (song_index == current_song_len - 1)
						{
							song_index = 0;
							current_song = 0;
						}
						else
							song_index++;
					}
				}
				TIM3_Out_Stop();
				
				Lcd_Printf(300,0,BLUE,WHITE,2,2,"%c", 'O');
				Uart_Printf("Congratulations! total Score = %d\n", score);
				Uart_Printf("Stage %d Clear! Press any key to continue\n", stage);
				Jog_Wait_Key_Pressed();
				Jog_Wait_Key_Released();
				Uart_Printf("Game Start\n");

				// 기본 노래로 복귀
				current_song = song_base;
				current_song_len = sizeof(song_base) / sizeof(song_base[0]);
				song_index = 0;

				score = 0;
				stage ++;
				next_stage = 0;
				cnt_time = 0;
				break;
			}
			#endif

			#if DEBUG
			if(game_clear)
			{
				TIM2_Stop();
				TIM3_Out_Stop();
				TIM4_Repeat_Interrupt_Enable(0, 0);

				//clear 노래
				current_song = song_perfect;
				current_song_len = sizeof(song_perfect) / sizeof(song_perfect[0]);
				song_index = 0;

				Uart_Printf("Congratulations!\n");
				Uart_Printf("You Clear The Game!\n");
				Uart_Printf("Total Score = %d\n", score);
				while(current_song == song_perfect)
				{
					if(SYSTICK_expired)
					{
						SYSTICK_expired = 0;

						TIM3_Out_Stop();

						TIM3_Out_Freq_Generation(tone_value[current_song[song_index][0]]);

						tone_duration_ms = current_song[song_index][1];

						if (song_index == current_song_len - 1)
						{
							song_index = 0;
							current_song = 0;
						}
						else
							song_index++;
					}
				}
				TIM3_Out_Stop();
				
				Uart_Printf("Press any key to restart\n");
				Jog_Wait_Key_Pressed();
				Jog_Wait_Key_Released();

				Uart_Printf("Game Start\n");

				// 기본 노래로 복귀
				current_song = song_base;
				current_song_len = sizeof(song_base) / sizeof(song_base[0]);
				song_index = 0;

				score = 0;
				stage = 1;
				next_stage = 0;
				cnt_time = 0;
				break;
			}
			#endif
		}
	}
}

#endif
#endif