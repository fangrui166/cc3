#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <string.h>

#define USE_MY_RANDOM 1

#define MAX_COIN    15
#define MIN_RANDOM  1000
#define MAX_RANDOM  9999-MIN_RANDOM
#define MAX_COUNTOFDAY 25
static unsigned short countOfDay = 0;
static unsigned int validity = 0;
typedef struct {
	int securityCode[MAX_COUNTOFDAY*MAX_COIN];
	int count;
    unsigned int date_seed_bk;
}invalidSecurityCode;
invalidSecurityCode invalidCode = {0};
unsigned int serial_number;

typedef enum{
    COIN_TYPE_OK,
    COIN_TYPE_USED,
    COIN_TYPE_FAKE,
}coin_analysis_type;
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
static unsigned short totp_get_serial_id(void)
{
    return serial_number;
}

struct tm Time_ConvUnixToCalendar(time_t t)
{
	struct tm *t_tm;
	t_tm = localtime(&t);
	t_tm->tm_year += 1900;
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
    printf("%d-%d-%d \n",t_tm.tm_year,t_tm.tm_mon,t_tm.tm_mday);
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
    return start+my_rand()%(max+1);
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
static coin_analysis_type totp_coin_count_analysis(unsigned int security_code,
                                            unsigned char * coin_count )
{
    int index_coin,index_count,index_invalid;
    unsigned int date_seed;
    unsigned short serial_id;
    coin_analysis_type ret = COIN_TYPE_OK;
    serial_id = totp_get_serial_id();
    date_seed = totp_get_date_seed();
    * coin_count = 0;
    if(date_seed != invalidCode.date_seed_bk){
        memset(&invalidCode, 0, sizeof(invalidSecurityCode));
        invalidCode.date_seed_bk = date_seed;
    }
    //printf("serial_id:%d, date_seed:%d\n",serial_id, date_seed);
    for(index_coin = 1; index_coin < MAX_COIN; index_coin ++){
        for(index_count = 0; index_count < MAX_COUNTOFDAY; index_count ++){
            unsigned short tempcode1,tempcode2;
            tempcode1 = totp_get_random_date(date_seed, index_count);
            tempcode2 = totp_get_random_id_coin(serial_id, index_count, index_coin);
            //printf("array[%d,%d]%d\n",index_coin, index_count, tempcode1*10000+tempcode2);
            if((tempcode1*10000+tempcode2) == security_code){
                for(index_invalid = 0; index_invalid < invalidCode.count;
                index_invalid ++){
                    if(security_code ==
                        invalidCode.securityCode[index_invalid]){
                        printf("验证码已经被使用过\n");
                        ret = COIN_TYPE_USED;
                        break;
                    }
                }
                if(index_invalid >= invalidCode.count){
                    printf("投币个数: %d\n",index_coin);
                    * coin_count = index_coin;
                    invalidCode.securityCode[invalidCode.count] = security_code;
                    invalidCode.count ++;
                    ret = COIN_TYPE_OK;
                }
                break;
            }
        }
        if(index_count < MAX_COUNTOFDAY) break;
    }
    if(index_coin >= MAX_COIN){
        printf("验证码无效\n");
        ret = COIN_TYPE_FAKE;
    }
    return ret;
}

main()
{
	int securityCode;
	int flag = 0;
	unsigned char coin = 0;
	coin_analysis_type ret;
	printf("请设置本机序列号［0：9999］:\n");
	flag = scanf("%d",&serial_number);
	if(1 !=flag) {
		getchar();
	}
	if(serial_number > 9999){
		printf("输入数据有误！设为默认值：1234\n");
		serial_number = 1234;
	}
	while(1){
		printf("请输入验证码：\n");
		scanf("%d",&securityCode);
		ret = totp_coin_count_analysis(securityCode, &coin);
	}
}
