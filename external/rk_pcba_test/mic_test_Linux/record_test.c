#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "record_test.h"  
#define LOG_TAG "i2c_controller"
//#define printf(format, args...) fprintf (stderr, format, args)

#define	    THRES_REC_FREQ	(20000)
#define 	THRES_REC_MAX	(32000)
#define 	THRES_REC_MIN	(1000)
#define 	THRES_REC_PASS_RATE	(60)
#define 	THRES_REC_TEST_TIME	(3)
// 原始音频在求Sum(xi^2)时控制在unsigned int32范围内的左移值
#define ORIGINAL_AUDIO_LEFTNUM        (13)   
// 对于录的音频，在ORIGINAL_AUDIO_LEFTNUM 的基础上增加的左移值
#define LEFT_SHIFT                    (1)     
// 录音音频在求Sum(xi^2)时控制在unsigned int32范围内的左移值
#define RECORD_AUDIO_LEFTNUM          (ORIGINAL_AUDIO_LEFTNUM +LEFT_SHIFT ) 
// Sum(xi*yj)控制在int32范围内的左移值
#define MOLE_LEFTNUM                  (RECORD_AUDIO_LEFTNUM + 1) 

static int check_index(int signal, int input[], int input_length);
static int check_agreement(int signal, int input[], int input_length);

//static FILE * fp_in;
//static FILE * fp_out[12];
static int index_result;
static int agreement_result;

static short rec_400hz[] =
{
	0x0001,0x071B,0x0E0A,0x149E,0x1AB2,0x201D,0x24BE,0x2877,0x2B32,0x2CDB,
	0x2D6B,0x2CDC,0x2B32,0x2878,0x24BE,0x201E,0x1AB2,0x149E,0x0E08,0x071B,
	0x0000,0xF8E6,0xF1F7,0xEB62,0xE54E,0xDFE3,0xDB41,0xD789,0xD4CE,0xD325,
	0xD296,0xD324,0xD4CF,0xD788,0xDB42,0xDFE3,0xE54E,0xEB62,0xF1F7,0xF8E5,
	0xFFFF,0x071B,0x0E08,0x149E,0x1AB2,0x201D,0x24BE,0x2877,0x2B32,0x2CDC,
	0x2D6B,0x2CDC,0x2B31,0x2878,0x24BE,0x201D,0x1AB3,0x149E,0x0E09,0x071B,
	0x0000,0xF8E5,0xF1F7,0xEB62,0xE54E,0xDFE3,0xDB43,0xD788,0xD4CE,0xD324,
	0xD295,0xD324,0xD4CE,0xD789,0xDB42,0xDFE3,0xE54F,0xEB61,0xF1F8,0xF8E5,
	0x0001,0x071B,0x0E09,0x149E,0x1AB2,0x201D,0x24BD,0x2878,0x2B32,0x2CDC,
	0x2D6B,0x2CDB,0x2B32,0x2876,0x24BE,0x201D,0x1AB2,0x149E,0x0E09,0x071B,
	0x0000,0xF8E5,0xF1F7,0xEB61,0xE54E,0xDFE2,0xDB42,0xD788,0xD4CF,0xD325,
	0xD295,0xD324,0xD4CF,0xD788,0xDB42,0xDFE3,0xE54E,0xEB61,0xF1F7,0xF8E5,
	0x0000,0x071B,0x0E08,0x149E,0x1AB3,0x201E,0x24BE,0x2877,0x2B32,0x2CDB,
	0x2D6B,0x2CDB,0x2B31,0x2878,0x24BE,0x201D,0x1AB2,0x149E,0x0E08,0x071B,
	0x0000,0xF8E5,0xF1F7,0xEB62,0xE54E,0xDFE2,0xDB42,0xD789,0xD4CF,0xD325,
	0xD295,0xD324,0xD4CE,0xD788,0xDB41,0xDFE3,0xE54E,0xEB61,0xF1F7,0xF8E6,
	0xFFFF,0x071B,0x0E09,0x149E,0x1AB2,0x201E,0x24BD,0x2877,0x2B32,0x2CDB,
	0x2D6B,0x2CDB,0x2B31,0x2877,0x24BF,0x201E,0x1AB1,0x149D,0x0E09,0x071B,
	0x0000,0xF8E6,0xF1F7,0xEB62,0xE54E,0xDFE2,0xDB42,0xD789,0xD4CE,0xD325,
	0xD296,0xD325,0xD4CE,0xD788,0xDB42,0xDFE3,0xE54E,0xEB61,0xF1F7,0xF8E5,
	0x0000,0x071A,0x0E09,0x149F,0x1AB1,0x201C,0x24BE,0x2877,0x2B31,0x2CDC,
	0x2D6B,0x2CDB,0x2B32,0x2877,0x24BD,0x201D,0x1AB3,0x149E,0x0E09,0x071B,
	0x0000,0xF8E5,0xF1F8,0xEB61,0xE54E,0xDFE3,0xDB42,0xD789,0xD4CE,0xD325,
	0xD295,0xD325,0xD4CF,0xD788,0xDB42,0xDFE3,0xE54F,0xEB62,0xF1F7,0xF8E5,
	0x0000,0x071B,0x0E08,0x149E,0x1AB2,0x201C,0x24BE,0x2877,0x2B31,0x2CDC,
	0x2D6A,0x2CDB,0x2B32,0x2877,0x24BE,0x201D
};


static short rec_word_mark[] = //  1000hz 48k rate
{
	0x013B,0x0695,0x0E9E,0x17D9,0x209A,0x27CD,0x2D72,0x3227,0x3682,0x3A8B,
	0x3DCB,0x3FAD,0x3FFF,0x3F06,0x3D28,0x3A9F,0x3747,0x32DC,0x2D48,0x26BB,
	0x1F9A,0x182F,0x1087,0x0887,0x002F,0xF7AD,0xEF56,0xE771,0xE023,0xD95C,
	0xD319,0xCD75,0xC8B0,0xC4FD,0xC26E,0xC0EF,0xC068,0xC0DB,0xC25D,0xC503,
	0xC8CA,0xCD87,0xD30D,0xD942,0xE012,0xE784,0xEF77,0xF7BF,0x0018,0x085C,
	0x1074,0x184D,0x1FCF,0x26D4,0x2D21,0x3299,0x3727,0x3ACC,0x3D81,0x3F32,
	0x3FC6,0x3F38,0x3D87,0x3ACE,0x3724,0x3290,0x2D1C,0x26D5,0x1FD9,0x1854,
	0x1071,0x0853,0x000D,0xF7B9,0xEF82,0xE795,0xE019,0xD937,0xD2FA,0xCD7C,
	0xC8D3,0xC518,0xC26A,0xC0D2,0xC04F,0xC0DE,0xC276,0xC51B,0xC8C9,0xCD72,
	0xD2F7,0xD93F,0xE027,0xE799,0xEF7B,0xF7AD,0x0005,0x0857,0x1080,0x185E,
	0x1FD4,0x26C7,0x2D12,0x3293,0x3731,0x3ADD,0x3D86,0x3F29,0x3FB9,0x3F32,
	0x3D8F,0x3ADD,0x372C,0x3289,0x2D0F,0x26CC,0x1FE0,0x1860,0x107A,0x084D,
	0xFFFF,0xF7B3,0xEF85,0xE79F,0xE01F,0xD932,0xD2F3,0xCD77,0xC8D5,0xC523,
	0xC271,0xC0D2,0xC049,0xC0D6,0xC276,0xC523,0xC8D1,0xCD71,0xD2F2,0xD939,
	0xE025,0xE79E,0xEF82,0xF7AC,0x0000,0x0851,0x107E,0x1864,0x1FDA,0x26C9,
	0x2D0D,0x328E,0x3730,0x3ADF,0x3D8C,0x3F2B,0x3FB8,0x3F2E,0x3D8B,0x3ADE,
	0x372E,0x328A,0x2D0B,0x26CA,0x1FDD,0x1863,0x107D,0x084F,0x0000,0xF7AF,
	0xEF83,0xE79D,0xE024,0xD934,0xD2F2,0xCD73,0xC8D2,0xC523,0xC274,0xC0D4,
	0xC048,0xC0D5,0xC277,0xC522,0xC8D3,0xCD72,0xD2F2,0xD936,0xE023,0xE79E,
	0xEF83,0xF7AF,0x0000,0x0851,0x107D,0x1863,0x1FDD,0x26CA,0x2D0F,0x328C,
	0x372E,0x3ADF,0x3D8D,0x3F2C,0x3FB6,0x3F2B,0x3D8C,0x3ADD,0x372E,0x328C,
	0x2D0D,0x26C9,0x1FDC,0x1862,0x107E,0x0853,0x0002,0xF7AE,0xEF83,0xE79D,
	0xE025,0xD936,0xD2F2,0xCD74,0xC8D4,0xC523,0xC275,0xC0D5,0xC049,0xC0D4,
	0xC273,0xC522,0xC8D3,0xCD73,0xD2F3,0xD936,0xE023,0xE79C,0xEF80,0xF7AF,
	0x0000,0x0851,0x107F,0x1860,0x1FDC,0x26CB,0x2D0E,0x328D,0x372E,0x3ADE,
	0x3D8B,0x3F2C,0x3FB8,0x3F2B,0x3D8D,0x3ADF
};

/* Q15,求解1/sqrt(x).其中x范围为[0.25,1),表是10bit的 */
const unsigned short g_s16Table_sqrt_10Bit[769] = {
65535,65408,65281,65155,65030,64905,64781,64658,64535,64414,64292,64172,64052,63933,63814,63696,63579,63463,63347,63232,
63117,63003,62889,62777,62664,62553,62442,62331,62222,62112,62004,61895,61788,61681,61575,61469,61363,61258,61154,61050,
60947,60845,60742,60641,60540,60439,60339,60239,60140,60041,59943,59845,59748,59651,59555,59459,59364,59269,59175,59081,
58987,58894,58801,58709,58617,58526,58435,58344,58254,58165,58075,57986,57898,57810,57722,57635,57548,57462,57376,57290,
57205,57120,57035,56951,56867,56784,56700,56618,56535,56453,56372,56291,56210,56129,56049,55969,55889,55810,55731,55653,
55574,55497,55419,55342,55265,55188,55112,55036,54960,54885,54810,54735,54661,54587,54513,54439,54366,54293,54221,54148,
54076,54004,53933,53862,53791,53720,53650,53580,53510,53440,53371,53302,53233,53165,53097,53029,52961,52894,52826,52760,
52693,52627,52560,52494,52429,52363,52298,52233,52169,52104,52040,51976,51912,51849,51785,51722,51660,51597,51535,51473,
51411,51349,51288,51226,51165,51104,51044,50984,50923,50863,50804,50744,50685,50626,50567,50508,50450,50391,50333,50275,
50218,50160,50103,50046,49989,49932,49876,49819,49763,49707,49652,49596,49541,49485,49430,49376,49321,49266,49212,49158,
49104,49050,48997,48943,48890,48837,48784,48731,48679,48627,48574,48522,48470,48419,48367,48316,48265,48214,48163,48112,
48061,48011,47961,47911,47861,47811,47761,47712,47663,47613,47564,47516,47467,47418,47370,47322,47273,47225,47178,47130,
47082,47035,46988,46941,46894,46847,46800,46754,46707,46661,46615,46569,46523,46477,46432,46386,46341,46296,46251,46206,
46161,46116,46072,46027,45983,45939,45895,45851,45807,45764,45720,45677,45633,45590,45547,45504,45462,45419,45376,45334,
45292,45249,45207,45165,45124,45082,45040,44999,44957,44916,44875,44834,44793,44752,44711,44671,44630,44590,44550,44510,
44470,44430,44390,44350,44310,44271,44232,44192,44153,44114,44075,44036,43997,43959,43920,43882,43843,43805,43767,43729,
43691,43653,43615,43577,43540,43502,43465,43428,43390,43353,43316,43279,43243,43206,43169,43133,43096,43060,43024,42987,
42951,42915,42879,42844,42808,42772,42737,42701,42666,42631,42595,42560,42525,42490,42456,42421,42386,42352,42317,42283,
42248,42214,42180,42146,42112,42078,42044,42010,41977,41943,41910,41876,41843,41809,41776,41743,41710,41677,41644,41611,
41579,41546,41514,41481,41449,41416,41384,41352,41320,41288,41256,41224,41192,41160,41129,41097,41065,41034,41003,40971,
40940,40909,40878,40847,40816,40785,40754,40723,40693,40662,40631,40601,40571,40540,40510,40480,40450,40420,40390,40360,
40330,40300,40270,40241,40211,40182,40152,40123,40093,40064,40035,40006,39977,39948,39919,39890,39861,39832,39803,39775,
39746,39718,39689,39661,39632,39604,39576,39548,39520,39492,39464,39436,39408,39380,39352,39325,39297,39269,39242,39215,
39187,39160,39133,39105,39078,39051,39024,38997,38970,38943,38916,38890,38863,38836,38810,38783,38756,38730,38704,38677,
38651,38625,38599,38572,38546,38520,38494,38469,38443,38417,38391,38365,38340,38314,38289,38263,38238,38212,38187,38162,
38136,38111,38086,38061,38036,38011,37986,37961,37936,37911,37887,37862,37837,37813,37788,37764,37739,37715,37690,37666,
37642,37617,37593,37569,37545,37521,37497,37473,37449,37425,37401,37378,37354,37330,37307,37283,37260,37236,37213,37189,
37166,37142,37119,37096,37073,37050,37027,37003,36980,36957,36935,36912,36889,36866,36843,36820,36798,36775,36753,36730,
36708,36685,36663,36640,36618,36596,36573,36551,36529,36507,36485,36463,36441,36419,36397,36375,36353,36331,36309,36287,
36266,36244,36222,36201,36179,36158,36136,36115,36093,36072,36051,36029,36008,35987,35966,35945,35924,35903,35882,35861,
35840,35819,35798,35777,35756,35735,35715,35694,35673,35653,35632,35612,35591,35571,35550,35530,35509,35489,35469,35448,
35428,35408,35388,35368,35347,35327,35307,35287,35267,35247,35228,35208,35188,35168,35148,35129,35109,35089,35070,35050,
35030,35011,34991,34972,34953,34933,34914,34894,34875,34856,34837,34817,34798,34779,34760,34741,34722,34703,34684,34665,
34646,34627,34608,34589,34571,34552,34533,34514,34496,34477,34458,34440,34421,34403,34384,34366,34347,34329,34310,34292,
34237,34219,34201,34183,34164,34146,34128,34110,34092,34074,34056,34038,34020,34002,33985,33967,33949,33931,33913,33896,
33878,33860,33843,33825,33807,33790,33772,33755,33737,33720,33703,33685,33668,33650,33633,33616,33599,33581,33564,33547,
33530,33513,33496,33478,33461,33444,33427,33410,33393,33377,33360,33343,33326,33309,33292,33276,33259,33242,33225,33209,
33192,33175,33159,33142,33126,33109,33093,33076,33060,33043,33027,33011,32994,32978,32962,32945,32929,32913,32897,32881,
32864,32848,32832,32816,32800,32784
};

const int g_mic_table[5] = {
7,1,8,2,9	
};

unsigned int I32MultI16(unsigned int x, unsigned short y)
{
	unsigned int iSum = 0;
	unsigned int tmp;

	iSum = iSum + (x>>16)*(y);
	tmp = x&0xFFFF;
	tmp = tmp*y;
	tmp = tmp>>16;
	iSum = iSum + tmp;
	return iSum;
}

unsigned int I32MultI32(unsigned int x,unsigned int y)
{
	unsigned int iSum = 0;
	unsigned int tmpy,tmpx,tmp;
	iSum = iSum + (x>>16)*(y>>16);

	tmpy = y&0xFFFF;
	tmpx = x>>16;
	tmp = (tmpx*tmpy)>>16;
	iSum = iSum + (tmp);

	tmpx = x&0xFFFF;
	tmpy = y>>16;
	tmp = (tmpx*tmpy)>>16;
	iSum = iSum + tmp;
	return iSum;
}

// 查表
unsigned short GetDenomValueByTable(unsigned int s32ParamB, unsigned int s32ParamC, short * psQ)
{
	unsigned int i32Result = 0;
	short sindex = 0;
	unsigned int i32Denom;
	unsigned short sValue;      

	short s16Q = 0;

	i32Denom = I32MultI32(s32ParamB,s32ParamC);

#if FLOAT_FLAG
	double x =  1.0 /sqrt((double)i32Denom);
#endif

	if(0 == i32Denom){
		++i32Denom;
	}

	if (!(i32Denom & 0xFFFF0000))
	{
		/* All of first 16 bits are zero */
		i32Denom <<= 16;
		s16Q = 8;
	}
	if (!(i32Denom & 0xFF000000))
	{
		/* All of first 8 bits are zero */
		i32Denom <<= 8;
		s16Q += 4;
	}
	if (!(i32Denom & 0xF0000000))
	{
		/* All of first 4 bits are zero */
		i32Denom <<= 4;
		s16Q += 2;
	}
	if (!(i32Denom & 0xC0000000))
	{
		/* All of first 2 bits are zero */
		i32Denom <<= 2;
		s16Q += 1;
	}

	
	//s32ParamB -= 0x80000000;
	i32Denom -= 0x3FE00001;

	sindex = i32Denom >> 22;

	sValue = g_s16Table_sqrt_10Bit[sindex];

	/****************************************/
#if LOG_FLAG
	double tx = 1.0*32768*32768*2/(1<<s16Q);
	double tmp = x*tx;
	assert(fabs(tmp-sValue) < 60);
#endif
	/********************************************/

	*psQ = s16Q;
	return sValue;
}

/* 获取分母中sum(xi^2)和sum(yj^2)的值 */
unsigned int GetDenomValue(short * px, int inLen, short nQ)
{
	unsigned int iSum = 0;
	int k;
	for( k = 0; k < inLen; k++)
	{
		iSum += (px[k]*px[k])>>nQ;
	}
	
	return iSum;
}


/* 求取s32ParamA/s32ParamB */  
unsigned int CalcInverse(unsigned int s32ParamA,unsigned short s32ParamB, short s16QB)
{
	unsigned int i32Result;
	if( 0 == s32ParamA)
	{
		return 0;
	}

	i32Result = I32MultI16(s32ParamA, s32ParamB);
	return i32Result;
}

int xcorr(short* px, short* py, int inLen,unsigned short us16Thresh)
{
	int nOverLapLen;                  // 重合长度
	int x,y;

	int i32CrossCorr;                 // 互相关值

	int k;
	int index = 0;

	short  nQx, nQy, nQxy;            // sum(xi*yj), sum(xi^2), sum(yj^2)的左移值

	unsigned int i32Denomx,i32Denomy; // sum(xi^2), sum(yj^2) 
	unsigned short s16Denom;          // 1/sqrt(sum(xi^2)*sum(yj^2)) 通过查表获得
	short  nDQ;                       // sum(xi^2), sum(yj^2)  Q值

	unsigned int i32Result;

	short nQ;
	unsigned short sqrt_2 = 46341;   // Q15
	/* 求sum(xi*yj), sum(xi^2), sum(yj^2)的左移值 */
	//GetQ(px,py,inLen,&nQxy,&nQx,&nQy);
	nQx = ORIGINAL_AUDIO_LEFTNUM;
	nQy = RECORD_AUDIO_LEFTNUM;
	nQxy = MOLE_LEFTNUM;
	if(0 != (nQx+nQy)%2)
	{
		nQ = (nQx+nQy+1)/2 - nQxy;
	}
	else
	{
		nQ = (nQx+nQy)/2 - nQxy;
	}
	/* 求sum(xi*yj), sum(xi^2) */
	i32Denomx = GetDenomValue(px,inLen,nQx);
	i32Denomy = GetDenomValue(py,inLen,nQy);

	/* 查表求1/sqrt(sum(yj^2)*sum(xi^2))和对应的Q值 */
	s16Denom = GetDenomValueByTable(i32Denomx,i32Denomy,&nDQ);

	/* 计算sum(xi*yj) */
	index = 0;
	/* 重合长度从1 --> inLen*/
	for(nOverLapLen = 1; nOverLapLen <= inLen; nOverLapLen++ )
	{
		i32CrossCorr = 0;
		for(k = 0;k < nOverLapLen;k++)
		{
			x = px[k];
			y = py[inLen-nOverLapLen+k];

			i32CrossCorr += ((x*y)>>nQxy);
		}	
		if(i32CrossCorr < 0)  /* 负数全部归零 */
		{
			i32CrossCorr = 0;
			i32Result = 0;
			continue;
		}

		i32Result = CalcInverse(i32CrossCorr,s16Denom,nDQ);
		if(0 == (nQx+nQy)%2)
		{			
			i32Result = (i32Result >>(16-nDQ+nQ));
		}
		else
		{
			i32Result = (I32MultI16(i32Result,sqrt_2) >> (15-nDQ+nQ));
		}
		if(i32Result > 32767)
			i32Result  =  32767;

		if(i32Result>us16Thresh)
		{
			return i32Result;
		}

	}
	/* 重合长度从inLen-1 --> 1 */
	for(nOverLapLen = inLen-1;nOverLapLen>=1;nOverLapLen--)
	{
		i32CrossCorr = 0;
		for(k = 0;k < nOverLapLen;k++)
		{
			y = py[k];
			x = px[inLen-nOverLapLen+k];
			i32CrossCorr += ((x*y)>>nQxy);
		}

		if(i32CrossCorr < 0)  /* 负数全部归零 */
		{
			i32CrossCorr = 0;
			i32Result = 0;	
			continue;
		}	

		i32Result = CalcInverse(i32CrossCorr,s16Denom,nDQ);
		if(0 == (nQx+nQy)%2)
		{			
			i32Result = (i32Result >> (16-nDQ+nQ));
		}
		else
		{
			i32Result = (I32MultI16(i32Result,sqrt_2) >> (15-nDQ+nQ));
		}
		if(i32Result > 32767)
			i32Result  =  32767;

		if(i32Result>us16Thresh)
		{
			return i32Result;
		}
	}
	
	return i32Result;
}

int CheckMicTable(int num)
{
	int ret = 0, j = 0;
	for(j = 0; j < 5; j++)
	{
		if(num == g_mic_table[j])
		{
			return j + 1;
		}
	}
    return ret;
}

int* recordTestWr(int audio_data[], int audio_length)
{	
	int *buf;  
	static int ccids[13];
    
	int signal = 0;
	int i = 0, ret = 0;
	int rlength = audio_length / 10;
	int pcmIndex[12];
	int *pcmInput[12];

	printf("start to check audio , recordtest!!!\n");
	buf = audio_data;
	memset(pcmIndex, 0, sizeof(pcmIndex));
	memset(ccids, 0, sizeof(ccids));
	if(NULL == buf) 
	{  
		return NULL;
	}
	
	for(i = 0; i <12; i++)
	{
		pcmInput[i] = malloc(sizeof(int) * rlength);
		if(pcmInput[i] == NULL)
		{
			printf("pcmInput malloc failed \n");
			return NULL;
		}
	}
	
	printf("split audio begin\n");
	while(i < audio_length)
	{
        signal = (buf[i]>>8)&15;
		if(signal > 0 && signal <= 12)
        {
			signal--;
			if(pcmIndex[signal] < rlength)
			{	
                *(pcmInput[signal] + pcmIndex[signal]) = buf[i];
			    pcmIndex[signal] ++;
			}	
        }
		else 
		{
            printf("signal : %d , less than 1 or larger than 12 \n", signal);
		}	
		i++;
	}
	printf("split audio success\n");

	// 检查标号
	for(i = 0; i < 12; i++)
	{
		ret = check_index(i, pcmInput[i], pcmIndex[i]);
		if(ret < 0)
		{
			printf("check index fail\n");
			if(ret != 0) 
			{
				ccids[i] = 1;
			}	
		}
	 }

	 // 检查一致性
	for(i = 0; i < 12; i++)
	{
		ret = check_agreement(i, pcmInput[i], pcmIndex[i]);
		if(ret < 0)
		{
			
			if(ret != 0) 
			{
				printf("check agreement fail\n");
				ccids[i] = 1;
			}else {
				printf("check agreement success\n");
			}	
				
		}
	}

	for(i = 0; i <12; i++)
	{
		free(pcmInput[i]);
	}

	return ccids;
}

// 检查音频文件中标号
static int check_index(int signal, int input[], int input_length)
{
	int i = 0, j =0;
	int buf;
	int channel_index, first_index;

	// 跳过最初的20个字节
	i = 20;
	first_index = (input[i] >> 8) & 0xf;
	i++;
	while(i < input_length)
	{
		channel_index = (input[i] >> 8) & 0xf;
		if(channel_index != first_index)
		{
			printf("check index fail, lost data, file index is %d, first_index is %d, channel_index is %d", signal, first_index, channel_index);
			return -12;
		}
		i++;
	}

	printf("check index success, channel_index is %d\n", channel_index);

	return 0;
}

// 检查一致性
static int check_agreement(int signal, int input[], int input_length)
{
	int i = 0, j = 0, offset = 0;
	printf("start to check agreement, signal is %d\n", signal);
	if(input == NULL || input_length < 40000)
	{
		printf("error in input pcm, content : %d , length less than one frame\n", input_length);
		return -21;
	}

	int buf[256]; // 音频文件中的原始数据
	short buf_shift[256]; // 移位以后的数据
	unsigned long pass_count = 0; // 通过的次数
	unsigned long total_count = 0; // 总的次数
	int pass_rate = 0;
	int first_index = 0;

	// 跳过最初的20个字节
	i = 20;

	// 读取文件的第一个sample，取其index最为比较的标准
	//fread(&first_index_in_file, sizeof(int), 1, fp_out[file_index]);
    first_index = input[i];
    i ++;
	first_index = (first_index >> 8) & 0xf;
	//if(first_index == 0x06)
	//{
	//	printf("check pass");
	//	return 0;
	//}
	
	//fseek(fp_out[file_index], 64000, SEEK_SET); // 跳过开始的1s
	//fseek(fp_out[file_index], 10240, SEEK_SET); //跳过0.08s
    i = i + 10240;

    while( i < input_length && input_length - i > 256)
    {
        total_count++;
		for(j = i, offset = 0; offset < 256; j++, offset++)
		{
			buf_shift[offset] = (input[j] >> 16) & 0xffff;
		}
        i = i + 256;
		
		if(xcorr(buf_shift, rec_400hz, 256, 20480) >= THRES_REC_FREQ)
		{
			printf("check agreemnt, %d success\n", pass_count);
			pass_count++;			
		}else {
			printf("check agreemnt, %d failed\n", pass_count);
		}
    }	

	pass_rate = pass_count * 100 / total_count;
	printf("signal : %d, pass_count is %d, total_count is %d, pass_rate is %d\n", signal, pass_count, total_count, pass_rate);
	if(pass_rate < THRES_REC_PASS_RATE)
	{
		//printf("check agreement fail");
		return -1;
	}
	return 0;
}
