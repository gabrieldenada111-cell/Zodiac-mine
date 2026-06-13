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
    cout << CIANO << "[*] Servidores ativos encontrados: " << servidores_descobertos.size() << "\n" << RESET << endl;

    if (servidores_descobertos.empty()) {
        cout << VERMELHO << "[-] Nenhuma sala ativa capturada no periodo." << RESET << endl;
    } else {
        cout << VERDE << NEGRITO << "[+] LISTA DE IPS COLETADOS:" << RESET << endl;
        for (const auto& ip_salvo : servidores_descobertos) {
            cout << VERDE << "    ➔ " << ip_salvo << RESET << endl;
        }
    }
    cout << VERMELHO << NEGRITO << "========================================================================" << RESET << endl;
    exit(sinal);
}

bool checar_porta_ativa(const string& ip, int porta) {
    int telefon = socket(AF_INET, SOCK_STREAM, 0);
    if (telefon < 0) return false;

    // ACELERAÇÃO: Timeout reduzido para 120ms para máxima velocidade
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 120000;
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

        char buffer[128];
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(telefon, buffer, sizeof(buffer) - 1, 0);
        close(telefon);

        return true;
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

    vector<int> portas_do_print = {
        25565, 25564, 25566, 25575, 25545, 25563,
        10000, 10092, 10112, 10521, 11021,
        19132
    };

    vector<FaixaIP> faixas_alvo = {
        {"156.146", 32, 63},   // Proton VPN Free Nodes (EUA / Holanda)
        {"37.120", 192, 223},  // Proton VPN / Datacamp
        {"185.159", 156, 159}, // Infraestrutura Proton
        {"84.17", 32, 63},     // Proton / LeaseWeb Free
        {"212.102", 32, 63},   // Proton Free EUA
        {"177.71", 0, 255},    // Fibras Regionais BR
        {"186.200", 0, 255},   // Internet Residencial BR
        {"179.182", 0, 255},   // Vivo Fibra Rotativo
        {"200.229", 0, 255},   // Claro/NET Virtua
        {"129.151", 0, 255},   // Oracle Cloud SP
        {"151.106", 0, 255}    // Hostinger BR
    };

    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<> rodar_faixa(0, faixas_alvo.size() - 1);
    uniform_int_distribution<> octeto4(1, 254);

    cout << CIANO << "\n[*] Caçador em modo varredura por bloco de portas ativo." << RESET << endl;
    cout << AMARELO << "[*] Pressione CTRL + C para fechar e exibir a lista coletada.\n" << RESET << endl;

    while (true) {
        FaixaIP faixa = faixas_alvo[rodar_faixa(gen)];
        uniform_int_distribution<> octeto3(faixa.min_o3, faixa.max_o3);

        // Sorteia um IP base
        string ip_atual = faixa.base + "." + to_string(octeto3(gen)) + "." + to_string(octeto4(gen));

        // MUDANÇA LOGÍSTICA: Testa TODAS as portas do vetor nesse mesmo IP antes de ir para o próximo
        for (int porta_atual : portas_do_print) {
            total_testado++;

            // Exibe o progresso dinâmico na mesma linha
            cout << AMARELO << "[ ZODIAC SCAN ] ➔ " << ip_atual << ":" << porta_atual
                 << " | Varreduras: " << total_testado << "\r" << flush;

            if (checar_porta_ativa(ip_atual, porta_atual)) {
                string resultado_formatado = ip_atual + ":" + to_string(porta_atual);

                // Exibe o aviso com estilo e emoji [🔥 ZODIAC DETECTED ⚡]
                cout << VERDE << NEGRITO << "\n\a[🔥 ZODIAC DETECTED ⚡] ➔ " << resultado_formatado << RESET << endl;

                servidores_descobertos.push_back(resultado_formatado);
            }
        }
    }

    return 0;
}
