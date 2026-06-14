#include <iostream>
#include <fstream> // Necessário para manipulação de arquivos (Gravação em disco)
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <random>
#include <csignal>

using namespace std;

const string RESET   = "\033[0m";
const string VERDE   = "\033[32m";
const string VERMELHO= "\033[31m";
const string CIANO   = "\033[36m";
const string AMARELO = "\033[33m";
const string NEGRITO = "\033[1m";

struct FaixaIP {
    string base;
    int min_o3;
    int max_o3;
};

vector<string> servidores_descobertos;
unsigned long long total_testado = 0;
string caminho_salvamento = "/home/gabriel/Downloads/Zodiac_Servers_Achados.txt";

void exibir_relatorio_final(int sinal) {
    cout << "\n\n" << VERMELHO << NEGRITO << "========================================================================" << RESET << endl;
    cout << AMARELO << NEGRITO << "             [ ZODIAC - RELATORIO FINAL DE VARREDURA ]" << RESET << endl;
    cout << VERMELHO << NEGRITO << "========================================================================" << RESET << endl;
    cout << CIANO << "[*] Total de portas/IPs testados: " << total_testado << RESET << endl;
    cout << CIANO << "[*] Servidores validos confirmados: " << servidores_descobertos.size() << "\n" << RESET << endl;

    if (servidores_descobertos.empty()) {
        cout << VERMELHO << "[-] Nenhuma sala ativa confirmada no periodo." << RESET << endl;
    } else {
        cout << VERDE << NEGRITO << "[+] LISTA DE IPS COLETADOS:" << RESET << endl;
        for (const auto& ip_salvo : servidores_descobertos) {
            cout << VERDE << "    ➔ " << ip_salvo << RESET << endl;
        }
        cout << AMARELO << "\n[*] O historico completo também foi salvo em: " << caminho_salvamento << RESET << endl;
    }
    cout << VERMELHO << NEGRITO << "========================================================================" << RESET << endl;
    exit(sinal); 
}

bool validar_protocolo_minecraft(const string& ip, int porta, string& dados_retornados) {
    int telefon = socket(AF_INET, SOCK_STREAM, 0);
    if (telefon < 0) return false;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 450000; 
    setsockopt(telefon, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(telefon, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    struct sockaddr_in ppt;
    memset(&ppt, 0, sizeof(ppt));
    ppt.sin_family = AF_INET;
    ppt.sin_port = htons(porta);

    if (inet_pton(AF_INET, ip.c_str(), &ppt.sin_addr) <= 0) {
        close(telefon);
        return false;
    }

    int resultado = connect(telefon, (struct sockaddr*)&ppt, sizeof(ppt));
    
    if (resultado >= 0) {
        unsigned char packet[] = {
            0x0F, 0x00, 0x2F, 0x09, 'l', 'o', 'c', 'a', 'l', 'h', 'o', 's', 't', 0x63, 0xDD, 0x01,
            0x01, 0x00
        };
        send(telefon, packet, sizeof(packet), 0);
        
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(telefon, buffer, sizeof(buffer) - 1, 0);
        close(telefon);
        
        if (bytes > 5) {
            string resposta = "";
            for (int i = 0; i < bytes; ++i) {
                if (buffer[i] >= 32 && buffer[i] <= 126) {
                    resposta += buffer[i];
                }
            }
            
            if (resposta.find("version") != string::npos || resposta.find("players") != string::npos || resposta.find("description") != string::npos) {
                dados_retornados = resposta;
                return true;
            }
        }
    }

    close(telefon);
    return false;
}

int main() {
    signal(SIGINT, exibir_relatorio_final);
    system("clear");
    
    cout << VERMELHO << NEGRITO << "========================================================================" << RESET << endl;
    cout << VERDE << "              |\\                                     /|" << endl;
    cout << VERDE << "              | \\           [ ZODIAC ]              / |" << endl;
    cout << VERDE << "              |  \\           __---__               /  |" << endl;
    cout << VERDE << "              |   \\        _-[_ _ _]-_            /   |" << endl;
    cout << VERDE << "              |    \\      /_ _ _ _ _ _\\          /    |" << endl;
    cout << VERDE << "              |_____\\    |_____________|        /_____|" << endl;
    cout << VERDE << "              | _ _ |    |  _   _   _  |        | _ _ |" << endl;
    cout << VERDE << "              |     |    | | | | | | | |        |     |" << endl;
    cout << VERDE << "              |     |    | |_| |_| |_| |        |     |" << endl;
    cout << VERDE << "              | _ _ |    |  _________  |        | _ _ |" << endl;
    cout << VERDE << "              |     |    | |    |    | |        |     |" << endl;
    cout << VERDE << "              |     |    | |    |    | |        |     |" << endl;
    cout << VERDE << "              |_____|____|_|____|____|_|________|_____|" << RESET << endl;
    cout << VERMELHO << NEGRITO << "========================================================================" << RESET << endl;

    vector<int> portas_alvo = {
        25565, 25564, 25566, 25567, 25568, 25569, 25570, 
        25575, 25545, 25563, 25600, 25700, 25800,        
        10000, 10092, 10112, 10521, 11021,               
        19132, 19133                                      
    };

    vector<FaixaIP> faixas_alvo = {
        {"51.81", 0, 255}, {"142.44", 0, 255}, {"192.99", 0, 255}, {"151.106", 0, 255},
        {"129.151", 0, 255}, {"144.22", 0, 255}, {"66.70", 0, 255}, {"167.114", 0, 255}
    };

    // Passo 3: Escolha do Modo de Operação
    cout << VERDE << NEGRITO << "\n[>] PASSO 3: MODO DE ESCOPO" << RESET << endl;
    cout << "    [1] Modo Automatico (Varre as empresas da lista de forma aleatoria)" << endl;
    cout << "    [2] Modo Escolher Faixa (Voce digita de onde comeca e onde termina)" << endl;
    cout << "    Escolha uma opcao: " << AMARELO;
    int modo_escolhido;
    cin >> modo_escolhido;
    cout << RESET;

    int bloco_inicio = 167;
    int bloco_fim = 51;
    bool usar_modo_escolhido = false;

    if (modo_escolhido == 2) {
        usar_modo_escolhido = true;
        cout << "\n    " << CIANO << "[+]" << RESET << " Digite o bloco para comecar (Ex: 167): " << AMARELO;
        cin >> bloco_inicio;
        cout << "    " << CIANO << "[+]" << RESET << " Digite o bloco para terminar (Ex: 51): " << AMARELO;
        cin >> bloco_fim;
        cout << RESET;
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> rodar_faixa(0, faixas_alvo.size() - 1);
    uniform_int_distribution<> aleatorio(1, 254);

    cout << CIANO << "\n[*] Zodiac Alpha-Hunter rodando. Pressione CTRL + C para ver o relatorio final.\n" << RESET << endl;

    // EXECUÇÃO DO MODO SELECIONADO
    if (usar_modo_escolhido) {
        int direcao = (bloco_inicio <= bloco_fim) ? 1 : -1;
        
        for (int o1 = bloco_inicio; o1 != bloco_fim + direcao; o1 += direcao) {
            for (int o2 = 1; o2 <= 254; ++o2) {
                for (int o3 = 0; o3 <= 255; ++o3) {
                    for (int o4 = 1; o4 <= 254; ++o4) {
                        
                        string ip_atual = to_string(o1) + "." + to_string(o2) + "." + to_string(o3) + "." + to_string(o4);

                        for (int porta_atual : portas_alvo) {
                            total_testado++;

                            cout << AMARELO << "[ ZODIAC FAIXA ] ➔ " << ip_atual << ":" << porta_atual 
                                 << " | Varreduras: " << total_testado << "\r" << flush;

                            string dados_servidor = "";
                            if (validar_protocolo_minecraft(ip_atual, porta_atual, dados_servidor)) {
                                string resultado_formatado = ip_atual + ":" + to_string(porta_atual);
                                
                                cout << VERDE << NEGRITO << "\n\a[🔥 ZODIAC DETECTED ⚡] ➔ " << resultado_formatado << RESET << endl;
                                cout << CIANO << "   ➔ Protocolo Validado: " << AMARELO << dados_servidor << RESET << endl;
                                
                                servidores_descobertos.push_back(resultado_formatado);

                                ofstream arquivo_disco(caminho_salvamento, ios::app);
                                if (arquivo_disco.is_open()) {
                                    arquivo_disco << "[🔥] " << resultado_formatado << " -> " << dados_servidor << "\n";
                                    arquivo_disco.close();
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        // MODO AUTOMÁTICO RECONSTRUÍDO E FECHADO
        while (true) {
            FaixaIP faixa = faixas_alvo[rodar_faixa(gen)];
            uniform_int_distribution<> octeto3(faixa.min_o3, faixa.max_o3);
            string ip_atual = faixa.base + "." + to_string(octeto3(gen)) + "." + to_string(aleatorio(gen));

            for (int porta_atual : portas_alvo) {
                total_testado++;

                cout << AMARELO << "[ ZODIAC AUTO ] ➔ " << ip_atual << ":" << porta_atual 
                     << " | Varreduras: " << total_testado << "\r" << flush;

                string dados_servidor = "";
                if (validar_protocolo_minecraft(ip_atual, porta_atual, dados_servidor)) {
                    string resultado_formatado = ip_atual + ":" + to_string(porta_atual);
                    
                    cout << VERDE << NEGRITO << "\n\a[🔥 ZODIAC DETECTED ⚡] ➔ " << resultado_formatado << RESET << endl;
cout << CIANO << "   ➔ Protocolo Validado: " << AMARELO << dados_servidor << RESET << endl;servidores_descobertos.push_back(resultado_formatado);ofstream arquivo_disco(caminho_salvamento, ios::app);if (arquivo_disco.is_open()) {arquivo_disco << "[🔥] " << resultado_formatado << " -> " << dados_servidor << "\n";arquivo_disco.close();}}}}}return 0;}
