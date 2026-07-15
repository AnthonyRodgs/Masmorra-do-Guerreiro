#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <optional>
#include <vector>

// ============================================================
// MAPA DO JOGO (Modelo Original)
// ============================================================
const char mapaOriginal[36][26] = {
    "1111111111111111111111111",
    "1232222222221222222222321",
    "1211112111121211112111121",
    "1211222111121211112202121",
    "1210212111121211111212121",
    "1222212111121211112222121",
    "1212112222222222222112221",
    "1212111221111111221122121",
    "1212222221111111222221121",
    "1211121221111111221211121",
    "1222221222221222221222221",
    "1111121111121211111211111",
    "1111121222221222221211111",
    "1111121211111111121211111",
    "1111121211111111121211111",
    "1111121210000000121211111",
    "1111121210000000121211111",
    "4422222210000000122222244",
    "1122121210000000121212211",
    "1122121210000000121212211",
    "1122121211110111121212211", 
    "1122121222220222221212211", 
    "1122121111112111111212211",
    "1122121111112111111212211",
    "1122222222222222222222201",
    "1122222111222221112222121",
    "1221122111212121112112121",
    "1222221111212121112112121",
    "1212121111222221102222221",
    "1222122011211121101111221",
    "1211111222222222221111221",
    "1211111111121211111111221",
    "1211111111121211111111221",
    "1211111111121212222222221",
    "1232222222200022222222321",
    "1111111111111111111111111"
};

char mapa[36][26]; // Matriz em tempo de execução

const float SIZE = 16; 

// Posições dos personagens
int posx0, posy0; // Guerreiro
int posx1, posy1; // Esqueleto
int posx2, posy2; // Zumbi
int posx3, posy3; // Morte
int posx4, posy4; // Fantasma

// Direções anteriores para os monstros não sambarem
int dirAnteriorMorte = -1;
int dirAnteriorFantasma = -1;
int dirAnteriorEsqueleto = -1;
int dirAnteriorZumbi = -1;

const int gaiolaX_Min = 9, gaiolaX_Max = 15;
const int gaiolaY_Min = 15, gaiolaY_Max = 19;
const int saidaX = 12;
const int saidaY = 21; 

bool foraDaGaiola[4];
bool monstrosEmRespawn[4];
float tempoRespawn[4];
const float tempoRespawnPadrao = 3.0f;

bool eh_tunel(int x, int y) {
    return y == 17 && (x < 0 || x >= 25 || x == 0 || x == 24);
}

void aplicar_tunel(int &x, int &y) {
    if (y == 17 && x < 0) x = 24;
    else if (y == 17 && x > 24) x = 0;
}

bool posicao_valida(int x, int y) {
    if (y < 0 || y >= 36) return false;
    if (x < 0 || x >= 25) return eh_tunel(x, y);
    return mapa[y][x] != '1';
}

bool esta_dentro_da_gaiola(int x, int y) {
    return (x >= gaiolaX_Min && x <= gaiolaX_Max && y >= gaiolaY_Min && y <= gaiolaY_Max);
}

bool posicao_ocupada_por_monstro(int x, int y, int indiceIgnorado) {
    int posX[] = {posx1, posx2, posx3, posx4};
    int posY[] = {posy1, posy2, posy3, posy4};
    for (int i = 0; i < 4; i++) {
        if (i == indiceIgnorado) continue;
        if (monstrosEmRespawn[i]) continue; 
        if (posX[i] == x && posY[i] == y) return true;
    }
    return false;
}

// Inicializa ou reinicia todo o estado do jogo
void inicializarJogo(int &pontuacao, int &moedasColetadas, bool &jogoAcabou, bool &jogadorVenceu, bool &fantasma_vulneravel,
                     bool &me, bool &md, bool &mc, bool &mb, bool &ie, bool &id, bool &ic, bool &ib) {
    
    // Copia o mapa original de volta
    for(int i = 0; i < 36; i++) {
        for(int j = 0; j < 25; j++) {
            mapa[i][j] = mapaOriginal[i][j];
        }
    }

    // Reseta status
    pontuacao = 0;
    moedasColetadas = 0;
    jogoAcabou = false;
    jogadorVenceu = false;
    fantasma_vulneravel = false;

    // Reseta inputs
    me = md = mc = mb = ie = id = ic = ib = false;

    // Reseta direções de IA
    dirAnteriorMorte = -1;
    dirAnteriorFantasma = -1;
    dirAnteriorEsqueleto = -1;
    dirAnteriorZumbi = -1;

    // Posições iniciais
    posx0 = 12; posy0 = 34; // Guerreiro
    posx1 = 11; posy1 = 16; // Esqueleto
    posx2 = 13; posy2 = 16; // Zumbi
    posx3 = 11; posy3 = 17; // Morte
    posx4 = 13; posy4 = 17; // Fantasma

    // Estados da Gaiola
    for(int i = 0; i < 4; i++) {
        foraDaGaiola[i] = false;
        monstrosEmRespawn[i] = false;
    }
    tempoRespawn[0] = 0.f;
    tempoRespawn[1] = 1.5f;
    tempoRespawn[2] = 3.0f;
    tempoRespawn[3] = 4.5f;
}

void moverSairDaGaiola(int &px, int &py, int indiceMonstro) {
    if (px < saidaX) {
        if (!posicao_ocupada_por_monstro(px + 1, py, indiceMonstro)) px++;
    } else if (px > saidaX) {
        if (!posicao_ocupada_por_monstro(px - 1, py, indiceMonstro)) px--;
    } else {
        if (!posicao_ocupada_por_monstro(px, py + 1, indiceMonstro)) py++;
    }
    if (px == saidaX && py >= saidaY) {
        foraDaGaiola[indiceMonstro] = true;
    }
}

void moverPerseguicao(int &px, int &py, int alvox, int alvoy, int &dirAnterior, int indiceMonstro) {
    int dx[] = { 0,  0, -1,  1}; 
    int dy[] = {-1,  1,  0,  0};
    int oposto[] = {1, 0, 3, 2}; 
    int melhorDir = -1;
    int menorDist = 99999;

    bool presoNaSaida = (py == 21 || py == 22) && (px >= 11 && px <= 13) && (alvoy < py);

    for (int d = 0; d < 4; d++) {
        int nx = px + dx[d];
        int ny = py + dy[d];

        if (!posicao_valida(nx, ny)) continue;
        if (posicao_ocupada_por_monstro(nx, ny, indiceMonstro)) continue;
        if (esta_dentro_da_gaiola(nx, ny)) continue; 
        if (presoNaSaida && d == 0) continue;
        if (dirAnterior != -1 && d == oposto[dirAnterior]) continue;

        int dist = abs(nx - alvox) + abs(ny - alvoy);
        if (dist < menorDist) {
            menorDist = dist;
            melhorDir = d;
        }
    }

    if (melhorDir == -1 && dirAnterior != -1) {
        int backDir = oposto[dirAnterior];
        int nx = px + dx[backDir];
        int ny = py + dy[backDir];
        if (posicao_valida(nx, ny) && !posicao_ocupada_por_monstro(nx, ny, indiceMonstro) && !esta_dentro_da_gaiola(nx, ny)) {
            melhorDir = backDir;
        }
    }

    if (melhorDir != -1) {
        px += dx[melhorDir];
        py += dy[melhorDir];
        dirAnterior = melhorDir;
    }
}

void moverAleatorio(int &px, int &py, int &dirAnterior, int indiceMonstro) {
    int dx[] = { 0,  0, -1,  1}; 
    int dy[] = {-1,  1,  0,  0};
    int oposto[] = {1, 0, 3, 2}; 
    std::vector<int> direcoesValidas;

    for (int d = 0; d < 4; d++) {
        int nx = px + dx[d];
        int ny = py + dy[d];

        if (!posicao_valida(nx, ny)) continue;
        if (posicao_ocupada_por_monstro(nx, ny, indiceMonstro)) continue;
        if (esta_dentro_da_gaiola(nx, ny)) continue; 
        if (dirAnterior != -1 && d == oposto[dirAnterior]) continue; 

        direcoesValidas.push_back(d);
    }

    if (direcoesValidas.empty() && dirAnterior != -1) {
        int backDir = oposto[dirAnterior];
        int nx = px + dx[backDir];
        int ny = py + dy[backDir];
        if (posicao_valida(nx, ny) && !posicao_ocupada_por_monstro(nx, ny, indiceMonstro) && !esta_dentro_da_gaiola(nx, ny)) {
            direcoesValidas.push_back(backDir);
        }
    }

    if (!direcoesValidas.empty()) {
        int escolha = direcoesValidas[rand() % direcoesValidas.size()];
        px += dx[escolha];
        py += dy[escolha];
        dirAnterior = escolha;
    }
}

bool colidiu(int px, int py, int ix, int iy) {
    return (px == ix && py == iy);
}

// ============================================================
// FUNÇÃO PRINCIPAL
// ============================================================
int main() {
    srand(time(nullptr));

    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Masmorra do Guerreiro");
    sf::View view(sf::FloatRect({0.f, 0.f}, {400.f, 616.f}));
    window.setView(view);

    auto carregarTextura = [](sf::Texture& tex, const std::string& arquivo) {
        if (!tex.loadFromFile(arquivo)) {
            std::cout << "\n[ERRO CRITICO] O ficheiro '" << arquivo << "' nao foi encontrado na pasta!\n";
            system("pause");
            return false;
        }
        return true;
    };

    sf::Texture texturaMenu, textureInterface, textureMoeda, textureEspada, textureMapa;
    sf::Texture textureGuerreiro, textureEsqueleto, textureZumbi, textureMorte, textureFantasma;
    sf::Font fonte;

    if (!carregarTextura(texturaMenu, "assets/images/menu.png")) return -1;
    if (!carregarTextura(textureInterface, "assets/images/interface.png")) return -1;
    if (!carregarTextura(textureMoeda, "assets/images/moeda.png")) return -1;
    if (!carregarTextura(textureEspada, "assets/images/espada.png")) return -1;
    if (!carregarTextura(textureMapa, "assets/images/mapa.png")) return -1;
    if (!carregarTextura(textureGuerreiro, "assets/images/guerreiro.png")) return -1;
    if (!carregarTextura(textureEsqueleto, "assets/images/esqueletobaixo.png")) return -1;
    if (!carregarTextura(textureZumbi, "assets/images/zumbibaixo.png")) return -1;
    if (!carregarTextura(textureMorte, "assets/images/mortebaixo.png")) return -1;
    if (!carregarTextura(textureFantasma, "assets/images/fantasmabaixo.png")) return -1;

    if (!fonte.openFromFile("assets/fonts/ARLRDBD.ttf")) {
        std::cout << "\n[ERRO CRITICO] A fonte 'ARLRDBD.ttf' nao foi encontrada!\n";
        return -1;
    }

    sf::Music musicaFundo;
    if (!musicaFundo.openFromFile("assets/sounds/tema.ogg")) {
        std::cout << "\n[AVISO] O ficheiro de audio 'tema.ogg' nao foi encontrado!\n";
    } else {
        musicaFundo.setLooping(true);
        musicaFundo.setVolume(50.f);
        musicaFundo.play();
    }

    sf::Sprite spriteMenu(texturaMenu);
    sf::Sprite spriteInterface(textureInterface);
    sf::Sprite spriteMoeda(textureMoeda);
    sf::Sprite spriteEspada(textureEspada);
    sf::Sprite spriteMapa(textureMapa);
    sf::Sprite spriteGuerreiro(textureGuerreiro);
    sf::Sprite spriteEsqueleto(textureEsqueleto);
    sf::Sprite spriteZumbi(textureZumbi);
    sf::Sprite spriteMorte(textureMorte);
    sf::Sprite spriteFantasma(textureFantasma);

    float escala = 0.85f;
    spriteMapa.setScale({escala, escala});
    spriteGuerreiro.setScale({escala, escala});
    spriteEsqueleto.setScale({escala, escala});
    spriteZumbi.setScale({escala, escala});
    spriteMorte.setScale({escala, escala});
    spriteFantasma.setScale({escala, escala});
    spriteMoeda.setScale({escala, escala});
    spriteEspada.setScale({escala, escala});

    sf::Text textoPontuacao(fonte);
    textoPontuacao.setCharacterSize(10);
    textoPontuacao.setFillColor(sf::Color::White);
    textoPontuacao.setOutlineColor(sf::Color::Black);
    textoPontuacao.setOutlineThickness(2.f);

    sf::RectangleShape overlayEscuro({400.f, 576.f});
    overlayEscuro.setFillColor(sf::Color(0, 0, 0, 195));

    sf::Text textoFim(fonte);
    textoFim.setCharacterSize(18); // Diminuído um pouco para caber as duas linhas confortavelmente
    textoFim.setFillColor(sf::Color::White);

    sf::Text textoInstrucao(fonte);
    textoInstrucao.setCharacterSize(12);
    textoInstrucao.setFillColor(sf::Color(200, 200, 200));
    textoInstrucao.setString("Pressione ESPACO para jogar novamente\nPressione ESC para sair");
    
    // Variáveis de controle do estado do jogo
    int pontuacao = 0;
    int moedasColetadas = 0;
    bool juegoAcabou = false;    
    bool jogadorVenceu = false; 
    bool jogoIniciado = false; 
    bool fantasma_vulneravel = false;

    bool move_esquerda = false, move_direita = false, move_cima = false, move_baixo = false;
    bool intencao_esquerda = false, intencao_direita = false, intencao_cima = false, intencao_baixo = false;

    // Inicializa o primeiro jogo
    inicializarJogo(pontuacao, moedasColetadas, juegoAcabou, jogadorVenceu, fantasma_vulneravel,
                    move_esquerda, move_direita, move_cima, move_baixo, 
                    intencao_esquerda, intencao_direita, intencao_cima, intencao_baixo);

    sf::Clock relogioMovimento;
    sf::Clock relogioMonstros;
    sf::Clock relogioModoForca;
    const float intervaloPasso = 0.2f; 
    const float intervaloMonstros = 0.3f; 
    const float tempoModoForca = 8.0f;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    window.close();
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
                    if (!jogoIniciado) {
                        jogoIniciado = true;
                        relogioMovimento.restart(); 
                        relogioMonstros.restart();
                    } 
                    else if (juegoAcabou) {
                        // REINICIA O JOGO SE ESTIVER NA TELA DE GAME OVER / VITÓRIA
                        inicializarJogo(pontuacao, moedasColetadas, juegoAcabou, jogadorVenceu, fantasma_vulneravel,
                                        move_esquerda, move_direita, move_cima, move_baixo, 
                                        intencao_esquerda, intencao_direita, intencao_cima, intencao_baixo);
                        relogioMovimento.restart(); 
                        relogioMonstros.restart();
                        if (musicaFundo.getStatus() != sf::Sound::Status::Playing) {
                            musicaFundo.play();
                        }
                    }
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Left) {
                    intencao_esquerda = true;  intencao_direita = false; intencao_cima = false; intencao_baixo = false;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Right) {
                    intencao_esquerda = false; intencao_direita = true; intencao_cima = false; intencao_baixo = false;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Up) {
                    intencao_esquerda = false; intencao_direita = false; intencao_cima = true;  intencao_baixo = false;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Down) {
                    intencao_esquerda = false; intencao_direita = false; intencao_cima = false; intencao_baixo = true;
                }
            }
        }

        if (!window.isOpen()) break;

        if (relogioMovimento.getElapsedTime().asSeconds() >= intervaloPasso && !juegoAcabou && jogoIniciado) {
            relogioMovimento.restart();

            if (posicao_valida(posx0 - 1, posy0) && intencao_esquerda) { move_esquerda = true; move_direita = false; move_cima = false; move_baixo = false; }
            if (posicao_valida(posx0 + 1, posy0) && intencao_direita) { move_esquerda = false; move_direita = true; move_cima = false; move_baixo = false; }
            if (posicao_valida(posx0, posy0 - 1) && intencao_cima) { move_esquerda = false; move_direita = false; move_cima = true; move_baixo = false; }
            if (posicao_valida(posx0, posy0 + 1) && intencao_baixo) { move_esquerda = false; move_direita = false; move_cima = false; move_baixo = true; }

            if      (posicao_valida(posx0 - 1, posy0) && move_esquerda) posx0--;
            else if (posicao_valida(posx0 + 1, posy0) && move_direita)  posx0++;
            else if (posicao_valida(posx0, posy0 - 1) && move_cima)     posy0--;
            else if (posicao_valida(posx0, posy0 + 1) && move_baixo)    posy0++;
            else    move_esquerda = move_direita = move_cima = move_baixo = false;
            
            aplicar_tunel(posx0, posy0);
            
            if (mapa[posy0][posx0] == '2') {
                mapa[posy0][posx0] = '0'; 
                moedasColetadas++;
                pontuacao += 10;
            }
            if (mapa[posy0][posx0] == '3') {
                mapa[posy0][posx0] = '0'; 
                fantasma_vulneravel = true;
                relogioModoForca.restart();
            }

            int* posicoesX[4] = {&posx1, &posx2, &posx3, &posx4};
            int* posicoesY[4] = {&posy1, &posy2, &posy3, &posy4};

            for (int i = 0; i < 4; i++) {
                if (monstrosEmRespawn[i] || !foraDaGaiola[i]) {
                    tempoRespawn[i] -= intervaloPasso;
                }
            }

            if (relogioMonstros.getElapsedTime().asSeconds() >= intervaloMonstros) {
                relogioMonstros.restart();
                for (int i = 0; i < 4; i++) {
                    if (!monstrosEmRespawn[i]) {
                        if (!foraDaGaiola[i]) {
                            if (tempoRespawn[i] <= 0.f) {
                                moverSairDaGaiola(*posicoesX[i], *posicoesY[i], i);
                            }
                        } 
                        else {
                            if (i == 0) {
                                moverPerseguicao(*posicoesX[i], *posicoesY[i], posx0, posy0, dirAnteriorEsqueleto, i);
                            } 
                            else if (i == 1) {
                                moverPerseguicao(*posicoesX[i], *posicoesY[i], posx0, posy0, dirAnteriorZumbi, i);
                            }
                            else if (i == 2) {
                                moverAleatorio(*posicoesX[i], *posicoesY[i], dirAnteriorMorte, i);
                            } 
                            else if (i == 3) {
                                moverAleatorio(*posicoesX[i], *posicoesY[i], dirAnteriorFantasma, i);
                            }
                        }
                    } else {
                        if (tempoRespawn[i] <= 0.f) {
                            monstrosEmRespawn[i] = false;
                            foraDaGaiola[i] = false;
                            tempoRespawn[i] = 0.f; 
                            
                            if (i == 0) { *posicoesX[i] = 11; *posicoesY[i] = 16; dirAnteriorEsqueleto = -1; }
                            if (i == 1) { *posicoesX[i] = 13; *posicoesY[i] = 16; dirAnteriorZumbi = -1; }
                            if (i == 2) { *posicoesX[i] = 11; *posicoesY[i] = 17; dirAnteriorMorte = -1; }
                            if (i == 3) { *posicoesX[i] = 13; *posicoesY[i] = 17; dirAnteriorFantasma = -1; }
                        }
                    }
                }
            }

            if (fantasma_vulneravel && relogioModoForca.getElapsedTime().asSeconds() >= tempoModoForca) {
                fantasma_vulneravel = false;
            }

            for (int i = 0; i < 4; i++) {
                if (monstrosEmRespawn[i]) continue;
                
                if (colidiu(posx0, posy0, *posicoesX[i], *posicoesY[i])) {
                    if (fantasma_vulneravel) {
                        monstrosEmRespawn[i] = true;
                        tempoRespawn[i] = tempoRespawnPadrao;
                        pontuacao += 50; 
                    } else {
                        juegoAcabou = true;
                    }
                }
            }

            int moedasRestantes = 0;
            for (int i = 0; i < 36; i++)
                for (int j = 0; j < 25; j++)
                    if (mapa[i][j] == '2') moedasRestantes++;

            if (moedasRestantes == 0) {
                juegoAcabou = true;
                jogadorVenceu = true;
            }
        }
        
        window.clear(sf::Color::Black);

        if (!jogoIniciado) {
            window.draw(spriteMenu);           
        } 
        else {
            textoPontuacao.setString(std::to_string(pontuacao));
            sf::FloatRect bounds = textoPontuacao.getLocalBounds();
            textoPontuacao.setPosition({380.f - bounds.size.x, 20.f});

            window.draw(spriteInterface);
            window.draw(textoPontuacao);

            spriteMapa.setPosition({0.f, 40.f});
            window.draw(spriteMapa);

            for (int i = 0; i < 36; i++) {
                for (int j = 0; j < 25; j++) {
                    if (mapa[i][j] == '2') {
                        spriteMoeda.setPosition({j * SIZE, (i * SIZE) + 40.f});
                        window.draw(spriteMoeda);
                    } else if (mapa[i][j] == '3') {
                        spriteEspada.setPosition({j * SIZE, (i * SIZE) + 40.f});
                        window.draw(spriteEspada);
                    }
                }
            }

            spriteGuerreiro.setPosition({posx0 * SIZE, (posy0 * SIZE) + 40.f});
            window.draw(spriteGuerreiro);

            if (fantasma_vulneravel) {
                sf::Color corVulneravel(50, 50, 255); 
                spriteEsqueleto.setColor(corVulneravel);
                spriteZumbi.setColor(corVulneravel);
                spriteMorte.setColor(corVulneravel);
                spriteFantasma.setColor(corVulneravel);
            } else {
                spriteEsqueleto.setColor(sf::Color::White);
                spriteZumbi.setColor(sf::Color::White);
                spriteMorte.setColor(sf::Color::White);
                spriteFantasma.setColor(sf::Color::White);
            }

            if (!monstrosEmRespawn[0]) {
                spriteEsqueleto.setPosition({posx1 * SIZE, (posy1 * SIZE) + 40.f});
                window.draw(spriteEsqueleto);
            }
            if (!monstrosEmRespawn[1]) {
                spriteZumbi.setPosition({posx2 * SIZE, (posy2 * SIZE) + 40.f});
                window.draw(spriteZumbi);
            }
            if (!monstrosEmRespawn[2]) {
                spriteMorte.setPosition({posx3 * SIZE, (posy3 * SIZE) + 40.f});
                window.draw(spriteMorte);
            }
            if (!monstrosEmRespawn[3]) {
                spriteFantasma.setPosition({posx4 * SIZE, (posy4 * SIZE) + 40.f});
                window.draw(spriteFantasma);
            }
        }

        if (juegoAcabou) {
            musicaFundo.stop();
            
            // 1. ADICIONA A PONTUAÇÃO DIRETAMENTE NO TEXTO FINAL
            if (jogadorVenceu) {
                textoFim.setString("Parabens, voce venceu!\nPontuacao Final: " + std::to_string(pontuacao));
            } else {
                textoFim.setString("Game Over!\nPontuacao: " + std::to_string(pontuacao));
            }

            // Centraliza o texto final
            sf::FloatRect b = textoFim.getLocalBounds();
            textoFim.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
            textoFim.setPosition({200.f, 260.f});

            // Centraliza as instruções de reiniciar
            sf::FloatRect bIns = textoInstrucao.getLocalBounds();
            textoInstrucao.setOrigin({bIns.position.x + bIns.size.x / 2.f, bIns.position.y + bIns.size.y / 2.f});
            textoInstrucao.setPosition({200.f, 330.f});

            window.draw(overlayEscuro);  
            window.draw(textoFim);       
            window.draw(textoInstrucao); 
        }

        window.display();
    }
    
    return 0;
}