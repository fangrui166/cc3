#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>

#define USE_MY_RANDOM 1


#define MAX_COIN    15
#define MIN_RANDOM  1000
#define MAX_RANDOM  9999-MIN_RANDOM
#define MAX_COUNTOFDAY 25

static unsigned short countOfDay = 0;
static unsigned int validity = 0;
#if USE_MY_RANDOM
#define RANDOM_MAX 0x7FFFFFFF
static unsigned long my_seed = 0;
static long my_do_rand(unsigned long *value)
{

   long quotient, remainder, t;

   quotient = *value / 127773L;
   remainder = *value % 127773L;
   t = 16807L * remainder - 2836L * quotient;

   if (t <= 0)
      t += 0x7FFFFFFFL;
  return ((*value = t) % ((unsigned long)RANDOM_MAX + 1));
}
int my_rand(void)
{
   return my_do_rand(&my_seed);
}
void my_srand(unsigned int seed)
{
   my_seed = seed;
}
#endif

struct tm Time_ConvUnixToCalendar(time_t t)
{
	struct tm *t_tm;
	t_tm = localtime(&t);
	t_tm->tm_year += 1900;	//localtime?????μ?m_yearˇР??μ￡????3??
    t_tm->tm_mon +=1;
	return *t_tm;
}

unsigned int totp_get_date_seed(void)
{
	time_t timer;
	struct tm t_tm;
    unsigned int date_seed = 0;
	time(&timer);
    t_tm = Time_ConvUnixToCalendar(timer);
    date_seed = (((t_tm.tm_year%100)*10000 )
            + ((t_tm.tm_mon%100) * 100)
            + (t_tm.tm_mday%100));
    date_seed &= 0xFFFFF;
    return date_seed;
}

unsigned int totp_my_random(int start, int end)
{
	int max = end;

#if USE_MY_RANDOM
    return start+(my_rand()%(max+1));
#else
#ifdef _WIN32
	return start+rand()%(max+1);
#else
	return start+random()%(max+1);
#endif
#endif
}
static void set_srand(unsigned int seed)
{
#if USE_MY_RANDOM
    my_srand(seed);
#else
    srand(seed);
#endif
}
/*
    date --> year month day --> 0xFFFF F
    count --> 0xFF
*/
static unsigned int totp_get_random_date(unsigned int date, unsigned char count)
{

	/*     date    |  count
	 *  0x FFFF F  |  0FF
	 */
    int seed = 0;
    seed = (((date & 0xFFFFF) << 12) | (count & 0x0FF));
    set_srand(seed);
    return totp_my_random(MIN_RANDOM,MAX_RANDOM);
}
/*
    serial --> 0xFFFF
    count  --> 0xFF
    coni   --> 0xFF
*/
static unsigned int totp_get_random_id_coin(unsigned short serial_id, unsigned char count,
                                                    unsigned char coin)
{
	/*     serial  |  count  | coin
	 *    0xFFFF   |   FF    |  FF
	 */
    int seed = 0;
    seed = (serial_id << 16) | (count << 8) | coin;
    set_srand(seed);
    return totp_my_random(MIN_RANDOM,MAX_RANDOM);
}
main()
{
	int coin = 0;
	int flag = 0;
	unsigned int securityCode1,securityCode2;
    unsigned int date_seed;
	unsigned short serial_number;
	printf("请输入使用机器的序列号［0：9999］:\n");
	flag = scanf("%d",&serial_number);
	if(1 !=flag) {
		getchar();
	}
	if(serial_number > 9999){
		printf("输入数据有误！设为默认值：1234\n");
		serial_number = 1234;
	}
	while(1){
		printf("请输入投币数[1:14]：\n");
		flag = scanf("%d",&coin);
		if(1 !=flag) {
			getchar();
			coin = MAX_COIN +1;
		}
		if(coin > MAX_COIN){
			printf("输入数据有误！\n");
			continue;
		}
        date_seed = totp_get_date_seed();
		countOfDay ++;
		if(countOfDay > MAX_COUNTOFDAY){
			printf("今天的币卖完了\n");
			continue;
		}
		securityCode1 = totp_get_random_date(date_seed, countOfDay);
		securityCode2 = totp_get_random_id_coin(serial_number, countOfDay, coin);
		printf("验证码：%d%d\n\n",securityCode1,securityCode2);
	}
}
