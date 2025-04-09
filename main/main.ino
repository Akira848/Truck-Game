#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(32, 16, 2);

// Definição dos botões
const int btnMove = 2;       // Mover seleção
const int btnConfirm = 3;    // Confirmar resposta
const int btnMath = 4;       // Modo Matemática
const int btnPortuguese = 5; // Modo Português


byte caminhaoCima[8] = {
  B00000,
  B11110,
  B11111,
  B01010,
  B00000,
  B00000,
  B00000,
  B00000
};

byte caminhaoBaixo[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11110,
  B11111,
  B01010,
  B00000
};

// Variáveis do jogo
enum GameMode {MODE_MATH, MODE_PORTUGUESE};
GameMode currentMode = MODE_MATH;

int fase = 1;
int rodada = 1;
int pontos = 0;
bool aguardandoResposta = false;
unsigned long tempoInicioRodada;
bool tempoEsgotado = false;

// Variáveis para modo matemática
int respostaCorreta = 0;
int respostaErrada = 0;
bool respostaSelecionada = true;

// Variáveis para modo português
String palavraCorreta;
String palavraErrada;
const String palavras3Letras[10] = {"SOL", "PE", "MAR", "LUA", "OVO", "PAO", "UVA", "ZIP", "FIM", "GEL"};
const String palavras4Letras[10] = {"CASA", "PATO", "LUPA", "BOLA", "TREM", "FOGO", "GATO", "RATO", "OSSO", "VELA"};
const String palavras5Letras[10] = {"CARRO", "BARCO", "LARVA", "TIGRE", "DRAGO", "CHUVA", "GIRAR", "HORTA", "JOGAR", "LIXO"};

void setup() {
  // Configuração dos botões
  pinMode(btnMove, INPUT_PULLUP);
  pinMode(btnConfirm, INPUT_PULLUP);
  pinMode(btnMath, INPUT_PULLUP);
  pinMode(btnPortuguese, INPUT_PULLUP);
  
  // Inicialização do LCD
  lcd.begin(16,2); // Pode ser desnecessário com I2C
  lcd.init();      // Correto para inicializar displays I2C
  lcd.backlight(); // Liga a luz de fundo
 
  
  // Mensagem de boas-vindas
  mostrarMensagemBoasVindas();
  
  // Carregar caracteres customizados
  lcd.createChar(0, caminhaoCima);
  lcd.createChar(1, caminhaoBaixo);
  
  randomSeed(analogRead(0));
}

void loop() {
  verificarSelecaoModo();
  
  if (!aguardandoResposta && fase <= (currentMode == MODE_MATH ? 4 : 3)) {
    if (rodada <= 10) {
      gerarPergunta();
      aguardandoResposta = true;
      tempoInicioRodada = millis();
      tempoEsgotado = false;
    } else {
      passarParaProximaFase();
    }
  }
  
  if (aguardandoResposta && !tempoEsgotado) {
    atualizarTimer();
    verificarBotoesJogo();
  }
  
  delay(100);
}

// ===== FUNÇÕES DO JOGO ===== //

void mostrarMensagemBoasVindas() {
  lcd.createChar(0, caminhaoCima);  // Parte superior do caminhão
  lcd.createChar(1, caminhaoBaixo);
  lcd.clear();
  lcd.setCursor(3, 0);
  /*lcd.print("BEM VINDO");
  lcd.setCursor(1, 1);
  lcd.print("CAMINHONEIRO");
  delay(2000);
  lcd.clear();*/
  lcd.setCursor(0, 0);
  lcd.write(byte(0));  // Caminhão superior esquerdo
  lcd.print("   BEM VINDO  ");
  lcd.write(byte(0));  // Caminhão superior direito
  
  lcd.setCursor(0, 1);
  lcd.write(byte(1));  // Caminhão inferior esquerdo
  lcd.print(" CAMINHONEIRO ");
  lcd.write(byte(1));  
  
  delay(5000);
}

void mostrarTelaInicial() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(currentMode == MODE_MATH ? " Jogo Matematica" : " Jogo Portugues");
  lcd.setCursor(0, 1);
  lcd.print(" Pressione START");
  
  while(digitalRead(btnMove) == HIGH && digitalRead(btnConfirm) == HIGH);
  delay(200);
}

void verificarSelecaoModo() {
  if (digitalRead(btnMath) == LOW) {
    currentMode = MODE_MATH;
    reiniciarJogo();
    lcd.clear();
    lcd.print(" Modo Matematica");
    delay(1000);
  }
  
  if (digitalRead(btnPortuguese) == LOW) {
    currentMode = MODE_PORTUGUESE;
    reiniciarJogo();
    lcd.clear();
    lcd.print(" Modo Portugues");
    delay(1000);
  }
}

void reiniciarJogo() {
  fase = 1;
  rodada = 1;
  pontos = 0;
  aguardandoResposta = false;
  mostrarTelaInicial();
}

// ===== MODO MATEMÁTICA ===== //

void gerarPerguntaMatematica() {
  lcd.clear();
  lcd.setCursor(0, 0);

  
  int num1, num2;
  char operador;
  
  switch(fase) {
    case 1: // Soma (60s)
      num1 = random(5, 50);
      num2 = random(5, 50);
      respostaCorreta = num1 + num2;
      operador = '+';
      break;
      
    case 2: // Subtração (75s)
      num1 = random(20, 100);
      num2 = random(5, num1-4);
      respostaCorreta = num1 - num2;
      operador = '-';
      break;
      
    case 3: // Multiplicação (90s)
      num1 = random(2, 12);
      num2 = random(2, 12);
      respostaCorreta = num1 * num2;
      operador = 'x';
      break;
      
    case 4: // Divisão (105s)
      num2 = random(2, 10);
      respostaCorreta = random(2, 10);
      num1 = num2 * respostaCorreta;
      operador = '/';
      break;
  }
  
  respostaErrada = gerarRespostaErrada(respostaCorreta);
  mostrarContaMatematica(num1, operador, num2);
  respostaSelecionada = random(0, 2) == 0;
  mostrarSelecao();
}

int gerarRespostaErrada(int correta) {
  int erro;
  do {
    int variacao = random(1, 6);
    erro = correta + (random(0, 2) ? variacao : -variacao);
  } while (erro == correta || erro <= 0);
  return erro;
}

void mostrarContaMatematica(int num1, char operador, int num2) {
  lcd.setCursor(9,0);
  lcd.print(num1);
  lcd.print(operador);
  lcd.print(num2);
  lcd.print("=?");
}

// ===== MODO PORTUGUÊS ===== //

void gerarPerguntaPortugues() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  int index;
  switch(fase) {
    case 1: // 3 letras (45s)
      index = random(0, 10);
      palavraCorreta = palavras3Letras[index];
      palavraErrada = gerarPalavraErrada(palavraCorreta, 3);
      break;
      
    case 2: // 4 letras (60s)
      index = random(0, 10);
      palavraCorreta = palavras4Letras[index];
      palavraErrada = gerarPalavraErrada(palavraCorreta, 4);
      break;
      
    case 3: // 5 letras (75s)
      index = random(0, 10);
      palavraCorreta = palavras5Letras[index];
      palavraErrada = gerarPalavraErrada(palavraCorreta, 5);
      break;
  }
  
  mostrarPalavras();
  respostaSelecionada = random(0, 2) == 0;
  mostrarSelecao();
}

String gerarPalavraErrada(String correta, int tamanho) {
  String errada;
  do {
    errada = "";
    for (int i = 0; i < tamanho; i++) {
      errada += (char)random('A', 'Z' + 1);
    }
  } while (errada == correta);
  return errada;
}

void mostrarPalavras() {
  lcd.setCursor(2, 1);
  lcd.print(" ");
  lcd.print(respostaSelecionada ? palavraCorreta : palavraErrada);
  lcd.setCursor(8, 1);
  lcd.print(" ");
  lcd.print(respostaSelecionada ? palavraErrada : palavraCorreta);
}

// ===== FUNÇÕES COMUNS ===== //

void gerarPergunta() {
  if (currentMode == MODE_MATH) {
    gerarPerguntaMatematica();
  } else {
    gerarPerguntaPortugues();
  }
}

void mostrarSelecao() {
  if (currentMode == MODE_MATH) {
    lcd.setCursor(2, 1);
    lcd.print(respostaSelecionada ? respostaCorreta : respostaErrada);
    lcd.setCursor(10, 1);
    lcd.print(respostaSelecionada ? respostaErrada : respostaCorreta);
  }
  
  lcd.setCursor(respostaSelecionada ? 0 : 8, 1);
  lcd.write(respostaSelecionada ? 0 : 1);
}

void atualizarTimer() {
  unsigned long tempoAtual = millis() - tempoInicioRodada;
  unsigned long tempoLimite;
  
  if (currentMode == MODE_MATH) {
    unsigned long temposMath[4] = {60000, 75000, 90000, 105000};
    tempoLimite = temposMath[fase - 1];
  } else {
    unsigned long temposPortugues[3] = {45000, 60000, 75000};
    tempoLimite = temposPortugues[fase - 1];
  }

  if (tempoAtual >= tempoLimite) {
    tempoEsgotado = true;
    lcd.setCursor(0, 0);
    lcd.print("0 ");
    respostaSelecionada = false;
    verificarResposta();
  } else {
    lcd.setCursor(0, 0);
    lcd.print("");
    lcd.print((tempoLimite - tempoAtual) / 1000);
    lcd.print(" ");
  }
}

void verificarBotoesJogo() {
  if (digitalRead(btnMove) == LOW) {
    respostaSelecionada = !respostaSelecionada;
    if (currentMode == MODE_PORTUGUESE) {
      mostrarPalavras();
    }
    mostrarSelecao();
    delay(300);
  }
  
  if (digitalRead(btnConfirm) == LOW) {
    verificarResposta();
    delay(500);
  }
}

void verificarResposta() {
  aguardandoResposta = false;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if (tempoEsgotado) {
    lcd.print(" TEMPO ESGOTADO!");
  } else if (respostaSelecionada) {
    lcd.print("Resposta Correta");
    pontos++;
  } else {
    lcd.print("Resposta Errada");
  }
  
  lcd.setCursor(5, 1);
  lcd.print("Pts:");
  lcd.print(pontos);
  
  rodada++;
  delay(2000);
}

void passarParaProximaFase() {
  fase++;
  rodada = 1;
  
  int maxFases = (currentMode == MODE_MATH) ? 4 : 3;
  
  if (fase > maxFases) {
    mostrarResultadoFinal();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Proxima Fase: ");
    lcd.print(fase);
    lcd.setCursor(0, 1);
    lcd.print(" Pressione START");
    
    while(digitalRead(btnMove) == HIGH);
    delay(200);
  }
}

void mostrarResultadoFinal() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Fim do Jogo!");
  lcd.setCursor(0, 1);
  lcd.print(" Acertos: ");
  lcd.print(pontos);
  lcd.print("/");
  lcd.print(currentMode == MODE_MATH ? "40" : "30");
  
  while(true) {
    delay(1000);
  }
}
