/*********************************
*                                *
* Firmware del Indoor MATIC V0.1 *
*                                *
*********************************/

// Incluye Librerias
#include <Arduino.h>
#include <LiquidCrystal.h> // lcd de 16 x 2
#include <DHT.h> // sensor de temp y humedad


/**************************************************
* C O N S T A N T E S - Sensores conectados al IM *
*                                                 *
* Comentar los sensores que no estén presentes    *
**************************************************/
#define DHT_TYPE           DHT21    // DHT 22  (AM2302)
//#define DHT_TYPE           DHT22    // DHT 22  (AM2302)
#define DHT_PIN_0          2        // pin al que va el cable de dato del sensor0 de temperatura
//#define DHT_PIN_1          3      // pin al que va el cable de dato del sensor1 de temperatura
#define LIGHT_SENSOR_PIN_0 A0       //PARA EL DE PEDRO
//#define LIGHT_SENSOR_PIN_0 A2       //define el pin para el Photo-resistor
//#define LIGHT_SENSOR_PIN_1 A0     //define el pin para el Photo-resistor
//#define SOIL_PIN_0         A1       //define el pin para sensor de humedad de tierra
//#define SOIL_PIN_1         A3     //define el pin para sensor de humedad de tierra

/*******************************************
 * C O N S T A N T E S- Textos para el lcd *
 ******************************************/
#define T_s0	  "Temp s0: "
#define T_s1	  "Temp s1: "
#define H_s0    " Hum s0: "
#define H_s1		" Hum s1: "
#define L_s0	  " Luz s0: "
#define L_s1		" Luz s1: "
#define S_s0    "Suelo 0: "
#define S_s1	  "Suelo 1: "
#define P	   	  " %"
#define C	      " C"
#define L			  " ^"

/**************************************************
 * C O N S T A N T E S - Relativas al Dispositivo *
 *************************************************/
//const String  dispositivo = "e4da3b7fbbce2345d7772b0674a318d5"; /*5*/  //nombre del dispositivo Importante cambiarlo por cada dispositivo
const String  dispositivo = "TEST";
const int     numeroSerie = 3;
//tiempo entre envío a la DB
//const long  intervalo = 360000; //constante de espera para mandar el GET
const long intervalo = 60000; //constante de espera para mandar el GET mas corto para TEST

/********************
* V A R I A B L E S *
********************/
unsigned long previousMillis = 0; //variable que va a guardar el valor de tiempo anterior para compararlo con el actual
unsigned long currentMillis; //varialble que guardará el valor de tiempo actual

int     debug = 2;

float   hum_0 = 999;  // humedad del sensor 0
float   hum_1 = 999;  // humedad del sensor 1
float   temp_0 = 999;  // temperatura del sensor 0
float   temp_1 = 999;  // temperatura del sensor 0

int     luz_0 = 999; //valor analógico del pin
int     luz_1 = 999; //valor analógico del pin
int     valorLuz_0 = 999; //valor mapeado
int     valorLuz_1 = 999; //valor mapeado

int     suelo_0 = 999; //valor analogico del pin
int     suelo_1 = 999; //valor analogico del pin
float   valorSuelo_0 = 999; //valor mapeado
float   valorSuelo_1 = 999; //valor mapeado

int     i = 0;


String stringDelSerial = "";         // a string to hold incoming data

boolean stringCompleta  = false;  // whether the string is complete
boolean online          = false;  // si esta ONLINE
boolean exito           = false;  //Si la web devolvio exito

/****************************************************
* Construct de librerias del LCD y los sensores DHT *
****************************************************/
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LiquidCrystal lcd(13, 12, 11, 10, 9, 8); //inicializa la libreria con los numeros de pines utilizados

#ifdef DHT_PIN_0
  DHT dht_0(DHT_PIN_0, DHT_TYPE); //inicializa la libreria para el pin
#endif

#ifdef DHT_PIN_1
  DHT dht_1(DHT_PIN_1, DHT_TYPE);
#endif

/*********************************************
 * S E T U P - Cosas que corren una sola vez *
 ********************************************/
void setup()
{
  Serial.begin(115200); // inicia comunicacion serie a 115.200 baudios
  //Serial.setTimeout(1000); // define el tiempo de espera de la comunicacion serial
  lcd.begin(16,2); // inicia el lcd y setea el numero de columnas y de filas del lcd

  #ifdef DHT_PIN_0
    dht_0.begin(); // inicia el sensor0 AM2301
  #endif

  #ifdef DHT_PIN_1
    dht_1.begin(); // inicia el sensor1 AM2301
  #endif

  lcdBienvenida();
}

/*******************************************
 * L O O P - Cosas que corren para siempre *
 ******************************************/
void loop()
{
  escuchaSerial();
  analizaComando(stringDelSerial);
  if (!online)
  {
    if (debug > 1)
    {
      Serial.print("\n");
      Serial.println(F("loop - > if(!online)"));
    }

    i++;
    lcdOFFLINE();
    estaConectado();
    if ( i % 5 == 0)
    {
      //leeSensores();
      lcdSensores();
    }

  }
  else
  {
    //leeSensores();
    lcdSensores();
    lcdEnviandoDatos();
    enviaDatos();
  }
}


/************************************************************
 * F U N C I O N E S - Cosas que se usan en Setup y en Loop *
 ***********************************************************/

// Genera un string a enviar
void enviaDatos()
{
  if (!online)
  {
    return;
  }
  currentMillis = millis(); //asigna el valor de millis() a la variable currentMillis

  if (previousMillis > currentMillis) // si previousMillis es mayor a currentMillis quiere decir que millis() volvio a cero porque se lleno entonces vuelvo tambien a cero previousMillis
  {
    previousMillis = 0;
  }

  if (currentMillis - previousMillis >= intervalo) //cuando la diferencia entre previousMillis y currentMillis es mayor o igual al intervalo se envian los datos
  {
    previousMillis = currentMillis;
    String GET = "<?1a1dc91c907325c69271ddf0c944bc72";
    GET += "&disp=";
    GET += dispositivo;
    GET += "&t0=";
    GET += temp_0;
    GET += "&h0=";
    GET += hum_0;
    GET += "&t1=";
    GET += temp_1;
    GET += "&h1=";
    GET += hum_1;
    GET += "&luz=";
    GET += valorLuz_0;
    GET += "&luz1=";
    GET += valorLuz_1;
    GET += "&ht=";
    GET += valorSuelo_0;
    GET += "&ht1=";
    GET += valorSuelo_1;
    GET += "&debug=";
    GET += debug;
    GET += ">";

    Serial.print(GET);
    delay(100);

    /*if (Serial.find("EXITO"))
    {
      lcdExito();
    }*/
  }
}

// Pregunta al Wifi si esta conectado y busca en el buffer Serial CONECTADO_OK para pasar a true la variable online
void estaConectado ()
{
  //pregunta al ESP si esta conectato
  delay(1000);
  Serial.print("[ESP_status]");
  //Si llega un OK
  /*if (Serial.find("CONECTADO_OK"))
  {
    online = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("- ONLINE -    ");
    delay(3000);
    if (debug > 1)
    {
      Serial.println(F("DBG_esp8266_CONECTADO"));
    }
  }
  else
  {
    online = false;
    if (debug > 1)
    {
      Serial.println(F("esp8266 OFFLINE"));
    }
  }
  */
}

// lee los sensores y guarda los valores en sus variables.
void leeSensores()
{
  #ifdef DHT_PIN_0
    hum_0        = dht_0.readHumidity();
    //delay(10);
    temp_0       = dht_0.readTemperature();
    //delay(10);
    if (isnan(hum_0) || isnan(temp_0) )
    {
      delay(3000);
      hum_0        = dht_0.readHumidity();
      //delay(10);
      temp_0       = dht_0.readTemperature();
      //delay(10);
    }
  #endif

  #ifdef DHT_PIN_1
    hum_1        = dht_1.readHumidity();
    //delay(10);
    temp_1       = dht_1.readTemperature();
    //delay(10);
    if (isnan(hum_0) || isnan(temp_0) )
    {
      delay(3000);
      hum_1        = dht_0.readHumidity();
      //delay(10);
      temp_1       = dht_0.readTemperature();
      //delay(10);
    }
  #endif

  #ifdef LIGHT_SENSOR_PIN_0
    luz_0        = analogRead(LIGHT_SENSOR_PIN_0);
    valorLuz_0   = map(luz_0,0,1024,0,100);
    //delay(10);
  #endif

  #ifdef LIGHT_SENSOR_PIN_1
    luz_1        = analogRead(LIGHT_SENSOR_PIN_1);
    valorLuz_1   = map(luz_1,0,1024,0,100);
    //delay(10);
  #endif

  #ifdef SOIL_PIN_0
    suelo_0      = analogRead(SOIL_PIN_0);
    valorSuelo_0 = map(suelo_0,0,1024,100,0);
    //delay(10);
  #endif

  #ifdef SOIL_PIN_1
    suelo_1      = analogRead(SOIL_PIN_1);
    valorSuelo_1 = map(suelo_1,0,1024,100,0);
    //delay(10);
  #endif

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

/******************************/
/* F U N C I O N E S   L C D  */
/******************************/
// Funcion que escribe dos renglones en la pantalla segun los parametros.
void lcdSensores( )
{
  if( debug > 1)
  {
    Serial.print("\n");
    Serial.println(F("lcdSensores delay 3000ms por gurpo de sensor"));
  }
  i = 0;

  #ifdef DHT_PIN_0
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(T_s0);
    lcd.print(temp_0);
    lcd.print(C);
    lcd.setCursor(0, 1);
    lcd.print(H_s0);
    lcd.print(hum_0);
    lcd.print(P);
    delay(3000);
  #endif

  #ifdef DHT_PIN_1
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(T_s1);
    lcd.print(temp_1);
    lcd.print(C);
    lcd.setCursor(0, 1);
    lcd.print(H_s1);
    lcd.print(hum_1);
    lcd.print(P);
    delay(3000);
  #endif

  #ifdef LIGHT_SENSOR_PIN_0
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(L_s0);
    lcd.print(valorLuz_0);
    lcd.print(L);
      #ifdef LIGHT_SENSOR_PIN_1
      lcd.setCursor(0, 1);
      lcd.print(L_s1);
      lcd.print(valorLuz_1);
      lcd.print(L);
      #endif
    delay(3000);
  #endif

  #ifdef SOIL_PIN_0
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(S_s0);
    lcd.print(valorSuelo_0);
      #ifdef SOIL_PIN_1
        lcd.setCursor(0, 1);
        lcd.print(S_s1);
        lcd.print(valorSuelo_1);
        #endif
    delay(3000);
  #endif
  //delay(500);
}

// escribe offline en el LCD
void lcdOFFLINE()
{
  if( debug > 1)
  {
    Serial.print("\n");
    Serial.println(F("lcdOFFLINE delay 500ms"));
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("- OFFLINE -  ");
  lcd.print(i);
  if (i % 2 == 0)
  {
    lcd.setCursor(0, 1);
    lcd.print("  Conectate al ");
  }
  else
  {
    lcd.setCursor(0, 1);
    lcd.print("WiFi IndoorMatic");
  }
  delay(500);
}

// escribe enviando datoss en el LCD
void lcdEnviandoDatos()
{
  if( debug > 1)
  {
    Serial.print("\n");
    Serial.println(F("lcdEnviandoDatos delay 500ms"));
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("E n v i a n d o ");
  lcd.print(i);
  lcd.setCursor(0, 1);
  //lcd.print("----------------");
  lcd.print("   D a t o s    ");
  delay(500);
}

// escibe el texto de bienvenida para el Setup
void lcdBienvenida()
{
  if( debug > 1)
  {
    Serial.print("\n");
    Serial.println(F("lcdBienvenida delay 7500ms"));
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("www.IndoorMatic ");
  lcd.print(i);
  lcd.setCursor(0, 1);
  lcd.print("         .com.ar");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BIENVENIDO");
  lcd.print(i);
  lcd.setCursor(0, 1);
  lcd.print("Num. Serie: ");
  lcd.print(numeroSerie);
  delay(2500);
}

void lcdExito()
{
  if( debug > 1)
  {
    Serial.print("\n");
    Serial.println(F("lcdExito delay 1000ms"));
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   D a t o s    ");
  lcd.setCursor(0, 1);
  lcd.print("Recibidos  O.K.");
  delay(1000);
}

/***********************************
 * F U N C I O N E S   S E R I A L *
/**********************************/
void escuchaSerial()
{
  if (Serial.available())
  {
    stringDelSerial = Serial.readString();
  }
  stringCompleta = true;
  if(debug > 1)
  {
    Serial.print("\n");
    Serial.print("string del Serial = ");
    Serial.println(stringDelSerial);
  }
}

void analizaComando( String comando )
{
  if (stringCompleta)
  {
    stringCompleta = false;
    if (comando.equals("CONECTADO_OK"))
    {
      online = true;
    }
    if (comando.equals("EXITO"))
    {
      exito = true;
    }
  }
  stringDelSerial = "";
}
