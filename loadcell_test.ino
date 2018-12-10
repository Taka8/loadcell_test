#include <Arduino.h>

void AE_HX711_Init(void);
void AE_HX711_Reset(void);
long AE_HX711_Read(int pin_num);
long AE_HX711_Averaging(int pin_num, char num);
float AE_HX711_getGram(int pin_num, char num);

//---------------------------------------------------//
// ロードセル　シングルポイント（ ビーム型）　ＳＣ６０１　１２０ｋＧ [P-12035]
//---------------------------------------------------//
#define OUT_VOL   0.001f      //定格出力 [V]
#define LOAD      120000.0f   //定格容量 [g]

#define CONNECTED_SENSOR_NUM 4

struct LoadcellPin {
	int dout;
	int slk;
};

// ピンの設定
struct LoadcellPin pins[CONNECTED_SENSOR_NUM] =
{
	{ 2, 3 },			// 1 番目のセンサー
	{ 4, 5 },			// 2 番目のセンサー
	{ 7, 6 },			// 3 番目のセンサー
	{ 8, 9 }			// 4 番目のセンサー
};

float offsets[4];		// オフセット値の値

void setup() {

	Serial.begin(9600);
	Serial.println("AE_HX711 start");
	AE_HX711_Init();
	AE_HX711_Reset();

	// オフセットの作成
	for (int i = 0; i < CONNECTED_SENSOR_NUM; i++) {
		offsets[i] = AE_HX711_getGram(i, 30);
	}
}

void loop()
{
	float data;
	char S1[20];
	char s[20];
	for (int i = 0; i < CONNECTED_SENSOR_NUM; i++) {
		data = AE_HX711_getGram(i, 1);
		sprintf(S1, "%s\t", dtostrf((data - offsets[i]), 5, 3, s));
		Serial.print(S1);
	}
	Serial.println();
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
		delayMicroseconds(1);
		digitalWrite(pins[i].slk, 0);
		delayMicroseconds(1);
	}
}

long AE_HX711_Read(int pin_num)
{
	long data = 0;

	while (digitalRead(pins[pin_num].dout) != 0);
	delayMicroseconds(1);
	for (int i = 0; i < 24; i++)
	{
		digitalWrite(pins[pin_num].slk, 1);
		delayMicroseconds(1);
		digitalWrite(pins[pin_num].slk, 0);
		delayMicroseconds(1);
		data = (data << 1) | (digitalRead(pins[pin_num].dout));
	}

	digitalWrite(pins[pin_num].slk, 1);
	delayMicroseconds(1);
	digitalWrite(pins[pin_num].slk, 0);
	delayMicroseconds(1);
	return data ^ 0x800000;
}

long AE_HX711_Averaging(int pin_num, char num)
{
	long sum = 0;
	for (int i = 0; i < num; i++) sum += AE_HX711_Read(pin_num);
	return sum / num;
}

/**
* 指定センサーのグラム数を取得する関数
* @param pin_num 指定センサーの番号
* @param num 平均化する回数
* @return グラム数
*/
float AE_HX711_getGram(int pin_num, char num)
{
#define HX711_R1  20000.0f
#define HX711_R2  8200.0f
#define HX711_VBG 1.25f
#define HX711_AVDD      4.2987f					//(HX711_VBG*((HX711_R1+HX711_R2)/HX711_R2))
#define HX711_ADC1bit   HX711_AVDD/16777216		//16777216=(2^24)
#define HX711_PGA 128
#define HX711_SCALE     (OUT_VOL * HX711_AVDD / LOAD *HX711_PGA)

	float data;

	data = AE_HX711_Averaging(pin_num, num)*HX711_ADC1bit;
	//Serial.println( HX711_AVDD);
	//Serial.println( HX711_ADC1bit);
	//Serial.println( HX711_SCALE);
	//Serial.println( data);
	data = data / HX711_SCALE;

	return -data;

}


