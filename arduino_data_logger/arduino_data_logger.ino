// Incluindo bibliotecas
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <EEPROM.h>
#include <RTClib.h>
#include <Wire.h>

// Mapeamento e Configurações
#define ADDR_ANIMACAO 0 // endereço da preferência de animação
#define ADDR_UNIDADE 1 // endereço da preferência de unidade
#define ADDR_PONTEIRO 2 // endereço do ponteiro do log
#define ADDR_LOG_INICIO 3 // primeiro byte da área de log

#define LOG_REGISTROS 50 // quantidade máxima de registros
#define LOG_BYTES 10  // tamanho de cada registros em bytes
#define LOG_OPTION 1 // permite imprimir o log no Serial Monitor

// Configuração dos Pinos
const int botao_ok = 3;
const int botao_direita = 4;
const int botao_esquerda = 5;
const int botao_menu = 6;
#define DHTPIN 2 // Configuração do DHT
#define DHTTYPE DHT22 // !! Mudar para DHT-11 no laboratório !!
#define LDRPIN A3 // Configuração do LDR

// Variáveis globais
int utc_offset = 0; // diferença de fuso horário (Brasília:UTC-3)
bool modoFahrenheit = false;
bool animacaoInicial = false;
int tela = 0;
int subTela = 0;

float ultimaTemp = 0;
float ultimaUmid = 0;
int   ultimaLuz  = 0;
int currentAddress = ADDR_LOG_INICIO;

// Configurações de intervalos (função millis)
unsigned long anteriorMillis = 0;
unsigned long lastDebounceTime = 0;

const long intervalo = 2000; // Tenta gravar a cada 2 segundos
const int debounceDelay = 250;

// Criando Instâncias
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 RTC;
DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  pinMode(LDRPIN, INPUT); 
  pinMode(botao_ok, INPUT);
  pinMode(botao_direita, INPUT);
  pinMode(botao_esquerda, INPUT);
  pinMode(botao_menu, INPUT);
  pinMode(LED_BUILTIN, OUTPUT); // LED embutido pisca ao gravar na EEPROM
  
  Wire.begin(); // inicia o barramento I2C (necessário para LCD e RTC)
  lcd.begin(16, 2);
  dht.begin();
  RTC.begin();
  Serial.begin(9600);
  
  //RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  /*
   O código acima define a hora do RTC para o 
   horário do sistema em que você compilou o sketch.

   Como o tempo de compilação é fixo, 
   se mantermos o comando na função setup(), 
   o RTC voltará ao tempo de compilação 
   sempre que o Arduino reinicia.

   Rode este código apenas uma vez para definir o tempo e comente a linha. 
  */

  // Carrega da EEPROM as preferências salvas
  carregarConfiguracoes();

  // Animação de abertura
  if (animacaoInicial)
    startupScreenAnimation();
  lcd.clear();
}

void loop()
{
  unsigned long atualMillis = millis();

  if (atualMillis - anteriorMillis >= intervalo) {
    anteriorMillis = atualMillis;

    // Obtém o horário atual do RTC e aplica o fuso horário
    DateTime now = RTC.now();
    DateTime horaAjustada = DateTime(now.unixtime() + (long)utc_offset * 3600); 
    // UTC_OFFSET × 3600 segundos para ajustar ao fuso local.

    // Verifica se algum sensor mudou e grava na EEPROM se necessário
    verificaEGrava(horaAjustada);

    // Se LOG_OPTION == 1, imprime o conteúdo completo da EEPROM
    if (LOG_OPTION) get_log();
  }

  verificarBotoes();

  switch (tela) {
    case 0: exibirTelaDados();     break;
    case 1: exibirTelaEscolha();   break;
    case 2: exibirTelaAnimacao();  break;
    case 3: exibirTelaLog();       break;
  }
}

void telaDados() {

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

  if (digitalRead(botao_ok) == HIGH &&
    (millis() - lastDebounceTime > debounceDelay)) {

    subTela = (subTela + 1) % 3;
    lcd.clear();
    lastDebounceTime = millis();
  }
}

void telaLog() {
  // contabiliza quantos registros válidos possuem na EEPROM
  int qtdRegistros = 0;
  for (int i = 0; i < LOG_REGISTROS; i++){
    long ts;
    EEPROM.get(ADDR_LOG_INICIO + (i * LOG_BYTES), ts);
    if (ts != 0 && ts != (long)0xFFFFFFFF) qtdRegistros++;
  }

  // exibir quantidade em relação ao total
  lcd.setCursor(0,0);
  lcd.print("Log: ");
  lcd.print(qtdRegistros); lcd.print("/"); lcd.print(LOG_REGISTROS);

  lcd.setCursor(0,1);
  lcd.print("Serial: ok");
}
 
void telaAtivarAnimacao()
{
  lcd.setCursor(0,0);
  lcd.print("Animacao inicial");
  
  lcd.setCursor(0,1);
  if (animacaoInicial) lcd.print("< Ativada     >");
  else lcd.print("< Desativada  >");

  // alterar estado da animação
  if (digitalRead(botao_ok) == HIGH &&
    (millis() - lastDebounceTime > debounceDelay)) {

    animacaoInicial = !animacaoInicial;
    EEPROM.update(ADDR_ANIMACAO, animacaoInicial ? 1 : 0);
    lastDebounceTime = millis();
  }  
}

void telaUnidadeTemp()
{
  lcd.setCursor(0,0);
  lcd.print("Unidade Temp:");  
  lcd.setCursor(0,1);
  lcd.print(modoFahrenheit ? "< Fahrenheit >" : "< Celsius    >");

  // alterar unidade de temperatura
  if (digitalRead(botao_ok) == HIGH && 
    (millis() - lastDebounceTime > debounceDelay)) {

    modoFahrenheit = !modoFahrenheit;    
    EEPROM.update(ADDR_UNIDADE, modoFahrenheit ? 1 : 0);
    lastDebounceTime = millis();
  }
}

void verificarBotoes() {
  // Sistema simples de debounce por tempo
  if ((millis() - lastDebounceTime) < debounceDelay) return;

  // BOTÃO DIREITA (Avança tela)
  if (digitalRead(botao_direita) == HIGH) {
    tela++;
    if (tela > 3) tela = 0; // Cicla entre as telas
    lcd.clear();
    lastDebounceTime = millis();
  }

  // BOTÃO ESQUERDA (Volta tela)
  if (digitalRead(botao_esquerda) == HIGH) {
    tela--;
    if (tela < 0) tela = 3; // Cicla entre as telas
    lcd.clear();
    lastDebounceTime = millis();
  }

  // BOTÃO MENU (Pode servir para resetar para a tela principal)
  if (digitalRead(botao_menu) == HIGH) {
    tela = 0;
    lcd.clear();
    lastDebounceTime = millis();
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

void verificaEGrava(DateTime horarioAtual) {
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

    EEPROM.put(currentAddress, (long)horarioAtual.unixtime());
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

// Animação da tela inicial
void startupScreenAnimation()
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