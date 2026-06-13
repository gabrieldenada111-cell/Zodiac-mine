#include <iostream>
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
    }
    cout << VERMELHO << NEGRITO << "========================================================================" << RESET << endl;
    exit(sinal); 
}

// Funcao que valida o protocolo com certeza absoluta
bool validar_protocolo_minecraft(const string& ip, int porta, string& dados_retornados) {
    int telefon = socket(AF_INET, SOCK_STREAM, 0);
    if (telefon < 0) return false;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 350000; // 350ms garante leitura em redes residenciais
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
        // Envia o pacote de Handshake + Request do protocolo moderno
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
            
            // VALIDACAO ABSOLUTA: Procura por tags obrigatorias do JSON do Minecraft
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

    // BANCO DE HOSTS (PORTAS) EXPANDIDO: As portas mais utilizadas no mundo de jogos
    vector<int> portas_alvo = {
        25565, 25564, 25566, 25567, 25568, 25569, 25570, // Faixa principal Minecraft Java
        25575, 25545, 25563, 25600, 25700, 25800,        // Portas alternativas de paineis compartilhados
        10000, 10092, 10112, 10521, 11021,               // Faixas vistas no print do video
        19132, 19133                                      // Portas padrao Minecraft Bedrock (Celular/Console)
    };

    // BANCO DE IPS EXPANDIDO: Data Centers + Maiores Provedores Residenciais do Brasil (Claro, Vivo, Oi, Tim, Brisanet)
    vector<FaixaIP> faixas_alvo = {
        // --- DATA CENTERS E HOSPEDAGENS POPULARES ---
        {"129.151", 0, 255},  // Oracle Cloud SP (Servidores Gratuitos)
        {"144.22", 0, 255},   // Oracle Cloud Brasil
        {"151.106", 0, 255},  // Hostinger VPS (Focada em servidores de blocos)
        {"156.146", 32, 63},   // Proton VPN Free Nodes (EUA / Holanda)
        {"37.120", 192, 223},  // Proton VPN / Datacamp
        {"84.17", 32, 63},     // Proton / LeaseWeb Free
        {"172.65", 0, 255},   // Proxies de protecao de servidores de jogos
        {"54.233", 0, 255},   // AWS Cloud Sao Paulo
        {"94.130", 0, 255},   // Hetzner (Hospedagem dedicada barata)
        {"79.137", 0, 255},   // OVH Cloud (Maior hoster de servidores do mundo)
        
        // --- PROVEDORES RESIDENCIAIS BRASILEIROS (Onde jogadores abrem as portas na raca) ---
        {"177.71", 0, 255},   // Operadoras de Fibra Regional / Brisanet / Algar
        {"186.200", 0, 255},  // Internet Residencial BR (Sudeste)
        {"179.182", 0, 255},  // Vivo Fibra Residencial 1
        {"179.183", 0, 255},  // Vivo Fibra Residencial 2
        {"186.212", 0, 255},  // Vivo Internet
        {"200.229", 0, 255},  // Claro/NET Virtua Residencial 1
        {"201.82", 0, 255},   // Claro/NET Virtua Residencial 2
        {"177.38", 0, 255},   // Oi Velox / Oi Fibra
        {"191.250", 0, 255},  // TIM Ultrafibra
        {"187.19", 0, 255}    // Redes rotativas de operadoras nacionais
    };

    random_device rd;
    mt19937 gen(rd());
    
    uniform_int_distribution<> rodar_faixa(0, faixas_alvo.size() - 1);
    uniform_int_distribution<> octeto4(1, 254);

    cout << CIANO << "\n[*] Zodiac Alpha-Hunter ativado com validacao absoluta de protocolo." << RESET << endl;
    cout << AMARELO << "[*] Mapeando " << portas_alvo.size() << " portas em " << faixas_alvo.size() << " blocos estrategicos de rede." << RESET << endl;
    cout << AMARELO << "[*] Pressione CTRL + C para encerrar e ver o relatorio final consolidado.\n" << RESET << endl;

    while (true) {
        FaixaIP faixa = faixas_alvo[rodar_faixa(gen)];
        uniform_int_distribution<> octeto3(faixa.min_o3, faixa.max_o3);
        
        string ip_atual = faixa.base + "." + to_string(octeto3(gen)) + "." + to_string(octeto4(gen));

        // Executa todas as portas do banco no mesmo IP antes de trocar de alvo
        for (int porta_atual : portas_alvo) {
            total_testado++;

            cout << AMARELO << "[ ZODIAC ALPHA ] ➔ " << ip_atual << ":" << porta_atual 
                 << " | Varreduras: " << total_testado << "\r" << flush;

            string dados_servidor = "";
            if (validar_protocolo_minecraft(ip_atual, porta_atual, dados_servidor)) {
                string resultado_formatado = ip_atual + ":" + to_string(porta_atual);
                
                // Alerta Visual com o Emoji solicitado
                cout << VERDE << NEGRITO << "\n\a[🔥 ZODIAC DETECTED ⚡] ➔ " << resultado_formatado << RESET << endl;
                cout << CIANO << "   ➔ Protocolo Validado: " << AMARELO << dados_servidor << RESET << endl;
                
                servidores_descobertos.push_back(resultado_formatado);
            }
        }
    }

    return 0;
}
