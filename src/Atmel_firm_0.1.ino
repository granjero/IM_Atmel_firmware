/*********************************
*                                *
* Firmware del Indoor MATIC V0.1 *
*                                *
*********************************/

/* L I B R E R I A S */
#include <Arduino.h>
#include <LiquidCrystal.h> // lcd de 16 x 2
#include <DHT.h> // sensor de temp y humedad

/* C O N S T A N T E S - Textos para el lcd */
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

/* C O N S T A N T E S */
const long intervaloEnvioDatos        = 360000; // 6 minutos constante de espera para mandar los datos
const long intervaloLecturaSensores   = 2000; //constante de espera para volver a leer los sensores
const long intervaloSatus             = 600000;
/* D E F I N E  C O N S T A N T E S - Sensores conectados al IM */ //Descomentar los sensores presentes en el hardware
//#define DHT_TYPE            DHT21   // Define tipo de sensor de TyH DHT 21  (AM2301)
#define DHT_TYPE            DHT22   // Define tipo de sensor de TyH DHT 22  (AM2302)
#define DHT_PIN_0           2       // pin al que va el cable de dato del sensor0 de temperatura
//#define DHT_PIN_1           3       // pin al que va el cable de dato del sensor1 de temperatura
//#define LIGHT_SENSOR_PIN_0  A0      //PARA EL DE PEDRO
#define LIGHT_SENSOR_PIN_0  A2      //define el pin para el Photo-resistor_0
//#define LIGHT_SENSOR_PIN_1  A0      //define el pin para el Photo-resistor_1
#define SOIL_PIN_0          A1      //define el pin para sensor de humedad de tierra
//#define SOIL_PIN_1          A3      //define el pin para sensor de humedad de tierra

/* V A R I A B L E S */
unsigned long millisUltimaLecturaSensores = 0; //varialble que guardará el valor de tiempo donde se leen los sensores
unsigned long millisUltimoEnvioDatos = 0; //varialble que guardará el valor de tiempo donde se enviaron los datos
unsigned long millisUltimoStatus = 0; //varialble que guardará el valor de tiempo donde se enviaron los datos
unsigned long millisAhora;

float hum_0             = 999;  // humedad del sensor 0
float hum_1             = 999;  // humedad del sensor 1
float temp_0            = 999;  // temperatura del sensor 0
float temp_1            = 999;  // temperatura del sensor 0

int luz_0               = 999;  //valor analógico del pin
int luz_1               = 999;  //valor analógico del pin
int valorLuz_0          = 999;  //valor mapeado
int valorLuz_1          = 999;  //valor mapeado

int suelo_0             = 999;  //valor analogico del pin
int suelo_1             = 999;  //valor analogico del pin
float valorSuelo_0      = 999;  //valor mapeado
float valorSuelo_1      = 999;  //valor mapeado

String stringDelSerial  = "";   // string que recibe lo que viene por el puerto serial
String datos            = "";   // string que tiene los valores de los sensores
boolean stringCompleta  = false;  // será true cuando llegue un comando por serial
boolean online          = false;  // bandera si esta ONLINE

/* I N I C I A L I Z A libreria del LCD*/
LiquidCrystal lcd( 13, 12, 11, 10, 9, 8 ); //inicializa la libreria con los numeros de pines utilizados

/* I N I C I A L I Z A libreria de los sensores de TyH definidos previamente*/
#ifdef DHT_PIN_0
  DHT dht_0( DHT_PIN_0, DHT_TYPE ); //inicializa la libreria para el pin
#endif

#ifdef DHT_PIN_1
  DHT dht_1( DHT_PIN_1, DHT_TYPE );
#endif

/* S E T U P - Cosas que corren una sola vez */
void setup()
{
  Serial.begin( 115200 ); // inicia comunicacion serie a 115.200 baudios

  lcd.begin( 16, 2 ); // inicia el lcd y setea el numero de columnas y de filas del lcd

  #ifdef DHT_PIN_0
    dht_0.begin(); // inicia el sensor0
  #endif

  #ifdef DHT_PIN_1
    dht_1.begin(); // inicia el sensor1
  #endif

  lcdBienvenida();
  delay( 5000 );
}

/* L O O P - Cosas que corren para siempre */
void loop()
{
  espStatus( 600000 );
  escuchaSerial();
  analizaComando( stringDelSerial );

  if ( !online )
  {
    espStatus( 2000 );
    lcdOFFLINE();
  }

  leeSensores();
  lcdSensores();

  if ( online )
  {
    enviaDatos();
  }
}

/* F U N C I O N E S */

void espStatus( unsigned long tiempo )
{
  millisAhora = millis();
  if (millisUltimoStatus > millisAhora)  // si millisUltimoStatus es mayor a millisAhora quiere decir que millis() volvio a cero porque se lleno entonces vuelvo tambien a cero millisUltimoStatus
  {
    millisUltimoEnvioDatos  = 0;
  }

  if (millisAhora - millisUltimoStatus >= tiempo)
  {
    millisUltimoStatus = millis();
    Serial.println( "[ESP_status]" );
  }
}

/* FUNCIONES SERIAL */
//toma el string del serial y lo guarda en stringDelSerial y pone en TRUE stringCompleta
void escuchaSerial()
{
  if (Serial.available())
  {
    stringDelSerial = Serial.readString();
    stringCompleta = true;
  }
}

//toma un string (el que llegó por serie) y aactua en consecuencia ;-)
void analizaComando( String comando )
{
  comando.trim();
  if ( stringCompleta )
  {
    stringCompleta = false;

    if ( comando.equals( "[ONLINE]" ) )
    {
      online = true;
      lcdONLINE();
    }

    else if ( comando.equals( "[OFFLINE]" ) )
    {
      online = false;
    }

    /*else if ( comando.equals( "[ENVIANDO_DATOS]" ) )
    {
      lcdEnviandoDatos();
    }*/

    else if ( comando.equals( "[EXITO]" ) )
    {
      lcdExito();
    }

    else if ( comando.equals( "[NO]" ) )
    {
      lcdNO();
    }

    else if ( comando.equals( "[FALLO_CONEXION]" ) )
    {
      lcdFalloConexion();
    }

    /*else if( comando.startsWith( "<" ) && comando.endsWith( ">" ) )
    {
      comando = comando.substring( 1, comando.length() - 1);
    }*/

    else
    {
    }
  }
  stringDelSerial = "";
}
/* FIN FUNCIONES SERIAL */

/* FUNCIONES DATOS*/
//si pasó el tiempo de intervaloEnvioDatos envia al esp un string con los valres de los sensores
void enviaDatos()
{
  millisAhora = millis();
  if (millisUltimoEnvioDatos > millisAhora)  // si millisUltimaLecturaSensores es mayor a millisAhora quiere decir que millis() volvio a cero porque se lleno entonces vuelvo tambien a cero millisUltimaLecturaSensores
  {
    millisUltimoEnvioDatos  = 0;
  }

  if (millisAhora - millisUltimoEnvioDatos >= intervaloEnvioDatos)
  {
    millisUltimoEnvioDatos = millis();

    datos = "<";
    datos += "&t0=";
    datos += temp_0;
    datos += "&h0=";
    datos += hum_0;
    datos += "&t1=";
    datos += temp_1;
    datos += "&h1=";
    datos += hum_1;
    datos += "&luz=";
    datos += valorLuz_0;
    datos += "&luz1=";
    datos += valorLuz_1;
    datos += "&ht=";
    datos += valorSuelo_0;
    datos += "&ht1=";
    datos += valorSuelo_1;
    datos += ">";

    Serial.print(datos);

    lcdEnviandoDatos();
  }
}
/* FIN FUNCIONES DATOS*/

/* FUNCIONES SENSORES */
// lee los sensores si pasó el iempo de intervaloLecturaSensores
void leeSensores() // Según la hoja de datos habría que leer cada dos segundos por eso el if
{
  millisAhora = millis();
  if (millisUltimaLecturaSensores > millisAhora)  // si millisUltimaLecturaSensores es mayor a millisAhora quiere decir que millis() volvio a cero porque se lleno entonces vuelvo tambien a cero millisUltimaLecturaSensores
  {
    millisUltimaLecturaSensores  = 0;
  }

  if (millisAhora - millisUltimaLecturaSensores >= intervaloLecturaSensores)
  {
    millisUltimaLecturaSensores = millis();
    #ifdef DHT_PIN_0
      hum_0   = dht_0.readHumidity();
      temp_0  = dht_0.readTemperature();
      if ( isnan( hum_0 ) || isnan( temp_0 ) )
      {
        hum_0   = 1000;
        temp_0  = 1000;
      }
    #endif

    #ifdef DHT_PIN_1
      hum_1   = dht_1.readHumidity();
      temp_1  = dht_1.readTemperature();
      if ( isnan( hum_0 ) || isnan( temp_0 ) )
      {
        hum_1   = 1000;
        temp_1  = 1000;
      }
    #endif

    #ifdef LIGHT_SENSOR_PIN_0
      luz_0        = analogRead( LIGHT_SENSOR_PIN_0 );
      valorLuz_0   = map( luz_0, 0, 1023, 0, 100 );
    #endif

    #ifdef LIGHT_SENSOR_PIN_1
      luz_1        = analogRead( LIGHT_SENSOR_PIN_1 );
      valorLuz_1   = map( luz_1, 0, 1023, 0, 100 );
    #endif

    #ifdef SOIL_PIN_0
      suelo_0      = analogRead( SOIL_PIN_0 );
      //valorSuelo_0 = map( suelo_0, 0, 1023, 100, 0 ); //con el modulo comprado
      valorSuelo_0 = map( suelo_0, 0, 1023, 0, 100 ); // con el casero
    #endif

    #ifdef SOIL_PIN_1
      suelo_1      = analogRead( SOIL_PIN_1 );
      //valorSuelo_1 = map( suelo_1, 0, 1023, 100, 0 ); //con el modulo comprado
      valorSuelo_1 = map( suelo_1, 0, 1023, 0, 100 ); // con el casero
    #endif
  }
}

/* FUNCIONES LCD */
void lcdBienvenida()
{
  lcd.clear();
  lcd.setCursor( 0, 0 );
  lcd.print("   Indoor");
  lcd.setCursor( 0, 1 );
  lcd.print("        Matic");
}

void lcdSensores( )
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

//escibe ONLINE en la pantalla
void lcdONLINE()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print( "   - ONLINE -   " );
  lcd.setCursor(0, 1);
  lcd.print( " INDOOR - MATIC " );
  delay(3000);
}

//escibe OFFLINE en la pantalla
void lcdOFFLINE()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print( "  Conectate al  " );
  lcd.setCursor(0, 1);
  lcd.print( "WiFi IndoorMatic" );
  delay(3000);
}

// escribe enviando datoss
void lcdEnviandoDatos()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print( "E n v i a n d o " );
  //lcd.print(i);
  lcd.setCursor(0, 1);
  lcd.print( "   D a t o s    " );
  delay(1000);
}

//escibe datos recibidos en la pantalla
void lcdExito()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print( "   D a t o s    " );
  lcd.setCursor(0, 1);
  lcd.print( "Enviados  O.K." );
  delay(1000);
}

void lcdNO()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print( "   D a t o s    " );
  lcd.setCursor(0, 1);
  lcd.print( "NO enviados (?)" );
  delay(1000);
}

void lcdFalloConexion()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print( "   F A L L O   " );
  lcd.setCursor(0, 1);
  lcd.print( "C O N E X I O N" );
  delay(1000);
}
