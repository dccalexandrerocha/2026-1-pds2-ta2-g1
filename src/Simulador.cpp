#include "Simulador.hpp"
#include "No.hpp"
#include "Enlace.hpp"
#include <iostream>
#include <sstream>
#include <vector>

// ── Helpers locais ────────────────────────────────────────────────────────────

static std::vector<std::string> tokenizar(const std::string& linha) {
    std::istringstream ss(linha);
    std::vector<std::string> tokens;
    std::string tok;
    while (ss >> tok) tokens.push_back(tok);
    return tokens;
}

// ── Construtor ────────────────────────────────────────────────────────────────

Simulador::Simulador()
    : estado_(EstadoSimulacao::PARADO),
      protocolo_(std::make_unique<RoteamentoDijkstra>())
{}

// ── REPL principal ────────────────────────────────────────────────────────────

void Simulador::executar() {
    std::cout << "Simulador de Rede | digite 'ajuda' para ver os comandos.\n";
    std::string linha;
    while (std::getline(std::cin, linha)) {
        if (linha.empty()) continue;
        processarComando(linha);
        if (estado_ == EstadoSimulacao::ENCERRADO) break;
    }
}

// ── Ciclo de vida ─────────────────────────────────────────────────────────────

void Simulador::iniciar() {
    if (estado_ != EstadoSimulacao::PARADO) {
        std::cout << "[ERRO] Simulacao ja iniciada.\n";
        return;
    }
    if (!topologia_.validarConectividade()) {
        std::cout << "[ERRO] Topologia invalida. Verifique os nos isolados.\n";
        return;
    }
    estado_ = EstadoSimulacao::EM_EXECUCAO;
    int t = escalonador_.getTempoAtual();
    std::cout << "[t=" << t << "] Topologia validada: "
              << topologia_.getNos().size()    << " nos, "
              << topologia_.getEnlaces().size()<< " enlaces.\n"
              << "[t=" << t << "] Simulacao iniciada.\n";
}

void Simulador::pausar() {
    if (estado_ != EstadoSimulacao::EM_EXECUCAO) {
        std::cout << "[ERRO] Simulacao nao esta em execucao.\n";
        return;
    }
    estado_ = EstadoSimulacao::PAUSADO;
    std::cout << "[t=" << escalonador_.getTempoAtual() << "] Simulacao pausada.\n";
}

void Simulador::retomar() {
    if (estado_ != EstadoSimulacao::PAUSADO) {
        std::cout << "[ERRO] Simulacao nao esta pausada.\n";
        return;
    }
    estado_ = EstadoSimulacao::EM_EXECUCAO;
    std::cout << "[t=" << escalonador_.getTempoAtual() << "] Simulacao retomada.\n";
}

void Simulador::encerrar() {
    coletor_.exibirResumo();
    estado_ = EstadoSimulacao::ENCERRADO;
    std::cout << "[t=" << escalonador_.getTempoAtual() << "] Simulacao encerrada.\n";
}

void Simulador::avancarPasso() {
    if (estado_ != EstadoSimulacao::EM_EXECUCAO) {
        std::cout << "[ERRO] Simulacao nao esta em execucao.\n";
        return;
    }
    escalonador_.avancarPasso();
}

// ── Injetar pacote ────────────────────────────────────────────────────────────

void Simulador::injetarPacote(const std::string& origem,
                              const std::string& destino)
{
    if (estado_ != EstadoSimulacao::EM_EXECUCAO) {
        std::cout << "[ERRO] Inicie a simulacao antes de injetar pacotes.\n";
        return;
    }

    int t = escalonador_.getTempoAtual();
    std::cout << "[t=" << t << "] Pacote injetado: "
              << origem << " -> " << destino << "\n";

    std::vector<std::string> caminho =
        protocolo_->calcularCaminho(topologia_, origem, destino);

    if (caminho.empty()) {
        std::cout << "[t=" << t << "] Nenhuma rota disponivel. Pacote descartado.\n";
        coletor_.registrarNo(origem, 0, 0, 1);
        return;
    }

    // Exibe o caminho calculado
    std::cout << "[t=" << t << "] Caminho calculado: ";
    for (size_t i = 0; i < caminho.size(); ++i) {
        if (i) std::cout << " -> ";
        std::cout << caminho[i];
    }
    std::cout << "\n";

    // Agenda a transmissao salto a salto
    for (size_t i = 0; i + 1 < caminho.size(); ++i) {
        const std::string& de   = caminho[i];
        const std::string& para = caminho[i + 1];
        int instante = t + static_cast<int>(i) + 1;

        // Procura o enlace que conecta de <-> para
        Enlace* enlaceUsado = nullptr;
        for (Enlace* e : topologia_.getVizinhos(de)) {
            No* outro = (e->getNoA()->getId() == de) ? e->getNoB() : e->getNoA();
            if (outro && outro->getId() == para) {
                enlaceUsado = e;
                break;
            }
        }

        if (!enlaceUsado) continue;

        std::string deCopy   = de;
        std::string paraCopy = para;
        Enlace* elCopy       = enlaceUsado;

        escalonador_.agendarEvento(
            instante,
            "",  // mensagem impressa dentro de transmitir()
            [deCopy, paraCopy, elCopy, instante]() {
                elCopy->transmitir(deCopy, paraCopy, instante);
            }
        );
    }
}

// ── Estado de enlace ──────────────────────────────────────────────────────────

void Simulador::alterarEstadoEnlace(const std::string& idEnlace, bool ativo) {
    Enlace* e = topologia_.getEnlace(idEnlace);
    if (!e) {
        std::cout << "[ERRO] Enlace '" << idEnlace << "' nao encontrado.\n";
        return;
    }
    e->setAtivo(ativo);
    int t = escalonador_.getTempoAtual();
    if (ativo) {
        std::cout << "[t=" << t << "] Enlace " << idEnlace << " restaurado.\n";
    } else {
        std::cout << "[t=" << t << "] Enlace " << idEnlace << " marcado como falho.\n";
        std::cout << "[t=" << t << "] Recalculando rotas...\n";
    }
}

EstadoSimulacao Simulador::getEstado() const { return estado_; }

// ── Processamento de comandos ─────────────────────────────────────────────────

void Simulador::processarComando(const std::string& linha) {
    auto tokens = tokenizar(linha);
    if (tokens.empty()) return;

    const std::string& cmd = tokens[0];

    if (cmd == "ajuda") {
        exibirAjuda();

    } else if (cmd == "add_no") {
        // add_no <tipo> <id>
        if (tokens.size() < 3) {
            std::cout << "[ERRO] Uso: add_no <tipo> <id>\n";
            return;
        }
        const std::string& tipo = tokens[1];
        const std::string& id   = tokens[2];
        std::unique_ptr<No> no;
        if (tipo == "host") {
            no = std::make_unique<Host>(id, id + ".local");
        } else if (tipo == "roteador") {
            no = std::make_unique<Roteador>(id, id + ".local");
        } else if (tipo == "switch") {
            no = std::make_unique<Switch>(id, id + ".local");
        } else {
            std::cout << "[ERRO] Tipo invalido. Use: host, roteador ou switch.\n";
            return;
        }
        if (topologia_.adicionarNo(std::move(no))) {
            std::cout << "[OK] No " << id << " (" << tipo << ") adicionado.\n";
        } else {
            std::cout << "[ERRO] No '" << id << "' ja existe.\n";
        }

    } else if (cmd == "add_enlace") {
        // add_enlace <id> <noA> <noB> latencia=<ms>
        if (tokens.size() < 5) {
            std::cout << "[ERRO] Uso: add_enlace <id> <noA> <noB> latencia=<ms>\n";
            return;
        }
        const std::string& idE  = tokens[1];
        const std::string& idA  = tokens[2];
        const std::string& idB  = tokens[3];

        double latencia = 1.0;
        // Parseia "latencia=<valor>"
        for (size_t i = 4; i < tokens.size(); ++i) {
            if (tokens[i].rfind("latencia=", 0) == 0) {
                latencia = std::stod(tokens[i].substr(9));
            }
        }

        No* noA = topologia_.getNo(idA);
        No* noB = topologia_.getNo(idB);
        if (!noA || !noB) {
            std::cout << "[ERRO] Um ou ambos os nos nao existem.\n";
            return;
        }
        auto enlace = std::make_unique<Enlace>(idE, noA, noB, 100.0, latencia);
        if (topologia_.adicionarEnlace(std::move(enlace))) {
            std::cout << "[OK] Enlace " << idE
                      << " adicionado: " << idA << " <-> " << idB << "\n";
        } else {
            std::cout << "[ERRO] Nao foi possivel adicionar o enlace.\n";
        }

    } else if (cmd == "iniciar") {
        iniciar();

    } else if (cmd == "pausar") {
        pausar();

    } else if (cmd == "retomar") {
        retomar();

    } else if (cmd == "proximo") {
        avancarPasso();

    } else if (cmd == "injetar") {
        if (tokens.size() < 3) {
            std::cout << "[ERRO] Uso: injetar <origem> <destino>\n";
            return;
        }
        injetarPacote(tokens[1], tokens[2]);

    } else if (cmd == "falha") {
        if (tokens.size() < 2) {
            std::cout << "[ERRO] Uso: falha <idEnlace>\n";
            return;
        }
        alterarEstadoEnlace(tokens[1], false);

    } else if (cmd == "recuperar") {
        if (tokens.size() < 2) {
            std::cout << "[ERRO] Uso: recuperar <idEnlace>\n";
            return;
        }
        alterarEstadoEnlace(tokens[1], true);

    } else if (cmd == "metricas") {
        coletor_.exibirResumo();

    } else if (cmd == "exportar") {
        if (tokens.size() < 2) {
            std::cout << "[ERRO] Uso: exportar <arquivo.csv>\n";
            return;
        }
        if (coletor_.exportarCSV(tokens[1])) {
            std::cout << "[OK] Metricas exportadas para " << tokens[1] << "\n";
        } else {
            std::cout << "[ERRO] Nao foi possivel exportar para " << tokens[1] << "\n";
        }

    } else if (cmd == "encerrar") {
        encerrar();

    } else {
        std::cout << "[ERRO] Comando desconhecido: '" << cmd
                  << "'. Digite 'ajuda' para ver os comandos.\n";
    }
}

void Simulador::exibirAjuda() const {
    std::cout <<
        "\nComandos disponiveis:\n"
        "  add_no <tipo> <id>                    Adiciona um no (host/roteador/switch)\n"
        "  add_enlace <id> <noA> <noB> latencia=<ms>  Conecta dois nos\n"
        "  iniciar                               Valida e inicia a simulacao\n"
        "  pausar                                Pausa a simulacao\n"
        "  retomar                               Retoma a simulacao pausada\n"
        "  proximo                               Avanca um passo de tempo\n"
        "  injetar <origem> <destino>            Injeta um pacote\n"
        "  falha <idEnlace>                      Marca enlace como falho\n"
        "  recuperar <idEnlace>                  Restaura enlace falho\n"
        "  metricas                              Exibe metricas no terminal\n"
        "  exportar <arquivo.csv>                Exporta metricas para CSV\n"
        "  encerrar                              Encerra a simulacao\n"
        "  ajuda                                 Lista os comandos\n\n";
}
