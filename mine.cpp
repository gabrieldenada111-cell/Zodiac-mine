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
    timeout.tv_usec = 350000; 
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

       // FAIXAS ATUALIZADAS: Foco total em Data Centers Globais de Servidores de Jogos (Maior densidade de portas 25565 do mundo)
    vector<FaixaIP> faixas_alvo = {
        {"51.81", 0, 255},    // OVH América do Norte (A maior densidade de servidores de blocos do planeta)
        {"142.44", 0, 255},   // OVH Canadá (Onde a maioria das redes de amigos aluga servidores baratos)
        {"192.99", 0, 255},   // OVH Montreal
        {"151.106", 0, 255},  // Hostinger VPS (Focada em servidores de jogos privados)
        {"129.151", 0, 255},  // Oracle Cloud São Paulo (Faixa grátis muito usada no BR)
        {"144.22", 0, 255},   // Oracle Cloud Brasil
        {"66.70", 0, 255},    // Data Centers compartilhados de jogos (EUA)
        {"167.114", 0, 255}   // Redes dedicadas a servidores cooperativos de jogadores
    };


    random_device rd;
    mt19937 gen(rd());
    
    uniform_int_distribution<> rodar_faixa(0, faixas_alvo.size() - 1);
    uniform_int_distribution<> octeto4(1, 254);

    cout << CIANO << "\n[*] Zodiac Alpha-Hunter + Auto-Save ativo em Downloads." << RESET << endl;
    cout << AMARELO << "[*] Pressione CTRL + C para exibir o relatorio final consolidado.\n" << RESET << endl;

    while (true) {
        FaixaIP faixa = faixas_alvo[rodar_faixa(gen)];
        uniform_int_distribution<> octeto3(faixa.min_o3, faixa.max_o3);
        string ip_atual = faixa.base + "." + to_string(octeto3(gen)) + "." + to_string(octeto4(gen));

        for (int porta_atual : portas_alvo) {
            total_testado++;

            cout << AMARELO << "[ ZODIAC ALPHA ] ➔ " << ip_atual << ":" << porta_atual 
                 << " | Varreduras: " << total_testado << "\r" << flush;

            string dados_servidor = "";
            if (validar_protocolo_minecraft(ip_atual, porta_atual, dados_servidor)) {
                string resultado_formatado = ip_atual + ":" + to_string(porta_atual);
                
                // 1. Exibição com som e emoji na tela
                cout << VERDE << NEGRITO << "\n\a[🔥 ZODIAC DETECTED ⚡] ➔ " << resultado_formatado << RESET << endl;
                cout << CIANO << "   ➔ Protocolo Validado: " << AMARELO << dados_servidor << RESET << endl;
                
                // 2. Salva na memória para o relatório final
                servidores_descobertos.push_back(resultado_formatado);

                // 3. ROTINA DE AUTO-SAVE: Grava em tempo real no arquivo de texto (modo append)
                ofstream arquivo_disco(caminho_salvamento, ios::app);
                if (arquivo_disco.is_open()) {
                    arquivo_disco << "[🔥] " << resultado_formatado << " -> " << dados_servidor << "\n";
                    arquivo_disco.close();
                }
            }
        }
    }

    return 0;
}
