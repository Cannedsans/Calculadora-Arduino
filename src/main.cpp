#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define I2C_ADDRESS 0x27
#define DEBOUNCE_DELAY 50

LiquidCrystal_I2C tela(I2C_ADDRESS, 16, 2);

const String teclado[4][4] = {
    {"7", "8", "9", "/"},
    {"4", "5", "6", "*"},
    {"1", "2", "3", "-"},
    {"00", "0", "+", "="}};

const int colunas[4] = {4, 5, 6, 7};
const int linhas[4] = {8, 9, 10, 11};

float calcularMultiplasOperacoes(String expr);
unsigned long ultimaLeitura = 0;
String expressao = "";

void setup()
{
  for (int i = 0; i < 4; i++)
  {
    pinMode(colunas[i], OUTPUT);
    digitalWrite(colunas[i], HIGH);
  }

  for (int i = 0; i < 4; i++)
  {
    pinMode(linhas[i], INPUT_PULLUP);
  }

  tela.begin(16, 2);
  tela.backlight();
  tela.cursor_on();
  tela.blink_on();
  tela.clear();
}

void loop()
{
  // Verifica se já passou tempo suficiente desde a última leitura
  if (millis() - ultimaLeitura > DEBOUNCE_DELAY)
  {

    bool teclaDetectada = false;

    for (int col = 0; col < 4; col++)
    {
      digitalWrite(colunas[col], LOW);

      for (int lin = 0; lin < 4; lin++)
      {
        if (digitalRead(linhas[lin]) == LOW)
        {
          // Debounce: espera até que a tecla seja solta
          while (digitalRead(linhas[lin]) == LOW)
          {
            delay(1);
          }

          String tecla = teclado[lin][col];

          // Processa a tecla pressionada
          if (tecla == "00")
          {
            expressao = "";
            tela.clear();
          }
          else if (tecla == "=")
          {
            // Calcula e exibe o resultado na segunda linha
            float resultado = calcularMultiplasOperacoes(expressao);
            tela.setCursor(0, 1);
            tela.print("= ");
            tela.print(resultado);
          }
          else
          {
            expressao += tecla;
            tela.print(tecla);
          }

          teclaDetectada = true;
          break; // Sai do loop de linhas após encontrar uma tecla
        }
      }

      digitalWrite(colunas[col], HIGH);
      if (teclaDetectada)
        break; // Sai do loop de colunas após encontrar uma tecla
    }

    ultimaLeitura = millis();
  }
}

float calcularMultiplasOperacoes(String expr)
{
  expr.replace(" ", "");

  // Primeiro processa * e /
  for (int i = 0; i < expr.length(); i++)
  {
    if (expr[i] == '*' || expr[i] == '/')
    {
      float a = expr.substring(0, i).toFloat();
      float b = expr.substring(i + 1).toFloat();
      float res = (expr[i] == '*') ? a * b : (b != 0 ? a / b : 0);
      expr = String(res) + expr.substring(i + 2);
      i = -1; // Reinicia o loop
    }
  }

  // Depois processa + e -
  for (int i = 0; i < expr.length(); i++)
  {
    if (expr[i] == '+' || expr[i] == '-')
    {
      float a = expr.substring(0, i).toFloat();
      float b = expr.substring(i + 1).toFloat();
      float res = (expr[i] == '+') ? a + b : a - b;
      expr = String(res) + expr.substring(i + 2);
      i = -1; // Reinicia o loop
    }
  }

  return expr.toFloat();
}