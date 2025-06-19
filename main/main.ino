#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definição dos botões
const int btnMove = 2;       // Mover seleção(botão branco)
const int btnConfirm = 3;    // Confirmar resposta(botão verde)
const int btnMath = 4;       // Modo Matemática(botão azul)
const int btnPortuguese = 5; // Modo Português(botão amarelo)
const int ledRed = 6;        // Led Vermelho
const int ledGreen = 7;      // Led Verde
const int buzzer = 8;        //Som

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
const String palavras3Letras[10] = {"ASA","GOL","MAR","SIM","OVO","SOL","UVA","MEL","FIM","GEL"};
const String palavras3LetrasErradas[10] = {"AZA","GOU","NAR","SIN","OVU","SEL","UWA","NEL","FIN","GEU"};
const String palavras4Letras[10] = {"CASA","BICO","MEIA","BOLO","TREM","BIFE","GELO","AZUL","OSSO","GATO"};
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
  pinMode(buzzer, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);

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
  tocarSomInicioJogo();
  piscarLedsInicioFim();

  while (true) {
    if (digitalRead(btnMath) == LOW) {
      currentMode = MODE_MATH;
      somMat(); 
      break;
    }
    if (digitalRead(btnPortuguese) == LOW) {
      currentMode = MODE_PORTUGUESE;
      somPort();
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
    somMat(); 
    reiniciarJogo();
  }
  if (digitalRead(btnPortuguese) == LOW) {
    currentMode = MODE_PORTUGUESE;
    somPort();
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
  if (currentMode == MODE_MATH){ 
  gerarPerguntaMatematica();}
  else{ gerarPerguntaPortugues();}
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
    lcd.setCursor(3, 1); lcd.print(palavraCorreta);
    lcd.setCursor(10, 1); lcd.print(palavraErrada);
  } else {
    lcd.setCursor(3, 1); lcd.print(palavraErrada);
    lcd.setCursor(10, 1); lcd.print(palavraCorreta);
  }

  respostaSelecionada = true; // Começa selecionando a esquerda
  mostrarSelecao();
}

// --- Seleção do caminhão ---
void mostrarSelecao() {
  lcd.setCursor(2, 1); lcd.print(' ');
  lcd.setCursor(9, 1); lcd.print(' ');
  int col = respostaSelecionada ? 2 : 9;
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
    tocarSomErro();
    ligaLedVermelho();
    
  } else if (acertou) {
    lcd.print("Resposta Correta");
    pontos++;
    tocarSomCorreto();
    ligaLedVerde();
    
  } else {
    lcd.print("Resposta Errada");
    tocarSomErro();
    ligaLedVermelho();
    
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
    tocarSomFimDeJogo();
    piscarLedsInicioFim();
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

void tocarSomCorreto() {
    tone(buzzer, 1200, 100); // nota aguda
  delay(120);
  tone(buzzer, 1500, 100); // mais aguda ainda
  delay(120);
  noTone(buzzer);
}

void tocarSomErro() {
 tone(buzzer, 400, 200); // tom grave
  delay(250);
  tone(buzzer, 300, 200); // mais grave
  delay(250);
  noTone(buzzer);
}

void somPort(){
  int melodia[] = {659, 698, 784, 659}; // mi, fá, sol, mi
  int duracao = 180;

  for (int i = 0; i < 4; i++) {
    tone(buzzer, melodia[i], duracao);
    delay(duracao + 40);
  }
  noTone(buzzer);
}

void somMat(){
int melodia[] = {262, 330, 392, 523}; // dó, mi, sol, dó (agudo)
  int duracao = 200;

  for (int i = 0; i < 4; i++) {
    tone(buzzer, melodia[i], duracao);
    delay(duracao + 50);
  }
  noTone(buzzer);
}

void tocarSomFimDeJogo() {
  int notas[] = {523, 494, 440, 392, 349, 330, 294, 262}; // Dó a Dó descendente (C5 a C4)
  for (int i = 0; i < 8; i++) {
    tone(buzzer, notas[i], 150);
    delay(170);
  }
  delay(200);
  tone(buzzer, 262, 100); // Dó
  delay(100);
  tone(buzzer, 392, 100); // Sol
  delay(100);
  tone(buzzer, 523, 300); // Dó (mais agudo)
  delay(320);
  noTone(buzzer);
}

void tocarSomInicioJogo() {
  int notas[] = {262, 294, 330, 349, 392, 440, 494, 523}; // Dó a Dó (C4 a C5)
  for (int i = 0; i < 8; i++) {
    tone(buzzer, notas[i], 150);
    delay(170);
  }
  noTone(buzzer);
}

void ligaLedVermelho(){
  digitalWrite(ledRed, HIGH);
  delay(3500);
  digitalWrite(ledRed, LOW);
  }

void ligaLedVerde(){
  digitalWrite(ledGreen, HIGH);
  delay(3500);
    digitalWrite(ledGreen, LOW);
  }

void piscarLedsInicioFim() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, HIGH);
    delay(300);
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, LOW);
    delay(300);
  }
}
