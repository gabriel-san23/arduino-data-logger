#include <LiquidCrystal_I2C.h> 
#include "DHT.h"

int botao_ok = 3; 
int botao_direita = 4; 
int botao_esquerda = 5; 
int botao_menu = 6;
int escolha = 0; 

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Configurações do DHT22 (lembre que no laboratório tempos o DHT-11)
#define DHTPIN 2
#define DHTTYPE DHT22 //Mudar para DHT-11 no laboratório
DHT dht(DHTPIN, DHTTYPE);

void setup() 
{ 
  pinMode(botao_ok, INPUT); 
  pinMode(botao_direita, INPUT); 
  pinMode(botao_esquerda, INPUT); 
  pinMode(botao_menu, INPUT);
  
  lcd.begin(16, 2); 
  dht.begin();
  
  lcd.clear();  
  atualizaVisor(); 
} 

void loop() 
{ 
    
  do  
  { 
    if (digitalRead(botao_direita) == HIGH) 
    { 
      escolha++; 
      // Trava no limite máximo de opções (2)
      if (escolha > 2) {
        escolha = 2; 
      }
      atualizaVisor();     
      delay(250); 
    } 

    if (digitalRead(botao_esquerda) == HIGH) 
    { 
      escolha--; 
      // Trava no limite mínimo (0)
      if (escolha < 0) {
        escolha = 0; 
      }
      atualizaVisor(); 
      delay(250); 
    } 
    
    if (digitalRead(botao_menu) == HIGH)   
    {
      escolha = 0; 
      atualizaVisor(); 
      delay(250); 
    }
  } while (digitalRead(botao_ok) != HIGH); 

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Selecionado!"); 
  delay(3000); 
  atualizaVisor(); 
} 

void atualizaVisor()
{
  lcd.clear(); 
  lcd.setCursor(0,0); 
  
  if (escolha == 0)
  {
    // Ler os valores de temperatura e umidade
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    lcd.setCursor(0,0);
    lcd.print("H: ");
    lcd.setCursor(4,0);
    lcd.print(humidity);
    lcd.setCursor(0,1);
    lcd.print("T: ");
    lcd.setCursor(4,1);
    lcd.print(temperature);
   
  }
  else if (escolha == 1) 
  {
    lcd.print("Fahrenheit F"); 
    lcd.setCursor(0,1); 
    lcd.print("     <= OK =>  ");
  }  

  else if (escolha == 2) 
  {
    lcd.print("Celsius C"); 
    lcd.setCursor(0,1); 
    lcd.print("     <= OK     ");
  }  
}
