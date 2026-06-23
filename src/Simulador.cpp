#include "Simulador.hpp"
#include "No.hpp"
#include "Enlace.hpp"
#include "Validacao.hpp"
#include <iostream>
#include <sstream>
#include <vector>

// ── Helpers locais ────────────────────────────────────────────────────────────
static std::vector<std::string> tokenizar(const std::string& linha) {
    std::vector<std::string> tokens;

    if (linha.empty()) {
        return tokens;
    }

    std::istringstream ss(linha);
    std::string tok;

    while (ss >> tok) {
        tokens.push_back(tok);
    }

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

void Simulador::coletarMetricas() {
    coletor_.reset();
    for (Enlace* e : topologia_.getEnlaces()) {
        if (e) e->reportarMetricas(coletor_);
    }
    for (No* n : topologia_.getNos()) {
        if (n) {
            coletor_.registrarNo(n->getId(),
                                 n->getPacotesEnviados(),
                                 n->getPacotesRecebidos(),
                                 n->getPacotesDescartados());
        }
    }
}

void Simulador::encerrar() {
    coletarMetricas();
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
    try {
        // Validar parâmetros
        Validacao::validarNaoVazio(origem, "Origem");
        Validacao::validarNaoVazio(destino, "Destino");

        if (origem == destino) {
            throw ExcecaoRede("Origem e destino não podem ser iguais");
        }

        if (estado_ != EstadoSimulacao::EM_EXECUCAO) {
            throw ExcecaoEstado("Inicie a simulação antes de injetar pacotes");
        }

        // Verificar se nós existem
        No* noOrigem = topologia_.getNo(origem);
        No* noDestino = topologia_.getNo(destino);
        
        if (!noOrigem) {
            throw ExcecaoRede("Nó de origem '" + origem + "' não existe");
        }
        if (!noDestino) {
            throw ExcecaoRede("Nó de destino '" + destino + "' não existe");
        }

        int t = escalonador_.getTempoAtual();
        std::cout << "[t=" << t << "] Pacote injetado: "
                  << origem << " -> " << destino << "\n";

        std::vector<std::string> caminho =
            protocolo_->calcularCaminho(topologia_, origem, destino);

        if (caminho.empty()) {
            std::cout << "[t=" << t << "] Nenhuma rota disponível. Pacote descartado.\n";
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
                if (!e) continue;  // Validação defensiva
                
                No* outro = nullptr;
                if (e->getNoA() && e->getNoA()->getId() == de) {
                    outro = e->getNoB();
                } else if (e->getNoB() && e->getNoB()->getId() == de) {
                    outro = e->getNoA();
                }
                
                if (outro && outro->getId() == para) {
                    enlaceUsado = e;
                    break;
                }
            }

            if (!enlaceUsado) {
                throw ExcecaoRede("Enlace entre " + de + " e " + para + 
                                " não encontrado");
            }

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

    } catch (const ExcecaoRede& e) {
        std::cout << "[" << e.what() << "]\n";
    } catch (const ExcecaoEstado& e) {
        std::cout << "[" << e.what() << "]\n";
    } catch (const ExcecaoMemoria& e) {
        std::cout << "[" << e.what() << "]\n";
    } catch (const std::exception& e) {
        std::cout << "[ERRO] Falha ao injetar pacote: " << e.what() << "\n";
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
    try {
        auto tokens = tokenizar(linha);
        if (tokens.empty()) return;

        const std::string& cmd = tokens[0];

        if (cmd == "ajuda") {
            exibirAjuda();

        } else if (cmd == "add_no") {
            // add_no <tipo> <id>
            if (tokens.size() < 3) {
                throw ExcecaoEntrada("Uso: add_no <tipo> <id>");
            }
            const std::string& tipo = tokens[1];
            const std::string& id   = tokens[2];
            
            // Validar ID
            Validacao::validarNaoVazio(id, "ID do nó");
            
            std::unique_ptr<No> no;
            if (tipo == "host") {
                no = std::make_unique<Host>(id, id + ".local");
            } else if (tipo == "roteador") {
                no = std::make_unique<Roteador>(id, id + ".local");
            } else if (tipo == "switch") {
                no = std::make_unique<Switch>(id, id + ".local");
            } else {
                throw ExcecaoEntrada("Tipo inválido. Use: host, roteador ou switch");
            }
            
            if (topologia_.adicionarNo(std::move(no))) {
                std::cout << "[OK] Nó " << id << " (" << tipo << ") adicionado.\n";
            } else {
                throw ExcecaoRede("Nó '" + id + "' já existe ou ID inválido");
            }

        } else if (cmd == "add_enlace") {
            // add_enlace <id> <noA> <noB> latencia=<ms>
            if (tokens.size() < 5) {
                throw ExcecaoEntrada("Uso: add_enlace <id> <noA> <noB> latencia=<ms>");
            }
            const std::string& idE  = tokens[1];
            const std::string& idA  = tokens[2];
            const std::string& idB  = tokens[3];

            // Validar IDs
            Validacao::validarNaoVazio(idE, "ID do enlace");
            Validacao::validarNaoVazio(idA, "ID do nó A");
            Validacao::validarNaoVazio(idB, "ID do nó B");

            if (idA == idB) {
                throw ExcecaoRede("Enlace não pode conectar um nó a si mesmo");
            }

            // Parseia "latencia=<valor>"
            double latencia = 1.0;
            for (size_t i = 4; i < tokens.size(); ++i) {
                if (tokens[i].rfind("latencia=", 0) == 0) {
                    std::string valorStr = tokens[i].substr(9);
                    latencia = Validacao::parseDouble(valorStr);
                    if (latencia < 0.0) {
                        throw ExcecaoEntrada("Latência não pode ser negativa");
                    }
                }
            }

            No* noA = topologia_.getNo(idA);
            No* noB = topologia_.getNo(idB);
            if (!noA || !noB) {
                if (!noA) throw ExcecaoRede("Nó '" + idA + "' não existe");
                if (!noB) throw ExcecaoRede("Nó '" + idB + "' não existe");
            }
            
            auto enlace = std::make_unique<Enlace>(idE, noA, noB, 100.0, latencia);
            if (topologia_.adicionarEnlace(std::move(enlace))) {
                std::cout << "[OK] Enlace " << idE
                          << " adicionado: " << idA << " <-> " << idB << "\n";
            } else {
                throw ExcecaoRede("Não foi possível adicionar o enlace");
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
                throw ExcecaoEntrada("Uso: injetar <origem> <destino>");
            }
            injetarPacote(tokens[1], tokens[2]);

        } else if (cmd == "falha") {
            if (tokens.size() < 2) {
                throw ExcecaoEntrada("Uso: falha <idEnlace>");
            }
            alterarEstadoEnlace(tokens[1], false);

        } else if (cmd == "recuperar") {
            if (tokens.size() < 2) {
                throw ExcecaoEntrada("Uso: recuperar <idEnlace>");
            }
            alterarEstadoEnlace(tokens[1], true);

        } else if (cmd == "metricas") {
            coletarMetricas();
            coletor_.exibirResumo();

        } else if (cmd == "exportar") {
            if (tokens.size() < 2) {
                throw ExcecaoEntrada("Uso: exportar <arquivo.csv>");
            }
            coletarMetricas();
            if (coletor_.exportarCSV(tokens[1])) {
                std::cout << "[OK] Métricas exportadas para " << tokens[1] << "\n";
            } else {
                throw ExcecaoArquivo("Não foi possível exportar para " + tokens[1]);
            }

        } else if (cmd == "encerrar") {
            encerrar();

        } else {
            throw ExcecaoEntrada("Comando desconhecido: '" + cmd + 
                               "'. Digite 'ajuda' para ver os comandos");
        }

    } catch (const ExcecaoEntrada& e) {
        std::cout << "[" << e.what() << "]\n";
    } catch (const ExcecaoRede& e) {
        std::cout << "[" << e.what() << "]\n";
    } catch (const ExcecaoArquivo& e) {
        std::cout << "[" << e.what() << "]\n";
    } catch (const ExcecaoMemoria& e) {
        std::cout << "[" << e.what() << "]\n";
    } catch (const ExcecaoSimulador& e) {
        std::cout << "[ERRO_GERAL] " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cout << "[ERRO_DESCONHECIDO] " << e.what() << "\n";
    } catch (...) {
        std::cout << "[ERRO_FATAIS] Exceção desconhecida (não é std::exception)\n";
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