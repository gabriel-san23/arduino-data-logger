#include <LiquidCrystal_I2C.h>
#include "DHT.h"
 
int botao_ok = 3;
int botao_direita = 4;
int botao_esquerda = 5;
int botao_menu = 6;

bool modoFahrenheit = false; 
bool animacaoInicial = false;
int tela = 0;
 
LiquidCrystal_I2C lcd(0x27, 16, 2);
 
// Configurações do DHT22 (lembre que no laboratório temos o DHT-11)
#define DHTPIN 2
#define DHTTYPE DHT22 // Mudar para DHT-11 no laboratório
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
 
  lcd.begin(16, 2);
  dht.begin();
  if (animacaoInicial == true)
    startupScreen();
  lcd.clear();  
}
 
void loop()
{
  telaDados();
  if (tela == 1)
    telaEscolha(); // Quando vai para a direita
  else if (tela == -1)
    telaAnimacao();
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
void telaDados()
{
  tela = 0;
  int subTela = 0; 
  bool sair = false;
 
  lcd.clear(); 
 
  while (!sair)
  {
    if (subTela == 0)
    {
      float temperature = dht.readTemperature();
      if (modoFahrenheit) {
        temperature = (1.8 * temperature) + 32.0; 
      }

      lcd.setCursor(0,0);
      lcd.print("T: ");
      lcd.print(temperature);
      
      if (modoFahrenheit) lcd.print(" F  "); 
      else lcd.print(" C  "); 
    }
    else if (subTela == 1)
    {
      float humidity = dht.readHumidity();
      lcd.setCursor(0,0);
      lcd.print("H: ");
      lcd.print(humidity);
      lcd.print(" %  ");
    }
    else if (subTela == 2)
    {      
      int lighting = analogRead(LDRPIN);
      lcd.setCursor(0,0);
      lcd.print("*L: ");
      lcd.print(lighting);
      lcd.print("    ");
    }
 
    if (digitalRead(botao_ok) == HIGH)
    {
      subTela++;
      if (subTela > 2) {
        subTela = 0; 
      }
      lcd.clear(); 
      
      while(digitalRead(botao_ok) == HIGH) {
        delay(10);
      }
      delay(50);
    }
 
    // PARA O BOTÃO DO MEIO
    if (digitalRead(botao_menu) == HIGH)
    {
      sair = true;

      while(digitalRead(botao_menu) == HIGH) {
        delay(10);
      }
      delay(50); 
    }
    delay(100); 

    // PARA O BOTÃO DA DIREITA
    if (digitalRead(botao_direita) == HIGH)
    {
      sair = true;
      tela++;
      
      while(digitalRead(botao_direita) == HIGH) {
        delay(10);
      }
      delay(50); 
    }
    delay(100); 

    // PARA O BOTÃO DA ESQUERDA
    if (digitalRead(botao_esquerda) == HIGH)
    {
      sair = true;
      tela--;
      
      while(digitalRead(botao_esquerda) == HIGH) {
        delay(10);
      }
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
