/*********************************
*                                *
* Firmware del Indoor MATIC V0.1 *
*                                *
*********************************/

/* L I B R E R I A S */
#include <Arduino.h>
#include <LiquidCrystal.h> // lcd de 16 x 2

byte Wi[8] = {
	0b10111,
	0b00000,
	0b00000,
	0b11110,
	0b00001,
	0b00110,
	0b00001,
	0b11110
};

byte Fi[8] = {
	0b10111,
	0b00000,
	0b00000,
	0b10000,
	0b10100,
	0b10100,
	0b11111,
  0b00000
};

byte Rst[8] = {
	0b11001,
	0b01100,
	0b00110,
	0b10011,
	0b11001,
	0b01100,
	0b00110,
	0b10011
};


#include <DHT.h> // sensor de temp y humedad

/* C O N S T A N T E S - Textos para el lcd */
#define T_s0  "T "
#define T_s1  "Temp s1: "
#define H_s0  "H "
#define H_s1  " Hum s1: "
#define L_s0  "Luz "
#define L_s1  " Luz s1: "
#define S_s0  "Suelo 0: "
#define S_s1  "Suelo 1: "
#define P     "% "
#define C     "C "
#define L     "^ "
#define sensor     "Sensor "

/* C O N S T A N T E S */
const long intervaloEnvioDatos        = 360000; // 6 minutos constante de espera para mandar los datos
const long intervaloLecturaSensores   = 2000; //constante de espera para volver a leer los sensores

/* D E F I N E  C O N S T A N T E S - Sensores conectados al IM */ //Descomentar los sensores presentes en el hardware
//#define DHT_TYPE            DHT21   // Define tipo de sensor de TyH DHT 21  (AM2301)
#define DHT_TYPE            DHT22   // Define tipo de sensor de TyH DHT 22  (AM2302)
#define DHT_PIN_0           2       // pin al que va el cable de dato del sensor0 de temperatura
//#define DHT_PIN_0           3       // error de soldadura quitar
#define DHT_PIN_1           3       // pin al que va el cable de dato del sensor1 de temperatura
#define BOMBA               7

//#define LIGHT_SENSOR_PIN_0  A0      //PARA EL DE PEDRO
#define LIGHT_SENSOR_PIN_0  A2      //define el pin para el Photo-resistor_0
//#define LIGHT_SENSOR_PIN_1  A0      //define el pin para el Photo-resistor_1
#define SOIL_PIN_0          A1      //define el pin para sensor de humedad de tierra
//#define SOIL_PIN_1          A3      //define el pin para sensor de humedad de tierra

/* V A R I A B L E S */
unsigned long millisUltimaLecturaSensores = 0; //varialble que guardará el valor de tiempo donde se leen los sensores
unsigned long millisUltimoEnvioDatos      = 0; //varialble que guardará el valor de tiempo donde se enviaron los datos
unsigned long millisUltimoStatus          = 0; //varialble que guardará el valor de tiempo del ultimo status
unsigned long millisFinRiego              = 0; //varialble que guardará el valor de tiempo donde comeinza a regar

unsigned long intervaloLCD = 3000;
unsigned long millisLCDUltimoMensaje = 0;
unsigned long millisAhora = 0;

int cantidadSensores = 0;
int cantidadMensajes = 0;

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
int promedioSuelo_0     = 0;    //el sensor casero da mucha amplitud el promedio es para bajarla.
int promedioSuelo_1     = 0;
int contadorSuelo       = 0;

String stringDelSerial  = "";   // string que recibe lo que viene por el puerto serial
String datos            = "";   // string que tiene los valores de los sensores

boolean stringCompleta  = false;  // será true cuando llegue un comando por serial
boolean online          = false;  // bandera si esta ONLINE

//bancderas de mesajes
boolean lcd_DHT_0           = true;
boolean lcd_DHT_1           = true;
boolean lcd_LUZ             = true;
boolean lcd_SUELO           = true;
boolean lcd_ONLINE          = false;
boolean lcd_OFFLINE         = false;
boolean lcd_ENVIA_DATOS     = false;
boolean lcd_DATOS_OK        = false;
boolean lcd_FALLO_CONEXION  = false;
boolean lcd_REGANDO         = false;
boolean lcd_RESPUESTA_NULA  = false;
boolean LOOP								= false;


int regarInt          = 2;
int tiempoRiegoInt    = 0;
int humidificadorInt  = 0;


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

  lcd.createChar( 0, Wi );
  lcd.createChar( 1, Fi );
	lcd.createChar( 2, Rst );
  lcd.begin( 16, 2 ); // inicia el lcd y setea el numero de columnas y de filas del lcd

  #ifdef DHT_PIN_0
    dht_0.begin(); // inicia el sensor0
    cantidadSensores++;
  #endif

  #ifdef DHT_PIN_1
    dht_1.begin(); // inicia el sensor1
    cantidadSensores++;
  #endif

  #ifdef SOIL_PIN_0
    cantidadSensores++;
  #endif

  // #ifdef LIGHT_SENSOR_PIN_0
  //   cantidadSensores++;
  // #endif

  pinMode(BOMBA, OUTPUT);
	digitalWrite(BOMBA, LOW);

  lcd.clear();
  lcd.setCursor( 0, 0 );
  lcd.print( F ( "   Indoor" ) );
  lcd.setCursor( 0, 1 );
  lcd.print( F ( "        Matic" ) );
  delay( 5000 );
	lcd.clear();
}

/* L O O P - Cosas que corren para siempre */
void loop()
{
  if ( !online )
  {
		if ( !LOOP )
		{
			lcdWiFi();
		}

		if ( LOOP )
		{
			lcdRST();
		}

		espStatus( 10000 );
  }

  leeSensores();
  escuchaSerial();
  analizaComando( stringDelSerial );

  mensajesLCD();

  if ( online )
  {
    enviaDatos();
		funcionRegar();
  }

  espStatus( 600000 );

}

/* F U N C I O N E S */

void espStatus( unsigned long tiempo )
{
  millisAhora = millis();
  if ( millisUltimoStatus > millisAhora )  // si millisUltimoStatus es mayor a millisAhora quiere decir que millis() volvio a cero porque se lleno entonces vuelvo tambien a cero millisUltimoStatus
  {
    millisUltimoStatus  = 0;
  }

  if ( millisAhora - millisUltimoStatus >= tiempo )
  {
    millisUltimoStatus = millis();
    Serial.println( F( "[ESP_status]" ) );
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
			LOOP = false;
      lcd_ONLINE = true;
      estadoLcdSensores( false );
			lcd.clear();
    }

    else if ( comando.equals( "[OFFLINE]" ) )
    {
      online = false;
      lcd_OFFLINE = true;
      estadoLcdSensores( false );
    }

    else if ( comando.equals( "[LOOP]" ) )
    {
      LOOP = true;
    }

    else if ( comando.equals( "[EXITO]" ) )
    {
      lcd_DATOS_OK = true;
      estadoLcdSensores( false );
    }

    // else if ( comando.equals( "[NO]" ) )
    // {
    //   lcdNO();
    // }

    else if ( comando.equals( "[FALLO_CONEXION]" ) )
    {
      lcd_FALLO_CONEXION = true;
      estadoLcdSensores( false );
    }

    else if ( comando.equals( "[RESPUESTA_NULA]" ) )
    {
      lcd_RESPUESTA_NULA = true;
      estadoLcdSensores( false );
    }

    else if( comando.startsWith( "<" ) && comando.endsWith( ">" ) )
    {
      parseaComandoRiego( comando );
    }

    else
    {
    }
  }
  stringDelSerial = "";
}

void parseaComandoRiego ( String comando )
{
  millisAhora = millis();

  comando = comando.substring( 1, comando.length() - 1 );

  int     del1; // indide de los delimitadores
  int     del2;
  int     del3;
  String  regar;
  String  tiempoRiego;
  String  humidificador;

  del1              = comando.indexOf( ';' );  //encuentra el primer delimitador
  regar             = comando.substring( 0, del1 );
  regarInt          = regar.toInt();

  del2              = comando.indexOf( ';', del1 + 1 );   //encuentra el siguiente delimitador
  tiempoRiego       = comando.substring( del1 + 1, del2 );
  tiempoRiegoInt    = tiempoRiego.toInt();

  del3              = comando.indexOf( ';', del2 + 1 );
  humidificador     = comando.substring( del2 + 1, del3 );
  humidificadorInt  = humidificador.toInt();

}
/* FIN FUNCIONES SERIAL */

/* FUNCIONES RIEGO*/
void funcionRegar( )
{
  millisAhora = millis();

  if ( regarInt == 1 )
  {
    regarInt = 0;

    lcd_REGANDO = true;
    estadoLcdSensores( false );

    millisFinRiego = ( (unsigned long)tiempoRiegoInt * 1000 ) + millisAhora;

    digitalWrite(BOMBA, HIGH);
		Serial.println( F ( "[REGANDO]" ) );

  }

  if ( millisAhora >= millisFinRiego )
  {
    digitalWrite(BOMBA, LOW);
		delay(250);
		millisFinRiego = 0;
  }
}

/* FIN FUNCIONES RIEGO*/


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

    promedioSuelo_0 = promedioSuelo_0 / contadorSuelo;
    promedioSuelo_1 = promedioSuelo_1 / contadorSuelo;

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
    datos += promedioSuelo_0;
    datos += "&ht1=";
    datos += promedioSuelo_1;
    datos += ">";

    Serial.println(datos);

    promedioSuelo_0 = 0;
    promedioSuelo_1 = 0;
    contadorSuelo = 0;

    lcd_ENVIA_DATOS = true;
    estadoLcdSensores( false );
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
        hum_0   = -1;
        temp_0  = -1;
      }
    #endif

    #ifdef DHT_PIN_1
      hum_1   = dht_1.readHumidity();
      temp_1  = dht_1.readTemperature();
      if ( isnan( hum_0 ) || isnan( temp_0 ) )
      {
        hum_1   = -1;
        temp_1  = -1;
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
      promedioSuelo_0 = promedioSuelo_0 + valorSuelo_0;
      contadorSuelo++;
    #endif

    #ifdef SOIL_PIN_1
      suelo_1      = analogRead( SOIL_PIN_1 );
      //valorSuelo_1 = map( suelo_1, 0, 1023, 100, 0 ); //con el modulo comprado
      valorSuelo_1 = map( suelo_1, 0, 1023, 0, 100 ); // con el casero
      promedioSuelo_1 = promedioSuelo_1 + valorSuelo_1;
    #endif
  }
}

void lcdWiFi()
{
	lcd.setCursor( 15, 0 );
	lcd.write( byte( 1 ) );
	lcd.setCursor( 15, 1 );
	lcd.write( byte( 0 ) );
}

void lcdRST()
{
	lcd.setCursor( 15, 0 );
	lcd.write( byte( 2 ) );
	lcd.setCursor( 15, 1 );
	lcd.write( byte( 2 ) );
}

void mensajesLCD()
{
  millisAhora = millis();

  #ifdef DHT_PIN_0
    if( ( lcd_DHT_0 ) && ( millisAhora - millisLCDUltimoMensaje >= intervaloLCD ) )
    {
      millisLCDUltimoMensaje = millis();
      lcd_DHT_0 = false;
      cantidadMensajes++;
      lcd.clear();
      lcd.setCursor( 0, 0 );
      lcd.print( T_s0 );
      lcd.print( temp_0, 1 );
      lcd.print( C );
			lcd.print( H_s0 );
			lcd.print( hum_0, 1 );
			lcd.print( P );
			lcd.setCursor( 0, 1 );

			#ifdef LIGHT_SENSOR_PIN_0
		  // if( ( lcd_LUZ ) && ( millisAhora - millisLCDUltimoMensaje >= intervaloLCD ) )
		  // {
		    // millisLCDUltimoMensaje = millis();
		    // lcd_LUZ = false;
		    // cantidadMensajes++;
		    // lcd.clear();
		    // lcd.setCursor(0, 0);
		    lcd.print( L_s0 );
		    lcd.print( valorLuz_0 );
		    lcd.print( L );
				lcd.print( sensor );
				lcd.print( "0" );


		      // #ifdef LIGHT_SENSOR_PIN_1
		      // lcd.setCursor(0, 1);
		      // lcd.print(L_s1);
		      // lcd.print(valorLuz_1);
		      // lcd.print(L);
		      // #endif
		      // return;
		    // }
		  #endif
      return;
    }
  #endif

  #ifdef DHT_PIN_1
    if( ( lcd_DHT_1 ) && ( millisAhora - millisLCDUltimoMensaje >= intervaloLCD ) )
    {
      millisLCDUltimoMensaje = millis();
      lcd_DHT_1 = false;
      cantidadMensajes++;
      lcd.clear();
      lcd.setCursor( 0, 0 );
      lcd.print( T_s1 );
      lcd.print( temp_1 );
      lcd.print( C );
      lcd.setCursor( 0, 1 );
      lcd.print( H_s1 );
      lcd.print( hum_1 );
      lcd.print( P );

			#ifdef LIGHT_SENSOR_PIN_1
				lcd.setCursor( 0, 1 );
				lcd.print( L_s0 );
				lcd.print( valorLuz_1 );
				lcd.print( L );
				lcd.print( sensor );
				lcd.print( "1");
			#endif
      return;
    }
  #endif



  #ifdef SOIL_PIN_0
    if( ( lcd_SUELO ) && ( millisAhora - millisLCDUltimoMensaje >= intervaloLCD ) )
    {
      millisLCDUltimoMensaje = millis();
      lcd_SUELO = false;
      cantidadMensajes++;
      lcd.clear();
      lcd.setCursor( 0, 0 );
      lcd.print( S_s0 );
      lcd.print( valorSuelo_0, 0 );
      #ifdef SOIL_PIN_1
        lcd.setCursor( 0, 1 );
        lcd.print( S_s1 );
        lcd.print( valorSuelo_1, 0 );
      #endif
      return;
    }
  #endif

  if ( cantidadSensores == cantidadMensajes )
  {
    estadoLcdSensores( true );
  }


  if( lcd_ONLINE && millisAhora - millisLCDUltimoMensaje >= intervaloLCD  )
  {
    millisLCDUltimoMensaje = millis();
    lcd_ONLINE = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print( F( "   - ONLINE -   " ) );
    lcd.setCursor(0, 1);
    lcd.print( F( " INDOOR - MATIC " ) );
    estadoLcdSensores( true );
    return;
  }

  if( lcd_OFFLINE && millisAhora - millisLCDUltimoMensaje >= intervaloLCD  )
  {
    millisLCDUltimoMensaje = millis();
    lcd_OFFLINE = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print( F( "  Conectate al  " ) );
    lcd.setCursor(0, 1);
    lcd.print( F( "WiFi IndoorMatic" ) );
    estadoLcdSensores( true );
    return;
  }

  if( lcd_ENVIA_DATOS && millisAhora - millisLCDUltimoMensaje >= intervaloLCD  )
  {
    millisLCDUltimoMensaje = millis();
    lcd_ENVIA_DATOS = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print( F( "E n v i a n d o " ) );
    lcd.setCursor(0, 1);
    lcd.print( F( "   D a t o s    " ) );
    estadoLcdSensores( true );
    return;
  }

  if( lcd_DATOS_OK && millisAhora - millisLCDUltimoMensaje >= intervaloLCD  )
  {
    millisLCDUltimoMensaje = millis();
    lcd_DATOS_OK = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print( F( "   D a t o s    " ) );
    lcd.setCursor(0, 1);
    lcd.print( F( "Enviados  O.K." ) );
    estadoLcdSensores( true );
    return;
  }

  if( lcd_FALLO_CONEXION && millisAhora - millisLCDUltimoMensaje >= intervaloLCD  )
  {
    millisLCDUltimoMensaje = millis();
    lcd_FALLO_CONEXION = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print( F( "   F A L L O   " ) );
    lcd.setCursor(0, 1);
    lcd.print( F( "C O N E X I O N" ) );
    estadoLcdSensores( true );
    return;
  }

  if( lcd_REGANDO && millisAhora - millisLCDUltimoMensaje >= intervaloLCD  )
  {
    millisLCDUltimoMensaje = millis();
    lcd_REGANDO = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print( F( "  REGANDO   " ) );
    estadoLcdSensores( true );
    return;
  }

  if( lcd_RESPUESTA_NULA && millisAhora - millisLCDUltimoMensaje >= intervaloLCD  )
  {
    millisLCDUltimoMensaje = millis();
    lcd_RESPUESTA_NULA = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print( F( "RESPUESTA" ) );
    lcd.setCursor(0, 1);
    lcd.print( F( "            NULA" ) );
    estadoLcdSensores( true );
    return;
  }

  if( !online && millisAhora - millisLCDUltimoMensaje >= intervaloLCD  )
  {
    millisLCDUltimoMensaje = millis();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print( F( "  Conectate al  " ) );
    lcd.setCursor(0, 1);
    lcd.print( F( "WiFi IndoorMatic" ) );
    estadoLcdSensores( true );
    return;
  }

  // if( lcd_X && millisAhora - millisLCDUltimoMensaje >= intervaloLCD  )
  // {
  //   millisLCDUltimoMensaje = millis();
  //   lcd_X = false;
  //
  //   estadoLcdSensores( true );
  //   return;
  // }
}

void estadoLcdSensores(boolean estado)
{
  cantidadMensajes = 0;
  lcd_DHT_0 = estado;
  lcd_DHT_1 = estado;
  lcd_LUZ = estado;
  lcd_SUELO = estado;
}
