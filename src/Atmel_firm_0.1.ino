// Firmware del Indoor MATIC 0.1

// Incluye Librerias
#include <Arduino.h>
#include <LiquidCrystal.h> // lcd de 16 x 2
#include <DHT.h> // sensor de temp y humedad

// Constantes
#define DHT_PIN_0          2        // pin al que va el cable de dato del sensor0 de temperatura
#define DHT_PIN_1          3        // pin al que va el cable de dato del sensor1 de temperatura
#define DHT_TYPE           DHT22    // DHT 22  (AM2302)
#define LIGHT_SENSOR_PIN_0 A2       //int     lightPin0 = A2;  //define el pin para el Photo-resistor
#define LIGHT_SENSOR_PIN_1 A0       //int     lightPin1 = A0;  //define el pin para el Photo-resistor
#define SOIL_PIN_0         A1       //int     soilPin0 = A1;  //define el pin para sensor de humedad de tierra
#define SOIL_PIN_1         A3       //int     soilPin1 = A3;  //define el pin para sensor de humedad de tierra

// Pantalla LCD
const String T_s0   = "Temp s0: ";
const String T_s1	  = "Temp s1: ";
const String H_s0   = " Hum s0: ";
const String H_s1		= " Hum s1: ";
const String L_s0	  = "Luz s0: ";
const String L_s1		= "Luz s1: ";
const String S_s0   = "H Suelo0: ";
const String S_s1	  = "H Suelo0: ";
const String P	   	= "%";
const String C	    = "C";
const String L			= "^";
const String ESP			= " ";

// Dispositivo
const String  dispositivo = "e4da3b7fbbce2345d7772b0674a318d5"; /*5*/  //nombre del dispositivo Importante cambiarlo por cada dispositivo
//const String  dispositivo = "TEST";
const long  intervalo = 360000; //constante de espera para mandar el GET

int     debug = 0;
float   hum_0;  // humedad del sensor 0
float   hum_1;  // humedad del sensor 1
float   temp_0;  // temperatura del sensor 0
float   temp_1;  // temperatura del sensor 0

int     luz_0; //valor analógico del pin
int     luz_1; //valor analógico del pin
int     valorLuz_0; //valor mapeado
int     valorLuz_1; //valor mapeado

int     suelo_0; //valor analogico del pin
int     suelo_1; //valor analogico del pin
float   valorSuelo_0; //valor mapeado
float   valorSuelo_1; //valor mapeado

int     i = 0;

boolean online = false;

// Construct de librerias del LCD y los sensores DHT
LiquidCrystal lcd(13, 12, 11, 10, 9, 8); //inicializa la libreria con los numeros de pines utilizados
DHT dht_0(DHT_PIN_0, DHT_TYPE); //inicializa la libreria para el pin
DHT dht_1(DHT_PIN_1, DHT_TYPE);

void setup()
{
  Serial.begin(115200); // inicia comunicacion serie a 115.200 baudios
  Serial.setTimeout(1000); // define el tiempo de espera de la comunicacion serial
  lcd.begin(16,2); // inicia el lcd y setea el numero de columnas y de filas del lcd
  dht_0.begin(); // inicia el sensor0 AM2301
  dht_1.begin(); // inicia el sensor1 AM2301
}

void loop()
{
  if (!online)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  - OFFLINE - ");
    lcd.setCursor(0, 1);
    lcd.print(i);
    i++;
    estaConecado();
    if (debug > 1)
    {
      Serial.println(F("Estamos OFFLINE (loop): "));
    }
  }

  //leeSensores();
  /*renglon_1, numero_1, renglon_1_unidad, renglon_2, float numero_2, String renglon_2_unidad*/
  pantalla(T_s0, i, C, T_s1, i+1, L);
  Serial.print(F("Estatus Conexion: "));
  Serial.println(online);
  delay(1000);
}



/*
 * FUNCIONES
 */

// busca en el buffer Serial CONECTADO_OK
void estaConecado ()
{
  if (Serial.find("CONECTADO_OK"))
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    - ONLINE -   ");
    if (debug > 1)
    {
      Serial.println(F("esp8266 CONECTADO"));
    }
    online = true;
  }
  else
  {
    if (debug > 1)
    {
      Serial.println(F("esp8266 OFFLINE"));
    }
    online = false;
  }
}

//lee los sensores y guarda los valores
void leeSensores()
{
	hum_0        = dht_0.readHumidity();
  temp_0       = dht_0.readTemperature();
  hum_1        = dht_1.readHumidity();
  temp_1       = dht_1.readTemperature();
  luz_0        = analogRead(LIGHT_SENSOR_PIN_0);
  valorLuz_0   = map(luz_0,0,1024,0,100);
  luz_1        = analogRead(LIGHT_SENSOR_PIN_1);
  valorLuz_1   = map(luz_1,0,1024,0,100);
  suelo_0      = analogRead(SOIL_PIN_0);
  valorSuelo_0 = map(suelo_0,0,1024,100,0);
  suelo_1      = analogRead(SOIL_PIN_1);
  valorSuelo_1 = map(suelo_1,0,1024,100,0);

  if (debug > 1)
  {
    Serial.print(F("Sensor H_0 "));
    Serial.println(hum_0);
    Serial.print(F("Sensor H_1 "));
    Serial.println(hum_1);
    Serial.print(F("Sensor T_0 "));
    Serial.println(temp_0);
    Serial.print(F("Sensor T_1 "));
    Serial.println(temp_1);
    Serial.print(F("Sensor L_0 "));
    Serial.println(valorLuz_0);
    Serial.print(F("Sensor L_1 "));
    Serial.println(valorLuz_1);
    Serial.print(F("Sensor S_0 "));
    Serial.println(valorSuelo_0);
    Serial.print(F("Sensor S_1 "));
    Serial.println(valorSuelo_1);
  }
}



void pantalla( String renglon_1, float numero_1, String renglon_1_unidad, String renglon_2, float numero_2, String renglon_2_unidad )
{
	lcd.clear();					//limpia la pantalla
    lcd.setCursor(0, 0);			//se posiciona en la columna 0 fila 0
    lcd.print(renglon_1);			//imprime el string del renglon 1
    lcd.print(numero_1);			//imprime el primer dato
    lcd.print(renglon_1_unidad);	//imprime la unidad del dato
    lcd.setCursor(0, 1);			//se posiciona en la columna 0 fila 1
    lcd.print(renglon_2);			//imprime el string del renglon 2
    lcd.print(numero_2);			//imprime el segundo dato
    lcd.print(renglon_2_unidad);	//imprime la unidad del dato
    delay(3500);					//espera 3.5 segundos
}
