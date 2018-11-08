#include <Arduino.h>

void AE_HX711_Init(void);
void AE_HX711_Reset(void);
long AE_HX711_Read(void);
long AE_HX711_Averaging(long adc, char num);
float AE_HX711_getGram(char num);

//---------------------------------------------------//
// ロードセル　シングルポイント（ ビーム型）　ＳＣ６０１　１２０ｋＧ [P-12035]
//---------------------------------------------------//
#define OUT_VOL   0.001f      //定格出力 [V]
#define LOAD      120000.0f   //定格容量 [g]

#define CONNECTED_SENSOR_NUM 2

struct LoadcellPin {
	int dout;
	int slk;
};

// ピンの設定
struct LoadcellPin pins[CONNECTED_SENSOR_NUM] =
{
	{ 2, 3 },
	{ 8, 9 }
};

float offset;

void setup() {

	Serial.begin(9600);
	Serial.println("AE_HX711 test");
	AE_HX711_Init();
	AE_HX711_Reset();
	offset = AE_HX711_getGram(30);
}

void loop()
{
	float data;
	char S1[20];
	char s[20];
	data = AE_HX711_getGram(5);
	sprintf(S1, "%s [g] (0x%4x)", dtostrf((data - offset), 5, 3, s), AE_HX711_Read());
	Serial.println(S1);
}

void AE_HX711_Init(void)
{
	for (int i = 0; i < CONNECTED_SENSOR_NUM; i++) {
		pinMode(pins[i].slk, OUTPUT);
		pinMode(pins[i].dout, INPUT);
	}
}

void AE_HX711_Reset(void)
{
	for (int i = 0; i < CONNECTED_SENSOR_NUM; i++) {
		digitalWrite(pins[i].slk, 1);
		delayMicroseconds(100);
		digitalWrite(pins[i].slk, 0);
		delayMicroseconds(100);
	}
}

long AE_HX711_Read(void)
{
	long data = 0;
	while (digitalRead(pins[0].dout) != 0);
	delayMicroseconds(10);
	for (int i = 0; i < 24; i++)
	{
		digitalWrite(pins[0].slk, 1);
		delayMicroseconds(5);
		digitalWrite(pins[0].slk, 0);
		delayMicroseconds(5);
		data = (data << 1) | (digitalRead(pins[0].dout));
	}
	//Serial.println(data,HEX);
	digitalWrite(pins[0].slk, 1);
	delayMicroseconds(10);
	digitalWrite(pins[0].slk, 0);
	delayMicroseconds(10);
	return data ^ 0x800000;
}

long AE_HX711_Averaging(long adc, char num)
{
	long sum = 0;
	for (int i = 0; i < num; i++) sum += AE_HX711_Read();
	return sum / num;
}

float AE_HX711_getGram(char num)
{
#define HX711_R1  20000.0f
#define HX711_R2  8200.0f
#define HX711_VBG 1.25f
#define HX711_AVDD      4.2987f//(HX711_VBG*((HX711_R1+HX711_R2)/HX711_R2))
#define HX711_ADC1bit   HX711_AVDD/16777216 //16777216=(2^24)
#define HX711_PGA 128
#define HX711_SCALE     (OUT_VOL * HX711_AVDD / LOAD *HX711_PGA)

	float data;

	data = AE_HX711_Averaging(AE_HX711_Read(), num)*HX711_ADC1bit;
	//Serial.println( HX711_AVDD);   
	//Serial.println( HX711_ADC1bit);   
	//Serial.println( HX711_SCALE);   
	//Serial.println( data);   
	data = data / HX711_SCALE;


	return data;
}


