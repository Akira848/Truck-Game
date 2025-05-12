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
bool respostaSelecionada = true;  // true = esquerda, false = direita
bool respostaCorretaEsquerda;     // true = correta está na esquerda

// Variáveis para modo português
String palavraCorreta;
String palavraErrada;
const String palavras3Letras[10] = {"ASA","GOL","MAR","SIM","OVO","MAE","UVA","NAO","FIM","GEL"};
const String palavras3LetrasErradas[10] = {"AZA","GOU","NAR","SIN","OVU","MAI","UWA","NAU","FIN","GEU"};
const String palavras4Letras[10] = {"CASA","BICO","MEIA","BOLO","TREM","BIFE","GELO","AZUL","OSSO","LAÇO"};
const String palavras4LetrasErradas[10] = {"CAZA","BICU","NEIA","BOLU","TREN","BIFI","JELO","ASUL","OSCO","LASO"};
const String palavras5Letras[10] = {"VOLTA","CLIMA","PRIMO","TIGRE","ACIMA","MILHO","CILIO","PLANO","PRAGA","TALCO"};
const String palavras5LetrasErradas[10] = {"VOUTA","CRIMA","PRIMU","TIRGE","ASIMA","MILIO","CILHO","PRANO","PLAGA","TAUCO"};

// Vetores de controle para palavras usadas
bool palavras3LetrasUsadas[10] = {false};
bool palavras4LetrasUsadas[10] = {false};
bool palavras5LetrasUsadas[10] = {false};

void setup() {
  pinMode(btnMove, INPUT_PULLUP);
  pinMode(btnConfirm, INPUT_PULLUP);
  pinMode(btnMath, INPUT_PULLUP);
  pinMode(btnPortuguese, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  lcd.createChar(0, caminhaoCima);
  lcd.createChar(1, caminhaoBaixo);

  mostrarMensagemBoasVindas();
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

// --- Boas-vindas e seleção de modo ---
void mostrarMensagemBoasVindas() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(byte(0)); lcd.print("   BEM VINDO  "); lcd.write(byte(0));
  lcd.setCursor(0, 1);
  lcd.write(byte(1)); lcd.print(" CAMINHONEIRO "); lcd.write(byte(1));

  while (true) {
    if (digitalRead(btnMath) == LOW) {
      currentMode = MODE_MATH;
      break;
    }
    if (digitalRead(btnPortuguese) == LOW) {
      currentMode = MODE_PORTUGUESE;
      break;
    }
    delay(100);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(currentMode == MODE_MATH ? " Modo Matematica" : " Modo Portugues");
  delay(1000);
  reiniciarJogo();
}

void verificarSelecaoModo() {
  if (digitalRead(btnMath) == LOW) {
    currentMode = MODE_MATH;
    reiniciarJogo();
  }
  if (digitalRead(btnPortuguese) == LOW) {
    currentMode = MODE_PORTUGUESE;
    reiniciarJogo();
  }
}

void reiniciarJogo() {
  fase = 1;
  rodada = 1;
  pontos = 0;
  aguardandoResposta = false;

  // Reseta palavras usadas (Modo Português)
  memset(palavras3LetrasUsadas, false, sizeof(palavras3LetrasUsadas));
  memset(palavras4LetrasUsadas, false, sizeof(palavras4LetrasUsadas));
  memset(palavras5LetrasUsadas, false, sizeof(palavras5LetrasUsadas));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(currentMode == MODE_MATH ? " Modo Matematica" : " Modo Portugues");
  delay(1000);
}

// --- Geração de perguntas ---
void gerarPergunta() {
  if (currentMode == MODE_MATH) gerarPerguntaMatematica();
  else gerarPerguntaPortugues();
}

void gerarPerguntaMatematica() {
  lcd.clear();
  int num1, num2;
  char op;
  switch(fase) {
    case 1: num1 = random(5, 50); num2 = random(5, 50); respostaCorreta = num1 + num2; op = '+'; break;
    case 2: num1 = random(20, 100); num2 = random(5, num1-4); respostaCorreta = num1 - num2; op = '-'; break;
    case 3: num1 = random(2, 10); num2 = random(2, 10); respostaCorreta = num1 * num2; op = 'x'; break;
    case 4: num2 = random(2, 10); respostaCorreta = random(2, 10); num1 = num2 * respostaCorreta; op = '/'; break;
  }
  respostaErrada = gerarRespostaErrada(respostaCorreta);

  // Exibe a conta
  lcd.setCursor(5, 0);
  lcd.print(num1);
  lcd.print(op);
  lcd.print(num2);
  lcd.print("=?");

  // Posiciona respostas aleatoriamente
  int posicaoCorreta = random(0, 2); // 0 = esquerda, 1 = direita
  respostaCorretaEsquerda = (posicaoCorreta == 0);

  if (respostaCorretaEsquerda) {
    lcd.setCursor(4, 1); lcd.print(respostaCorreta);
    lcd.setCursor(10, 1); lcd.print(respostaErrada);
  } else {
    lcd.setCursor(4, 1); lcd.print(respostaErrada);
    lcd.setCursor(10, 1); lcd.print(respostaCorreta);
  }

  respostaSelecionada = true; // Começa selecionando a esquerda
  mostrarSelecao();
}

int gerarRespostaErrada(int correta) {
  int e;
  do {
    int var = random(1, 6);
    e = correta + (random(0, 2) ? var : -var);
  } while(e <= 0 || e == correta);
  return e;
}

void gerarPerguntaPortugues() {
  lcd.clear();
  int i = random(0, 10);
  bool palavraUsada = true;

  while (palavraUsada) {
    palavraUsada = false;
    i = random(0, 10);

    switch(fase) {
      case 1:
        if (!palavras3LetrasUsadas[i]) {
          palavraCorreta = palavras3Letras[i];
          palavraErrada = palavras3LetrasErradas[i];
          palavras3LetrasUsadas[i] = true;
          palavraUsada = false;
        } else {
          palavraUsada = true;
        }
        break;
      case 2:
        if (!palavras4LetrasUsadas[i]) {
          palavraCorreta = palavras4Letras[i];
          palavraErrada = palavras4LetrasErradas[i];
          palavras4LetrasUsadas[i] = true;
          palavraUsada = false;
        } else {
          palavraUsada = true;
        }
        break;
      case 3:
        if (!palavras5LetrasUsadas[i]) {
          palavraCorreta = palavras5Letras[i];
          palavraErrada = palavras5LetrasErradas[i];
          palavras5LetrasUsadas[i] = true;
          palavraUsada = false;
        } else {
          palavraUsada = true;
        }
        break;
    }
  }

  lcd.setCursor(3, 0);
  lcd.print("QUAL A CERTA?");

  // Posiciona palavras aleatoriamente
  int posicaoCorreta = random(0, 2);
  respostaCorretaEsquerda = (posicaoCorreta == 0);

  if (respostaCorretaEsquerda) {
    lcd.setCursor(4, 1); lcd.print(palavraCorreta);
    lcd.setCursor(10, 1); lcd.print(palavraErrada);
  } else {
    lcd.setCursor(4, 1); lcd.print(palavraErrada);
    lcd.setCursor(10, 1); lcd.print(palavraCorreta);
  }

  respostaSelecionada = true; // Começa selecionando a esquerda
  mostrarSelecao();
}

// --- Seleção do caminhão ---
void mostrarSelecao() {
  lcd.setCursor(2, 1); lcd.print(' ');
  lcd.setCursor(8, 1); lcd.print(' ');
  int col = respostaSelecionada ? 2 : 8;
  lcd.setCursor(col, 1);
  lcd.write(byte(0));
}

// --- Timer e botões ---
void atualizarTimer() {
  unsigned long t = millis() - tempoInicioRodada;
  unsigned long lim = (currentMode == MODE_MATH ? (unsigned long[]){60000, 75000, 90000, 105000}[fase-1] : (unsigned long[]){45000, 60000, 75000}[fase-1]);
  
  if (t >= lim) {
    tempoEsgotado = true;
    lcd.setCursor(0, 0); lcd.print("0 ");
    verificarResposta();
  } else {
    lcd.setCursor(0, 0);
    lcd.print((lim - t) / 1000);
    lcd.print(" ");
  }
}

void verificarBotoesJogo() {
  if (digitalRead(btnMove) == LOW) {
    respostaSelecionada = !respostaSelecionada;
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

  bool acertou = (respostaSelecionada == respostaCorretaEsquerda);

  if (tempoEsgotado) {
    lcd.print(" TEMPO ESGOTADO!");
  } else if (acertou) {
    lcd.print("Resposta Correta");
    pontos++;
  } else {
    lcd.print("Resposta Errada");
  }

  lcd.setCursor(4, 1);
  lcd.print("PONTOS:"); lcd.print(pontos);
  rodada++;
  delay(2000);
}

void passarParaProximaFase() {
  fase++;
  rodada = 1;
  int maxF = (currentMode == MODE_MATH ? 4 : 3);

  if (fase > maxF) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("  Fim do Jogo!");
    lcd.setCursor(0, 1); lcd.print(" Acertos: "); lcd.print(pontos); lcd.print("/"); lcd.print(maxF * 10);
    
    while (digitalRead(btnConfirm) == HIGH) {
      delay(100);
    }
    
    currentMode = MODE_MATH;
    fase = 1;
    aguardandoResposta = false;
    mostrarMensagemBoasVindas();
    randomSeed(analogRead(0));
  } else {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(" Proxima Fase: "); lcd.print(fase);
    lcd.setCursor(0, 1); lcd.print(" Pressione MOVE");
    
    while (digitalRead(btnMove) == HIGH) {
      delay(100);
    }
    
    delay(200);
  }
}