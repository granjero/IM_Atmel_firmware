/*********************************
*                                *
* Firmware del Indoor MATIC V0.1 *
*                                *
*********************************/

// Incluye Librerias
#include <Arduino.h>
#include <LiquidCrystal.h> // lcd de 16 x 2
#include <DHT.h> // sensor de temp y humedad

/********************************************
 * C O N S T A N T E S - Textos para el lcd *
 *******************************************/
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
 *                                                *
 * C O N S T A N T E S - Relativas al Dispositivo *
 *                                                *
 *************************************************/
//const String  dispositivo = "TEST";
/*quito la constante porque se la paso al esp que va a poner la macadd*/
//const String  dispositivo = "e4da3b7fbbce2345d7772b0674a318d5"; /*5*/  //nombre del dispositivo Importante cambiarlo por cada dispositivo
//const int     numeroSerie = 5;

//tiempos
const long    intervaloDatos      = 360000; //constante de espera para mandar el GET
const long    intervaloSensores   = 2000; //constante de espera para volver a leer los sensores
const long    intervaloRespuesta  = 10000; //constante de espera para dar por finalizado el tiempo de espera

/**************************************************
* C O N S T A N T E S - Sensores conectados al IM *
*                                                 *
* Comentar los sensores que no estén presentes    *
**************************************************/

//#define DHT_TYPE           DHT21    // DHT 22  (AM2302)
#define DHT_TYPE           DHT22    // DHT 22  (AM2302)
#define DHT_PIN_0          2        // pin al que va el cable de dato del sensor0 de temperatura
//#define DHT_PIN_1          3      // pin al que va el cable de dato del sensor1 de temperatura
//#define LIGHT_SENSOR_PIN_0 A0       //PARA EL DE PEDRO
#define LIGHT_SENSOR_PIN_0 A2       //define el pin para el Photo-resistor
//#define LIGHT_SENSOR_PIN_1 A0     //define el pin para el Photo-resistor
#define SOIL_PIN_0         A1       //define el pin para sensor de humedad de tierra
//#define SOIL_PIN_1         A3     //define el pin para sensor de humedad de tierra

/********************
* V A R I A B L E S *
********************/

unsigned long previousMillisEnviaDatos  = 0; //variable que va a guardar el valor de tiempo anterior para compararlo con el actual
unsigned long previousMillisLeeSensores = 0; //variable que va a guardar el valor de tiempo anterior para compararlo con el actual
unsigned long previousMillisRespuesta   = 0; //variable que va a guardar el valor de tiempo anterior para compararlo con el actual

unsigned long currentMillis; //varialble que guardará el valor de tiempo actual

int debug = 0;

float hum_0         = 999;  // humedad del sensor 0
float hum_1         = 999;  // humedad del sensor 1
float temp_0        = 999;  // temperatura del sensor 0
float temp_1        = 999;  // temperatura del sensor 0

int luz_0           = 999;  //valor analógico del pin
int luz_1           = 999;  //valor analógico del pin
int valorLuz_0      = 999;  //valor mapeado
int valorLuz_1      = 999;  //valor mapeado

int suelo_0         = 999;  //valor analogico del pin
int suelo_1         = 999;  //valor analogico del pin
float valorSuelo_0  = 999;  //valor mapeado
float valorSuelo_1  = 999;  //valor mapeado
int dbg             = 0;    //

int i               = 0;

String stringDelSerial  = "";         // a string to hold incoming data
String mac              = "";         // si todo sale bien aca vendra la mac del esp8266

boolean stringCompleta          = false;  // whether the string is complete
boolean online                  = false;  // bandera si esta ONLINE
boolean exito                   = true;   // bandera si la web devolvio exito
boolean esperandoRespuesta      = false;
boolean errorLecturaSensor      = false;
boolean banderaTiempoDatos      = true;
boolean banderaTiempoSensores   = false;
boolean banderaTiempoRespuesta  = false;

/****************************************************
* Construct de librerias del LCD y los sensores DHT *
****************************************************/

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
  delay(2500);
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
      i++;
      lcdOFFLINE();
      i++;
      lcdOFFLINE();
      leeSensores();
      lcdSensores();
      estaConectado();
  }

  if (online)
  {
    enviaDatos();
    leeSensores();
    lcdSensores();
  }
}



/***********************************************************/
/***********************************************************/
/***********************************************************/
/************************************************************
 * F U N C I O N E S - Cosas que se usan en Setup y en Loop *
 ***********************************************************/
 /***********************************************************/
 /***********************************************************/
 /***********************************************************/

// Genera el string a enviar
void enviaDatos()
{
  if (!online)
  {
    return;
  }
  esperas("datos", previousMillisEnviaDatos);
  if ( banderaTiempoDatos || !exito )//cuando la diferencia entre previousMillis y currentMillis es mayor o igual al intervalo se envian los datos
  {
    banderaTiempoDatos = false;
    if (!exito)
    {
      delay(1000);
    }
    lcdEnviandoDatos();
    //previousMillisEnviaDatos = currentMillis; // setea el valor de previousMillis a currentMillis

    String GET = "<";
    //GET += "&disp=";
    //GET += dispositivo;
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
    GET += dbg;
    GET += ">";

    //delay(500);
    Serial.print(GET);
  }
}

// Pregunta al Wifi si esta conectado
// to do revisar
void estaConectado ()
{
  Serial.print("[ESP_status]");
}

// lee los sensores y guarda los valores en sus variables.
void leeSensores()
{
  esperas("sensores", previousMillisLeeSensores);
  if ( banderaTiempoSensores || errorLecturaSensor )//cuando la diferencia entre previousMillisLeeSensores y currentMillis es mayor o igual al intervalo entra al if
  {
    banderaTiempoSensores = false;
    if (errorLecturaSensor)
    {
      delay(3000);
      errorLecturaSensor = false;
    }

    #ifdef DHT_PIN_0
      hum_0        = dht_0.readHumidity();
      temp_0       = dht_0.readTemperature();
      if (isnan(hum_0) || isnan(temp_0) )
      {
        lcdErrorLeeSensores();
        errorLecturaSensor = true;
      }
    #endif

    #ifdef DHT_PIN_1
      hum_1        = dht_1.readHumidity();
      temp_1       = dht_1.readTemperature();
      if (isnan(hum_0) || isnan(temp_0) )
      {
        lcdErrorLeeSensores();
        errorLecturaSensor = true;
      }
    #endif

    #ifdef LIGHT_SENSOR_PIN_0
      luz_0        = analogRead(LIGHT_SENSOR_PIN_0);
      valorLuz_0   = map(luz_0,0,1023,0,100);
    #endif

    #ifdef LIGHT_SENSOR_PIN_1
      luz_1        = analogRead(LIGHT_SENSOR_PIN_1);
      valorLuz_1   = map(luz_1,0,1023,0,100);
    #endif

    #ifdef SOIL_PIN_0
      suelo_0      = analogRead(SOIL_PIN_0);
      //valorSuelo_0 = map(suelo_0,0,1023,100,0); //con el modulo comprado
      valorSuelo_0 = map(suelo_0,0,1023,0,100); // con el casero
    #endif

    #ifdef SOIL_PIN_1
      suelo_1      = analogRead(SOIL_PIN_1);
      //valorSuelo_1 = map(suelo_1,0,1023,100,0); //con el modulo comprado
      valorSuelo_1 = map(suelo_1,0,1023,0,100); // con el casero
    #endif
  }
}

// esperas
void esperas( String bandera, unsigned long previousMillisQuien )
{
  //asigna el valor de millis() a la variable currentMillis
  currentMillis = millis();

  // si previousMillis es mayor a currentMillis quiere decir
  // que millis() volvio a cero porque se le lleno el buffer
  // entonces se resetan los previousMillis
  if (previousMillisQuien > currentMillis)   {
    previousMillisEnviaDatos  = 0;
    previousMillisRespuesta   = 0;
    previousMillisLeeSensores = 0;
  }

  if (bandera == "datos")
  {
    if ( currentMillis - previousMillisQuien >= intervaloDatos  )//cuando la diferencia entre previousMillis y currentMillis es mayor o igual al intervalo se envian los datos
    {
      previousMillisEnviaDatos = currentMillis;
      banderaTiempoDatos = true;
    }
  }

  if (bandera == "sensores")
  {
    if ( currentMillis - previousMillisQuien >= intervaloSensores  )//cuando la diferencia entre previousMillis y currentMillis es mayor o igual al intervalo se envian los datos
    {
      previousMillisLeeSensores = currentMillis;
      banderaTiempoSensores = true;
    }
  }

  if (bandera == "respuesta")
  {

    if ( currentMillis - previousMillisQuien >= intervaloRespuesta  )//cuando la diferencia entre previousMillis y currentMillis es mayor o igual al intervalo se envian los datos
    {
    previousMillisRespuesta = currentMillis;
    banderaTiempoRespuesta = true;
    }
  }
}


/******************************/
/******************************/
/******************************/
/******************************/
/* F U N C I O N E S   L C D  */
/******************************/
/******************************/
/******************************/
/******************************/

// Funcion que escribe dos renglones en la pantalla segun los sensores presentes.
void lcdSensores( )
{
  if ( esperandoRespuesta || errorLecturaSensor )
  {
    return;
  }

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
}

// Funcion que escribe dos renglones en la pantalla segun los sensores presentes cuando está offline
void lcdSensoresOffline( )
{
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
}



// escribe offline
void lcdOFFLINE()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  - OFFLINE -  ");
  //lcd.print(i);
  if (i % 2 == 0)
  {
    lcd.setCursor(0, 1);
    lcd.print("WiFi IndoorMatic");
  }
  else
  {
    lcd.setCursor(0, 1);
    lcd.print("  Conectate al ");
  }
  delay(1000);
}

// escribe enviando datoss
void lcdEnviandoDatos()
{
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
// to do obtener la mac del esp
void lcdBienvenida()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Indoor ");
  lcd.setCursor(0, 1);
  lcd.print("         Matic  ");
}

//escibe datos recibidos en la pantalla
void lcdExito()
{
  if( debug > 1)
  {
    delay(1000);
    Serial.print("\n");
    Serial.println(F("lcdExito delay 1000ms"));
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   D a t o s    ");
  lcd.setCursor(0, 1);
  lcd.print("Enviados  O.K.");
  delay(1000);
}

//escibe fallo de conexion en la pantalla
void lcdFracaso()
{
  if( debug > 1)
  {
    delay(1000);
    Serial.print("\n");
    Serial.println(F("lcdFracaso delay 1000ms"));
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("c o n e x i o n ");
  lcd.setCursor(0, 1);
  lcd.print(" f a l l i d a ");
  delay(1000);
}

//escibe fallo de conexion en la pantalla
void lcdTiempoAgotado()
{
  if( debug > 1)
  {
    delay(1000);
    Serial.print("\n");
    Serial.println(F("lcdTiempoAgotado delay 1000ms"));
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T i e m p o ");
  lcd.setCursor(0, 1);
  lcd.print("A g o t a d o ");
  delay(1000);
}

//escibe ONLINE en la pantalla
void lcdONLINE()
{
  if( debug > 1)
  {
    delay(1000);
    Serial.print("\n");
    Serial.println(F("lcdOnline delay 3000ms"));
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   - ONLINE -   ");
  lcd.setCursor(0, 1);
  lcd.print(" INDOOR - MATIC ");
  delay(3000);
}

//escibe ONLINE en la pantalla
void lcdErrorLeeSensores()
{
  if( debug > 1)
  {
    delay(1000);
    Serial.print("\n");
    Serial.println(F("lcdErrorLeeSensores delay 1000ms"));
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" - E R R O R -  ");
  lcd.setCursor(0, 1);
  lcd.print("S E N S O R E S ");
  delay(1000);
}


/***********************************
 * F U N C I O N E S   S E R I A L *
/**********************************/
// Escucha Serial = Deja en stringDelSerial una linea que vino por Serie a 115200 baudios
void escuchaSerial()
{
  //Serial.println("Funcion escuchaSerial");
  if (Serial.available())
  {
    stringDelSerial = Serial.readString();
    stringCompleta = true;
  }

  if(debug > 0)
  {
    Serial.print("\n");
    Serial.print("string del Serial = ");
    Serial.println(stringDelSerial);
  }
}

void analizaComando( String comando )
{
  //Serial.println("Funcion alizaComando");
  comando.trim();

  if (stringCompleta)
  {
    stringCompleta = false;

    if (comando.equals("CONECTADO_OK"))
    {
      online = true;
      //exito = true;
      //esperandoRespuesta = false;
      lcdONLINE();
    }

    else if (comando.equals("EXITO"))
    {
      exito = true;
      esperandoRespuesta = false;
      dbg = 0;
      lcdExito();
    }

    else if (comando.equals("conexion_FALLIDA"))
    {
      exito = false;
      online = false;
      //esperandoRespuesta = false;
      dbg += 1;

      lcdFracaso();
    }

    else if( comando.startsWith( "<" ) && comando.endsWith( ">" ) )
    {
      //datoRecibido = true;
      comando = comando.substring( 1, comando.length() - 1);
      String  dispositivo = comando;
    }

    else
    {
      exito = false;
      //esperandoRespuesta = false;
    }
  }
  stringDelSerial = "";
}
