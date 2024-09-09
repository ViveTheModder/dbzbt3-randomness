#include <stdio.h>
#define CHARA_COUNT 161
#define MAP_COUNT 35
#define BGM_COUNT 24

//to clarify, these global variables have their endians (order of bytes) changed, so for example:
//0x271 is how it looks like for the line of code, but in assembly, it's more like 0x7102 (instruction: 71 02 02 24, load immediate)

unsigned int SomeKindOfTimer = 0x271; //(ADDR: 0x202FF060)
// starts from 1 to 2 (in title screen), and increases until going to Character Select, then resets when approaching 625 
// the way it increases is unknown, but is likely based on the number of actions (e.g. background sprites moving, voice lines etc.)
// so, during a match for example, based on its value, it will increase and reset continuously, until the match is over
// sample values (in hex, and they change REALLY fast, so this is at 5% speed): 00, 08, 10, 18, 20, 28... (increases by 8, then 16)

unsigned long int const1 = 0x9908B0DF00000000; //0x00000000DFB00899
unsigned int const2 = 0x701B748D; //0x8D741B70 - (ADDR: 0x2031E1E0)
unsigned int seed1 = 0xB6368E52; //0x528E36B6 - (ADDR: 0x2031DBB0)
unsigned int seed2 = 0x2B0033A4; //0xA433002B - (ADDR: 0x2031E56C)
//now, for the seeds themselves, although the output is kind of random, and not really dependent on the previous output like the action timer,
//it is still very much pre-determined, because whether you wait during a match or a menu, you will get the following exact values for sure
//the first two values always correspond to the initial bootup and going to the title screen
//it is unknown if at some point these values reset or not, but just know they change every 2-3 seconds
/* [seed1 values - 40]
00000080,528E36B6,9F356B78,B0EA4AD2,868B6A27,E700F0DB,026A7427,4A0F0CD4,0DE99D29,082DE3E6,CA27BF8E,31BAADB3,3DDDE638,54EC5641,E6E1D4D9,68E0E587,
F80EA1A9,04393E87,130FBB27,24B21DC2,D5892713,50574960,4C93CDCF,B5D236B0,7C13140C,F101427A,8B801E6B,78B1F541,0F250DE8,6BF468B4,9F671C09,ADD24A70,
1A1EA05C,76BB75C9,AF271B07,BDAD707A,FB2AB3FD,6E19D1BD,5E634AA3,49FC3897
[seed2 values - 51]
99DECB87,A433002B,9D5E26F5,D5013E19,4E31AE63,21C4EBC4,8C41A123,28731D5A,D430DDFD,09E26A03,68E74477,4A19C5F0,CCAA60B5,702B010,7E04F15C,B984E973,
F1F3CB64,0F688473,DB43CEFA,9FAD1511,B80080E0,25DF3F00,2BBDFD57,08AD08F1,337D1136,AAC432D4,17849C9C,31ACE110,D55615DD,67BE27B3,9DF79DAD,04AD3691,
807B4B1E,36A9A154,85579EEA,8C922B94,2F51CA97,3AF8F32E,2245BE61,76BA8FE2,159FAE3E,8EA937CD,E2F4578F,EDE01D33,ABB4D5F0,E894D423,5F37A66C,F659724C,
0977BEE0,D1D69A00,AB749AC4
*/

unsigned int zero = 0; //(ADDR: 0x2031ABAC)
//function prototypes
unsigned long int GetOriginalRNG();
unsigned int GetLimitedRNG(long rng, int limit);
void AssistRNG(unsigned int param);

int main() 
{
    unsigned long int rng = GetOriginalRNG();
    printf("RNG: %i\n",rng);
	printf("Random Character ID: %i\n",GetLimitedRNG(rng,CHARA_COUNT));
	printf("Random Map ID: %i\n",GetLimitedRNG(rng,MAP_COUNT));
	printf("Random BGM ID: %i\n",GetLimitedRNG(rng,BGM_COUNT));

    return 0;
}

unsigned int GetLimitedRNG(long rng, int limit)
{
	if (limit==0) return 0; //original instruction: trap(7);
	return rng%limit;
}
unsigned long int GetOriginalRNG(void)
{
  unsigned int result1;
  unsigned long int result2;
  unsigned int *PTR;
  int DecreasingCount;
  
  if (0x26f < SomeKindOfTimer) { //check if SomeKindOfTimer is greater than 623 (this is always true)
    if (SomeKindOfTimer == 0x271) { //check if SomeKindOfTimer is 625 (also true, given how it's initialized)
      AssistRNG(0x1571);
    }
    PTR = &seed1;
    DecreasingCount = 0xe2;
    do {
      DecreasingCount = DecreasingCount + -1;
      *PTR = PTR[0x18d] ^ (*PTR & 0x80000000 | PTR[1] & 0x7fffffff) >> 1 ^ *(unsigned int *)(&const1 + (PTR[1] & 1) * 4);
      PTR = PTR + 1;
    } while (DecreasingCount > -1);
    SomeKindOfTimer = 0;
    seed2 = const2 ^ (seed2 & 0x80000000 | seed1 & 0x7fffffff) >> 1 ^ *(unsigned int *)(&const1 + (seed1 & 1) * 4);
  }
  DecreasingCount = SomeKindOfTimer;
  SomeKindOfTimer++;
  result1 = (&seed1)[DecreasingCount] ^ (&seed1)[DecreasingCount] >> 0xb;
  result2 = (long)(int)result1 ^ (long)(int)(result1 << 7) & 0xffffffff9d2c5680U;
  result2 = result2 ^ (long)((int)result2 << 0xf) & 0xffffffffefc60000U;
  return result2 ^ (long)(int)((unsigned int)result2 >> 0x12);
}
void AssistRNG(unsigned int param)
{
  int ActionCount; 
  ActionCount = 1;
  seed1 = param;
  do {
    SomeKindOfTimer = ActionCount + 1;
    (&seed1)[ActionCount] =
         ((&zero)[ActionCount] ^ (unsigned int)(&zero)[ActionCount] >> 0x1e) * 0x6c078965 + ActionCount;
    ActionCount = SomeKindOfTimer;
  } while (SomeKindOfTimer < 0x270); //less than 624
  return;
}