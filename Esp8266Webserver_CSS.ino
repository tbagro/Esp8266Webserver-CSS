//************Bibliotecas****************************
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHTesp.h> //https://github.com/beegee-tokyo/DHTesp

//************variaveis do router
#define ssid      "SSID"       // WiFi SSID
#define password  "Senha"   // WiFi password
//************ variaveis GPIO*************************
const uint8_t GPIOPIN[4] = {D1, D6, D7, D8}; // Led array
String  statusGpio[4] = {"OFF", "OFF", "OFF", "OFF"}; // inicia todos os pins e
bool  boolGpio[4] = {0, 0, 0, 0}; //muda o status dos botões no server
//************variais de leitura dos sensores
DHTesp dht;
float   t = 0 ;               //varivaveis que armazenam temporariamente a ultima leitura do sensor
float   h = 0 ;               //varivaveis que armazenam temporariamente a ultima leitura do sensor
//************Memoria SPIFFS*************************
#include "FS.h";

void SPIFFS_Save_Data() { // Grava as variaveis na  SPIFFS
  // Assign a file name e.g. 'names.dat' or 'data.txt' or 'data.dat' try to use the 8.3 file naming convention format could be 'data.d'
  char filename [] = "datalog.txt";                     // Assign a filename or use the format e.g. SD.open("datalog.txt",...);
  if (SPIFFS.exists(filename)) SPIFFS.remove(filename); // First in this example check to see if a file already exists, if so delete it
  File dataFile = SPIFFS.open(filename, "a+");          // Open a file for reading and writing (appending)
  if (!dataFile) Serial.println("file open failed");    // Check for errors
  else {
    for ( int x = 0 ; x < 4 ; x++ ) {
      dataFile.println(boolGpio[x]);                        // Write data to file
      Serial.print("Save; ");
      Serial.println(boolGpio[x]);
    }
  }
  dataFile.close(); // Close the file
}

void Read_Data() { //Lê as variaveis na  SPIFFS na função  SPIFFSSetup()
  char filename [] = "datalog.txt";                     // Assign a filename or use the format e.g. SD.open("datalog.txt",...);
  String line_input;
  File dataFile = SPIFFS.open(filename, "r");           // Open the file again, this time for reading
  if (!dataFile) Serial.println("file open failed");    // Check for errors
  while (dataFile.available()) {
    for ( uint8_t x = 0 ; x < 4 ; x++ ) {
      line_input = dataFile.readStringUntil('\n'); boolGpio[x] = line_input.toInt();
      Serial.print("Read ");
      Serial.println(boolGpio[x]);
    }
  }
  dataFile.close();   // Close the file
  Serial.println("*  Dados lidos da  SPIFFS");
}

void SPIFFS_Setup() { //Recupera as variaveis no reinicio da placa - manter no Setup
  bool ok = SPIFFS.begin();
  if (ok) Serial.println("Spiffs started"); else Serial.println("SPIFFS.failed");
  Read_Data();//Lê as variaveis na  SPIFFS
}

//************variaveis do servidor***********
ESP8266WebServer server ( 80 ); // porta local do servidor

IPAddress ip(192, 168, 1, 116); //NodeMCU static IP
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);


String buildWebsite() { //pagina home html

  String webSite = "<html lang='pt-br'><head>";
  webSite += "<meta charset='UTF-8'>";
  webSite += "<meta http-equiv='refresh' content='60'>";
  webSite +  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";//define a responsividade da pagina

  webSite += "<style>";
  webSite += "html{font-family: 'Lucida Sans', sans-serif; position: relative;   min-height: 100%;}";
  webSite += "body{ margin: 0 0 60px;}";
  webSite += "header{background-color: #ff7f00; color: #ffffff;padding: 2px;  width:100%;}";
  webSite += "button{font-size: 40px; border-radius: 10px;border: 10px;color: white;padding: 30px 50px;text-align: center;display: inline-block;font-weight: bold;margin:1px -1px;cursor: pointer;}";
  webSite += ".buttonON{font-weight: bold;  background-color: white;  color: black; border: 3px solid #008CBA;}";
  webSite += ".buttonOFF{font-weight: bold;  background-color: white;  color: black; border: 4px solid #ff0000;}";
  webSite += ".grid-container {display: grid;  grid-template-columns: auto ;  text-align: center;  padding: 1px }";
  webSite += ".grid-container2{display: grid;  grid-template-columns: auto auto ;   padding:2px;}";
  webSite += ".grid-item1 {padding: 1px; padding-left: 20%;   padding-right: 6%; }";
  webSite += ".grid-item2 {padding: 1px;  padding-right: 25%;}";
  webSite += "h1 {text-align: center; font-size: 30px;    font-weight: bolder;   font-style: italic;  text-shadow: 1px 1px 2px rgba(0,0,0,0,5);}";
  webSite += "h3{font-size: 40px;  text-align: center;}";
  webSite += "h4{font-size: 40px;  text-align: center;}"; 
  webSite += "span{font-size: 35px; font-weight: bold;}";
  webSite += "table{font-size: 35px; padding-left: 20%;  padding-right: 20%;}";
  webSite += "tr{text-align: center;}";
  webSite += "td{padding: 5px;}";
  webSite += "footer{font-size:20px; font-weight: bold; text-align: center; position: absolute; bottom: 0px; height: 60px;}";

  webSite += "</style >";
  //
  webSite += "<title>Mini estação Metereologica </title> ";// titulo da pagina no navegador
  webSite += " </head> ";
  webSite += "<header>";
  webSite +=       "<h1>Webserver ESP8266 </h1> ";
  webSite +=       "<h3>Mini estação Metereologica </h3> ";
  webSite += " </header> ";
  //
  webSite += "<body>";

  //tabela dos sensores

  webSite +=       "<section class='grid-container'> ";//tabela responsiva
  webSite +=       "<table>";  // Tableau des relevés
  webSite +=         "<thead><tr><th>Sensor </th > <th>Medição </th> <th>Leitura </th > </tr > </thead > "; //Entrada
  webSite +=         "<tbody>";  // Conteudo da tabela
  webSite +=           "<tr><td>DHT11 </td > <td>Temp </td > <td>"; // primeira linha exibe a  temperatura
  webSite +=             t;// exibe a variavel de temperatura
  webSite +=             "&deg; C </td > </tr> ";
  webSite +=           "<tr class = 'active'><td>DHT11 </td ><td>Umid </td> <td>"; // primeira linha exibe a  umidade
  webSite +=             h;// exibe a variavel de umidade
  webSite +=             " % </td> </tr> ";
  webSite +=       " </tbody></table>  ";
  webSite +=       " </section > ";//fecha tabela responsiva

  //tabela dos botões


  webSite +=       "<h3 >GPIO</h3>";
  webSite +=       "<section  class='grid-container2'>";// D5
  webSite +=         "<h4  class='grid-item1'>D5 ";
  webSite +=           "<span >";
  webSite +=           statusGpio[0]; //Array botão D5 exibe estatus do pino
  webSite +=         "</span></h4>";
  webSite +=         (boolGpio[0]) ? "<form class='grid-item2' action=' / ' method='POST'><button type='button submit' name='D5' value='1' class='buttonON'>ON</button></form>" : "<form class='grid-item2' action=' / ' method='POST'><button type='button submit' name='D5' value='0' class='buttonOFF'>OFF</button></form>";
  webSite +=       "</section>";
  //
  webSite +=       "<section  class='grid-container2'>";// D6
  webSite +=         "<h4  class='grid-item1'>D6 ";
  webSite +=           "<span class='badge'>";
  webSite +=           statusGpio[1]; //Array botão D6 exibe estatus do pino
  webSite +=         "</span></h4>";
  webSite +=         (boolGpio[1]) ? "<form class='grid-item2' action=' / ' method='POST'><button type='button submit' name='D6' value='1' class='buttonON'>ON</button></form>" : "<form class='grid-item2' action=' / ' method='POST'><button type='button submit' name='D6' value='0' class='buttonOFF'>OFF</button></form>";
  webSite +=       "</section>";
  //
  webSite +=       "<section  class='grid-container2'>";// D7
  webSite +=         "<h4  class='grid-item1'>D7 ";
  webSite +=           "<span class='badge'>";
  webSite +=           statusGpio[2]; //Array botão D7 exibe estatus do pino
  webSite +=         "</span></h4>";
  webSite +=         (boolGpio[2]) ? "<form class='grid-item2' action=' / ' method='POST'><button type='button submit' name='D7' value='1' class='buttonON'>ON</button></form>" : "<form class='grid-item2' action=' / ' method='POST'><button type='button submit' name='D7' value='0' class='buttonOFF'>OFF</button></form>";
  webSite +=       "</section>";
  //
  webSite +=       "<section  class='grid-container2'>";// D8
  webSite +=         "<h4  class='grid-item1'>D8 ";
  webSite +=           "<span class='badge'>";
  webSite +=           statusGpio[3]; //Array botão D8 exibe estatus do pino
  webSite +=         "</span></h4>";
  webSite +=         (boolGpio[3]) ? "<form class='grid-item2' action=' / ' method='POST'><button type='button submit' name='D8' value='1' class='buttonON'>ON</button></form>" : "<form class='grid-item2' action=' / ' method='POST'><button type='button submit' name='D8' value='0' class='buttonOFF'>OFF</button></form>";
  webSite +=       "</section>";


  webSite +=       "</body>";
  webSite +=    "<footer>Tiago Batista-2018</footer>";
  webSite +=  "</html>";
  return webSite;
}

void check() { //monitora o status dos botões
  for ( int x = 0 ; x < 4 ; x++ ) {
    if (boolGpio[x] == 0) { // Array de consulta dos pinos GPIO se boolGpio[x] ==0 liga o pino GPIOPIN[x]
      digitalWrite(GPIOPIN[x], HIGH);
    } else {
      digitalWrite(GPIOPIN[x], LOW);
    }
  }
}

void updateGPIO(int gpio, String DxValue) { // função de alteração do status do pino (pino,status)
  Serial.println("");
  Serial.println("Update GPIO "); Serial.print(GPIOPIN[gpio]); Serial.print(" -> "); Serial.println(DxValue);

  if ( DxValue == "1") { // liga o pino
    statusGpio[gpio] = "On"; // alterna Status do texto para ON
    boolGpio[gpio] = 0; //muda o status do botão para OFF
    server.send ( 200, "text/html", buildWebsite() );//atualiza a pagina
    SPIFFS_Save_Data(); // grava a alteração na memoria  SPIFFS
  } else if ( DxValue == "0" ) {//desliga o pino
    statusGpio[gpio] = "Off";
    boolGpio[gpio] = 1;//muda o status do botão para ON
    SPIFFS_Save_Data(); // grava a alteração na memoria SPIFFS
    server.send ( 200, "text/html", buildWebsite() ); // Atualiza a pagina
  } else {
    Serial.println("Err Led Value");
  }
}

void handleRoot() { // verifica as variaveis do html
  if ( server.hasArg("D5") ) { // se button nome == D5
    updateGPIO(0, server.arg("D5"));// chama a função updateGPIO e verifica no webserver se o name='D5' foi precionado ==1
  } else if ( server.hasArg("D6") ) {
    updateGPIO(1, server.arg("D6")); // chama a função updateGPIO e verifica no webserver se o name='D6' foi precionado ==1
  } else if ( server.hasArg("D7") ) {
    updateGPIO(2, server.arg("D7")); // chama a função updateGPIO e verifica no webserver se o name='D7' foi precionado ==1
  } else if ( server.hasArg("D8") ) {
    updateGPIO(3, server.arg("D8")); // chama a função updateGPIO e verifica no webserver se o name='D8' foi precionado ==1
  } else {
    server.send ( 200, "text/html", buildWebsite() ); // Atualiza a pagina
  }
}

//*****************Setup**********************
void setup() {
  for ( int x = 0 ; x < 5 ; x++ ) {
    pinMode(GPIOPIN[x], OUTPUT); // Array de consulta dos pinos GPIO
  }
  dht.setup(D4, DHTesp::DHT11); // conecta o sensor na porta D4 //DHT11 || DHT22

  Serial.begin ( 115200 ); // impressão do monitor serial 115200 é o ideal para o ESP
  // leitura das variaveis da configuração de connexão
  WiFi.begin ( ssid, password ); // senha do router
  WiFi.config(ip, gateway, subnet);// ip fixo
  // Attente de la connexion au réseau WiFi / Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {//enquanto não conecta mostra as tentativas
    delay ( 500 ); Serial.print ( "." );
  }
  // Connexion WiFi  / WiFi connexion is OK
  Serial.println ( "" );
  Serial.print ( "Connected to " ); Serial.println ( ssid );
  Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );
  //
  SPIFFS_Setup();//recupera os dados da SPIFFS
  //monta o servidor
  server.on ( "/", handleRoot );// recupera a pagina principal
  server.begin(); // inicia o servidor
  Serial.println ( "HTTP server started" );
}
//*****************timers**********************
unsigned long TempoAnterior = 0; // cronometro de leitura do sensor
//*****************Loop*************************
void loop() {
  unsigned long TempoAtual = millis(); // inicia a contagem do tempo
  //funções
  server.handleClient(); // executa o servidor
  check(); // verifica a variavel boolGpio[gpio]
  //leitura dos senres
  if ((unsigned long)(TempoAtual - TempoAnterior) >= 1000) {// A cada 1 segundo faz a leitura do sensor
    // faz a leitura dos sensores
    t = dht.getTemperature(); // temperatura em Célsius
    h = dht.getHumidity(); //umidade
  }
  TempoAnterior = TempoAtual; // zera o cronometro
}
