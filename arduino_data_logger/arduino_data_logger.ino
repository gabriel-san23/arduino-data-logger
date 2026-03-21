#include <LiquidCrystal_I2C.h>
#include "DHT.h"
 
int botao_ok = 3;
int botao_direita = 4;
int botao_esquerda = 5;
int botao_menu = 6;
 
bool modoFahrenheit = false;
 
LiquidCrystal_I2C lcd(0x27, 16, 2);
 
// Configurações do DHT22 (lembre que no laboratório temos o DHT-11)
#define DHTPIN 2
#define DHTTYPE DHT22 //Mudar para DHT-11 no laboratório
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
 
  lcd.clear();  
}
 
void loop()
{
  telaDados();
  telaEscolha();
}
 
void telaDados()
{
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
 
    if (digitalRead(botao_direita) == HIGH || digitalRead(botao_menu) == HIGH)
    {
      sair = true;
     
      while(digitalRead(botao_direita) == HIGH || digitalRead(botao_menu) == HIGH) {
        delay(10);
      }
      delay(50);
    }
    delay(100);
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
