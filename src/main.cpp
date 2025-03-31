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
  {"00", "0", "+", "="}
};

const int colunas[4] = {4, 5, 6, 7};
const int linhas[4] = {8, 9, 10, 11};

float calcularMultiplasOperacoes(String expr);
String expressao = "";
bool resultadoCalculado = false;
unsigned long ultimaLeitura = 0;

void setup() {
  // Configura os pinos dos botões
  for (int i = 0; i < 4; i++){
    pinMode(colunas[i], OUTPUT);
    digitalWrite(colunas[i], HIGH);
  }
  for (int i = 0; i < 4; i++){
    pinMode(linhas[i], INPUT_PULLUP);
  }
  
  tela.begin(16, 2);
  tela.backlight();
  tela.cursor_on();
  tela.blink_on();
  tela.clear();
}

void loop() {
  if (millis() - ultimaLeitura > DEBOUNCE_DELAY) {
    bool teclaDetectada = false;
    for (int col = 0; col < 4; col++){
      digitalWrite(colunas[col], LOW);
      for (int lin = 0; lin < 4; lin++){
        if(digitalRead(linhas[lin]) == LOW){
          // Aguarda a liberação da tecla
          while(digitalRead(linhas[lin]) == LOW){
            delay(1);
          }
          String tecla = teclado[lin][col];

          // Processamento da tecla pressionada
          if (tecla == "00") {
            if (resultadoCalculado) {
              // Se o resultado já foi calculado, apaga somente a segunda linha (resultado)
              resultadoCalculado = false;
              tela.setCursor(0, 1);
              tela.print("                "); // limpa a linha de resultado
            } else {
              // Se não, remove o último caractere da expressão
              if (expressao.length() > 0) {
                expressao.remove(expressao.length() - 1);
                // Atualiza a linha de expressão
                tela.clear();
                tela.setCursor(0, 0);
                tela.print(expressao);
              }
            }
          } 
          else if (tecla == "=") {
            // Calcula e exibe o resultado na segunda linha
            float resultado = calcularMultiplasOperacoes(expressao);
            tela.setCursor(0, 1);
            tela.print("= ");
            tela.print(resultado);
            resultadoCalculado = true;
          } 
          else {
            // Se o resultado já foi calculado e o usuário pressiona uma nova tecla, reinicia a expressão
            if (resultadoCalculado) {
              expressao = "";
              tela.clear();
              resultadoCalculado = false;
            }
            expressao += tecla;
            // Atualiza a exibição da expressão na primeira linha
            tela.clear();
            tela.setCursor(0, 0);
            tela.print(expressao);
          }

          teclaDetectada = true;
          break; // Sai do loop de linhas
        }
      }
      digitalWrite(colunas[col], HIGH);
      if (teclaDetectada) break; // Sai do loop de colunas
    }
    ultimaLeitura = millis();
  }
}

float calcularMultiplasOperacoes(String expr) {
  // Remove espaços, se houver
  expr.replace(" ", "");

  // Tokenização: separa números e operadores em um array
  const int MAX_TOKENS = 20;
  String tokens[MAX_TOKENS];
  int tokenCount = 0;
  String currentToken = "";
  for (int i = 0; i < expr.length(); i++){
    char c = expr.charAt(i);
    if ((c >= '0' && c <= '9') || c == '.') {
      currentToken += c;
    } else if (c == '+' || c == '-' || c == '*' || c == '/') {
      // Trata sinal negativo: se o '-' for o primeiro caractere ou seguir outro operador, faz parte do número
      if (c == '-' && (i == 0 || (expr.charAt(i - 1) == '+' || expr.charAt(i - 1) == '-' || expr.charAt(i - 1) == '*' || expr.charAt(i - 1) == '/'))) {
        currentToken += c;
      } else {
        if (currentToken.length() > 0) {
          tokens[tokenCount++] = currentToken;
          currentToken = "";
        }
        tokens[tokenCount++] = String(c);
      }
    }
  }
  if (currentToken.length() > 0) {
    tokens[tokenCount++] = currentToken;
  }
  
  // Primeira passagem: resolve multiplicação e divisão
  for (int i = 0; i < tokenCount; i++){
    if (tokens[i] == "*" || tokens[i] == "/"){
      float a = tokens[i - 1].toFloat();
      float b = tokens[i + 1].toFloat();
      float res = 0;
      if (tokens[i] == "*") {
        res = a * b;
      } else {
        res = (b != 0) ? a / b : 0;
      }
      tokens[i - 1] = String(res);
      // Realoca os tokens (remove os tokens i e i+1)
      for (int j = i; j < tokenCount - 2; j++){
        tokens[j] = tokens[j + 2];
      }
      tokenCount -= 2;
      i = -1; // Reinicia o loop para garantir que todas as operações de * e / sejam processadas
    }
  }
  
  // Segunda passagem: resolve adição e subtração
  float result = tokens[0].toFloat();
  for (int i = 1; i < tokenCount; i += 2){
    String op = tokens[i];
    float operand = tokens[i + 1].toFloat();
    if (op == "+") {
      result += operand;
    } else if (op == "-") {
      result -= operand;
    }
  }
  
  return result;
}