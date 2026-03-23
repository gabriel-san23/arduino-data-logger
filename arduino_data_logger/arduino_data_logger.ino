#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <EEPROM.h>
#include <RTClib.h>
#include <Wire.h>

#define LOG_OPTION 1 //imprime o log no Serial Monitor de cada loop
#define SERIAL_OPTION 0

//Mapeamento EEPROM
#define ADDR_ANIMACAO 0 // endereço da preferência de animação
#define ADDR_UNIDADE 1 // endereço da preferência de unidade
#define ADDR_PONTEIRO 2 // endereço do ponteiro do log
#define ADDR_LOG_INICIO 3 // primeiro byte da área de log
#define LOG_REGISTROS 50 // quantidade máxima de registros
#define LOG_BYTES 10  // tamanho de cada registros em bytes

#define UTC_OFFSET 0
 
int botao_ok = 3;
int botao_direita = 4;
int botao_esquerda = 5;
int botao_menu = 6;

bool modoFahrenheit = false;
bool animacaoInicial = false;
int tela = 0;

float ultimaTemp = 0;
float ultimaUmid = 0;
int   ultimaLuz  = 0;

int currentAddress = ADDR_LOG_INICIO;
 
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 RTC;
 
// Configurações do DHT22 (lembre que no laboratório temos o DHT-11)
#define DHTPIN 2
#define DHTTYPE DHT11 // Mudar para DHT-11 no laboratório
DHT dht(DHTPIN, DHTTYPE);

// Configurações do LDR
#define LDRPIN A3
 
void setup()
{
  pinMode(LDRPIN, INPUT); 
  pinMode(botao_ok, INPUT);
  pinMode(botao_direita, INPUT);
  pinMode(botao_esquerda, INPUT);
  pinMode(botao_menu, INPUT);
  pinMode(LED_BUILTIN, OUTPUT); // LED embutido teria que piscar ao gravar na EEPROM
  
  Wire.begin(); // inicia o barramento I2C (necessário para LCD e RTC)
  lcd.begin(16, 2);
  dht.begin();
  Serial.begin(9600);
  RTC.begin(); // inicia o módulo RTC
  
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

// Carrega da EEPROM as preferências salvas
  carregarConfiguracoes();

  if (animacaoInicial == true)
    startupScreen();
  lcd.clear();  
}
 
void loop()
{ 
  // Obtém o horário atual do RTC e aplica o fuso horário
  DateTime now = RTC.now();
  DateTime ajustado = DateTime(now.unixtime() + (long)UTC_OFFSET * 3600); // UTC_OFFSET × 3600 segundos para ajustar ao fuso local.

  // Se LOG_OPTION == 1, imprime o conteúdo completo da EEPROM
  if (LOG_OPTION) get_log();

  // Verifica se algum sensor mudou e grava na EEPROM se necessário
  verificaEGrava(ajustado);

  // Imprime data e hora no Serial Monitor se SERIAL_OPTION == 1
  if (SERIAL_OPTION) {
    Serial.print(ajustado.day());   Serial.print("/");
    Serial.print(ajustado.month()); Serial.print("/");
    Serial.print(ajustado.year());  Serial.print(" ");
    Serial.print(ajustado.hour());  Serial.print(":");
    Serial.print(ajustado.minute());Serial.print(":");
    Serial.println(ajustado.second());
  }

//  telaDados();
//  if (tela == 1)
//    telaEscolha(); // Quando vai para a direita
//  else if (tela == -1)
//    telaAnimacao();
//  else if (tela == -2)
//    telaLog();

  if (tela == 0) {
    telaDados(); 
  } 
  else if (tela == 1) {
    telaEscolha();
    tela = 0; // Volta para tela principal ao sair
  } 
  else if (tela == -1) {
    telaAnimacao();
    tela = 0;
  } 
  else if (tela == -2) {
    telaLog();
    tela = 0;
  }
}

void carregarConfiguracoes(){
  byte a = EEPROM.read(ADDR_ANIMACAO);
  byte u = EEPROM.read(ADDR_UNIDADE);
  byte p = EEPROM.read(ADDR_PONTEIRO);

  if (a > 1) { a = 0; EEPROM.write(ADDR_ANIMACAO, 0); }
  if (u > 1) { u = 0; EEPROM.write(ADDR_UNIDADE, 0); }
  if (p >= LOG_REGISTROS) { p = 0; EEPROM.write(ADDR_PONTEIRO, 0); }

  animacaoInicial = (a == 1);
  modoFahrenheit = (u == 1);

  //if (p >= LOG_REGISTROS) p = 0;
  currentAddress = ADDR_LOG_INICIO + (p * LOG_BYTES);
}

void verificaEGrava(DateTime ajustado) {
  float novaTemp = dht.readTemperature();
  float novaUmid = dht.readHumidity();
  int   novaLuz  = analogRead(LDRPIN);

  if (isnan(novaTemp) || isnan(novaUmid)) return;

  if (novaTemp != ultimaTemp || novaUmid != ultimaUmid || novaLuz != ultimaLuz) {

    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);

    int ti = (int)(novaTemp * 100);
    int ui = (int)(novaUmid * 100);

    EEPROM.put(currentAddress, (long)ajustado.unixtime());
    EEPROM.put(currentAddress + 4, ti);
    EEPROM.put(currentAddress + 6, ui);
    EEPROM.put(currentAddress + 8, novaLuz);

    currentAddress += LOG_BYTES;
    if (currentAddress >= ADDR_LOG_INICIO + (LOG_REGISTROS * LOG_BYTES))
      currentAddress = ADDR_LOG_INICIO;

    EEPROM.update(ADDR_PONTEIRO, (currentAddress - ADDR_LOG_INICIO) / LOG_BYTES);

    ultimaTemp = novaTemp;
    ultimaUmid = novaUmid;
    ultimaLuz  = novaLuz;
  }
}

void get_log(){
  Serial.println("=== LOG EEPROM ===");
  Serial.println("Timestamp\t\tTemp(C)\t\tUmidade(%)\tLuz");

  for (int addr = ADDR_LOG_INICIO; addr < ADDR_LOG_INICIO + (LOG_REGISTROS * LOG_BYTES); addr += LOG_BYTES) {
    long ts;
    int  ti, ui, li;

    EEPROM.get(addr, ts);
    EEPROM.get(addr + 4, ti);
    EEPROM.get(addr + 6, ui);
    EEPROM.get(addr + 8, li);

    if (ts != (long)0xFFFFFFFF) {
      DateTime dt = DateTime(ts);
      Serial.print(dt.timestamp(DateTime::TIMESTAMP_FULL));
      Serial.print("\t");
      Serial.print(ti / 100.0);
      Serial.print(" C\t\t");
      Serial.print(ui / 100.0);
      Serial.print(" %\t\t");
      Serial.println(li);
    }
  }
  Serial.println("=== FIM ===");
}

void telaLog() {
  int total = 0;
  for (int i = 0; i < LOG_REGISTROS; i++){
    long ts;
    EEPROM.get(ADDR_LOG_INICIO + (i * LOG_BYTES), ts);
    if (ts != 0 && ts != (long)0xFFFFFFFF) total++;
  }

  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Log: ");
  lcd.print(total);
  lcd.print("/");
  lcd.print(LOG_REGISTROS);
  lcd.setCursor(0,1); lcd.print("Serial: ok");

  get_log();

  while (digitalRead(botao_ok)       == LOW &&
         digitalRead(botao_menu)     == LOW &&
         digitalRead(botao_direita)  == LOW &&
         digitalRead(botao_esquerda) == LOW) {
    delay(50);
  }
  delay(50);
  tela = 0;
}

void startupScreen()
{
  wine1();
  delay(1000);  
  wine2();
  delay(1000);  
  wine3();
  delay(1000);  
  wine4();
  delay(1000);
}

void telaDados() {
  //tela = 0;
  int subTela = 0;
  bool sair = false;
  lcd.clear();

  while (!sair) {
    lcd.setCursor(0, 0);
    if (subTela == 0) {
      float t = dht.readTemperature();
      if (modoFahrenheit) t = (1.8 * t) + 32.0;
      lcd.print("T: "); lcd.print(t);
      lcd.print(modoFahrenheit ? " F  " : " C  ");
    }
    else if (subTela == 1) {
      lcd.print("H: "); lcd.print(dht.readHumidity()); lcd.print(" %  ");
    }
    else {
      lcd.print("*L: "); lcd.print(analogRead(LDRPIN)); lcd.print("    ");
    }

    if (digitalRead(botao_ok) == HIGH) {
      subTela = (subTela + 1) % 3;
      lcd.clear();
      while (digitalRead(botao_ok) == HIGH) delay(10);
      delay(50);
    }
    if (digitalRead(botao_menu) == HIGH) {
      sair = true;
      while (digitalRead(botao_menu) == HIGH) delay(10);
      delay(50);
    }
    if (digitalRead(botao_direita) == HIGH) {
      sair = true; tela++;
      while (digitalRead(botao_direita) == HIGH) delay(10);
      delay(50);
    }
    if (digitalRead(botao_esquerda) == HIGH) {
      sair = true; tela--;
      while (digitalRead(botao_esquerda) == HIGH) delay(10);
      delay(50);
    }
    delay(100);
  }
}
 
void telaAnimacao()
{
  bool sair = false;
  lcd.clear();
  
  while(!sair) {
    lcd.setCursor(0,0);
    lcd.print("Animacao inicial");
    
    lcd.setCursor(0,1);
    if (animacaoInicial) lcd.print("< Ativada     >");
    else lcd.print("< Desativada  >");

    if (digitalRead(botao_esquerda) == HIGH || digitalRead(botao_direita) == HIGH) {
      animacaoInicial = !animacaoInicial; 
      
      EEPROM.update(ADDR_ANIMACAO, animacaoInicial ? 1 : 0);
      while(digitalRead(botao_esquerda) == HIGH || digitalRead(botao_direita) == HIGH) {
        delay(10);
      }
      delay(50);
    }

    if (digitalRead(botao_ok) == HIGH || digitalRead(botao_menu) == HIGH) {
      sair = true;
      
      while(digitalRead(botao_ok) == HIGH || digitalRead(botao_menu) == HIGH) {
        delay(10);
      }
      delay(50);
    }
  }
}

void telaEscolha()
{
  bool sair = false;
  lcd.clear();
  
  while(!sair) {
    lcd.setCursor(0,0);
    lcd.print("Unidade Temp:");
    
    lcd.setCursor(0,1);
    if (modoFahrenheit) lcd.print("< Fahrenheit >  ");
    else lcd.print("< Celsius    >  ");

    if (digitalRead(botao_esquerda) == HIGH || digitalRead(botao_direita) == HIGH) {
      modoFahrenheit = !modoFahrenheit; 
      
      EEPROM.update(ADDR_UNIDADE, modoFahrenheit ? 1 : 0);
      while(digitalRead(botao_esquerda) == HIGH || digitalRead(botao_direita) == HIGH) {
        delay(10);
      }
      delay(50);
    }

    if (digitalRead(botao_ok) == HIGH || digitalRead(botao_menu) == HIGH) {
      sair = true;
      
      while(digitalRead(botao_ok) == HIGH || digitalRead(botao_menu) == HIGH) {
        delay(10);
      }
      delay(50);
    }
  }
}

void wine1() {
lcd.clear();
byte image24[8] = {B10001, B10001, B10001, B10001, B10001, B10001, B01110, B00000};
byte image08[8] = {B00000, B01110, B10001, B01010, B01010, B10001, B10001, B10001};
byte image25[8] = {B10001, B10001, B01010, B00100, B00100, B00100, B01110, B00000};
byte image09[8] = {B00000, B00000, B00000, B00000, B00000, B11011, B10101, B10001};
lcd.createChar(0, image24);
lcd.createChar(1, image08);
lcd.createChar(2, image25);
lcd.createChar(3, image09);
lcd.setCursor(7, 1);
lcd.write(byte(0));
lcd.setCursor(7, 0);
lcd.write(byte(1));
lcd.setCursor(8, 1);
lcd.write(byte(2));
lcd.setCursor(8, 0);
lcd.write(byte(3));
}

void wine2() {
lcd.clear();
byte image25[8] = {B10001, B10001, B01010, B00100, B00100, B00100, B01110, B00000};
byte image09[8] = {B00000, B00000, B00000, B00000, B00000, B11011, B10101, B10001};
byte image22[8] = {B00000, B00001, B00010, B00100, B00100, B00011, B00000, B00000};
byte image23[8] = {B10001, B00010, B00100, B01000, B10000, B00000, B00000, B00000};
byte image08[8] = {B00000, B01100, B01000, B10001, B00011, B00100, B01000, B10000};
byte image07[8] = {B00000, B00000, B00000, B00000, B00001, B00010, B00100, B01000};
lcd.createChar(0, image25);
lcd.createChar(1, image09);
lcd.createChar(2, image22);
lcd.createChar(3, image23);
lcd.createChar(4, image08);
lcd.createChar(5, image07);
lcd.setCursor(8, 1);
lcd.write(byte(0));
lcd.setCursor(8, 0);
lcd.write(byte(1));
lcd.setCursor(5, 1);
lcd.write(byte(2));
lcd.setCursor(6, 1);
lcd.write(byte(3));
lcd.setCursor(7, 0);
lcd.write(byte(4));
lcd.setCursor(6, 0);
lcd.write(byte(5));
}

void wine3() {
lcd.clear();
byte image25[8] = {B10001, B10001, B01010, B00100, B00100, B00100, B01110, B00000};
byte image09[8] = {B00000, B00000, B11000, B00100, B00000, B11011, B10101, B10001};
byte image08[8] = {B10010, B01101, B00001, B01101, B10010, B00000, B00000, B00000};
byte image07[8] = {B11111, B00000, B00000, B00000, B11111, B00000, B00000, B00000};
byte image06[8] = {B00011, B00100, B00100, B00100, B00011, B00000, B00000, B00000};
lcd.createChar(0, image25);
lcd.createChar(1, image09);
lcd.createChar(2, image08);
lcd.createChar(3, image07);
lcd.createChar(4, image06);
lcd.setCursor(8, 1);
lcd.write(byte(0));
lcd.setCursor(8, 0);
lcd.write(byte(1));
lcd.setCursor(7, 0);
lcd.write(byte(2));
lcd.setCursor(6, 0);
lcd.write(byte(3));
lcd.setCursor(5, 0);
lcd.write(byte(4));
}

void wine4() {
lcd.clear();
byte image25[8] = {B11111, B11111, B01110, B00100, B00100, B00100, B01110, B00000};
byte image09[8] = {B00000, B00000, B00000, B00000, B00000, B11011, B10101, B10001};
byte image10[8] = {B00000, B00000, B10001, B10001, B10001, B10101, B01010, B00000};
byte image11[8] = {B00000, B00000, B00100, B00100, B00100, B00100, B00100, B00000};
byte image12[8] = {B00000, B00000, B10001, B11001, B10101, B10011, B10001, B00000};
byte image13[8] = {B00000, B00000, B11110, B10000, B11100, B10000, B11110, B00000};
byte image26[8] = {B00000, B11110, B10000, B10000, B10000, B10000, B11110, B00000};
byte image27[8] = {B00000, B01100, B10010, B10010, B10010, B10010, B01101, B00000};
lcd.createChar(0, image25);
lcd.createChar(1, image09);
lcd.createChar(2, image10);
lcd.createChar(3, image11);
lcd.createChar(4, image12);
lcd.createChar(5, image13);
lcd.createChar(6, image26);
lcd.createChar(7, image27);
lcd.setCursor(8, 1);
lcd.write(byte(0));
lcd.setCursor(8, 0);
lcd.write(byte(1));
lcd.setCursor(9, 0);
lcd.write(byte(2));
lcd.setCursor(10, 0);
lcd.write(byte(3));
lcd.setCursor(11, 0);
lcd.write(byte(4));
lcd.setCursor(12, 0);
lcd.write(byte(5));
lcd.setCursor(9, 1);
lcd.write(byte(6));
lcd.setCursor(10, 1);
lcd.write(byte(7));
}
