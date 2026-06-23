/**
 * @file Simulador.cpp
 * @brief Implementacao do orquestrador principal da simulacao.
 *
 * Contem o REPL (Read-Eval-Print Loop) que interpreta comandos do
 * usuario, o ciclo de vida da simulacao (iniciar/pausar/retomar/
 * encerrar) e a logica de injecao de pacotes com agendamento de
 * eventos salto a salto.
 *
 * A interface do terminal foi pensada para legibilidade: cabecalhos
 * delimitados por linhas horizontais, colunas alinhadas na tabela
 * de comandos e separadores visuais entre secoes de saida.
 *
 * @author Grupo 1 - PDS2 TA2 2026/1
 */

#include "Simulador.hpp"
#include "No.hpp"
#include "Enlace.hpp"
#include "Validacao.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

// ── Helpers locais ────────────────────────────────────────────────────────────

/**
 * @brief Divide uma linha de texto em tokens separados por espaco.
 *
 * @param linha Linha lida do terminal.
 * @return Vetor de strings com os tokens encontrados.
 */
static std::vector<std::string> tokenizar(const std::string& linha) {
    std::vector<std::string> tokens;
    if (linha.empty()) return tokens;
    std::istringstream ss(linha);
    std::string tok;
    while (ss >> tok) tokens.push_back(tok);
    return tokens;
}

/**
 * @brief Imprime uma linha horizontal de separacao.
 *
 * Usada para delimitar cabecalhos e rodapes de secoes na saida
 * do terminal, melhorando a legibilidade visual.
 *
 * @param largura Numero de caracteres da linha (padrao 60).
 */
static void imprimirSeparador(int largura = 60) {
    std::cout << std::string(largura, '-') << "\n";
}

// ── Construtor ────────────────────────────────────────────────────────────────

/**
 * @brief Inicializa o simulador no estado PARADO com Dijkstra como protocolo.
 */
Simulador::Simulador()
    : estado_(EstadoSimulacao::PARADO),
      protocolo_(std::make_unique<RoteamentoDijkstra>())
{}

// ── REPL principal ────────────────────────────────────────────────────────────

/**
 * @brief Loop principal de leitura de comandos (REPL).
 *
 * Exibe o banner de boas-vindas e le comandos do stdin ate que o
 * usuario encerre a simulacao ou o stream seja fechado.
 */
void Simulador::executar() {
    imprimirSeparador();
    std::cout << "  Simulador de Rede de Telecomunicacao\n";
    std::cout << "  PDS2 TA2 2026/1 | Grupo 1\n";
    imprimirSeparador();
    std::cout << "  Digite 'ajuda' para ver os comandos disponiveis.\n";
    imprimirSeparador();
    std::cout << "\n";

    std::string linha;
    while (std::getline(std::cin, linha)) {
        if (linha.empty()) continue;
        processarComando(linha);
        if (estado_ == EstadoSimulacao::ENCERRADO) break;
    }
}

// ── Ciclo de vida ─────────────────────────────────────────────────────────────

/**
 * @brief Valida a topologia e transiciona para o estado EM_EXECUCAO.
 *
 * So pode ser chamado quando o simulador esta no estado PARADO.
 * Exibe a contagem de nos e enlaces apos validacao bem-sucedida.
 */
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

/**
 * @brief Pausa a simulacao em execucao.
 *
 * Transiciona de EM_EXECUCAO para PAUSADO. Pacotes agendados
 * aguardam ate que retomar() seja chamado.
 */
void Simulador::pausar() {
    if (estado_ != EstadoSimulacao::EM_EXECUCAO) {
        std::cout << "[ERRO] Simulacao nao esta em execucao.\n";
        return;
    }
    estado_ = EstadoSimulacao::PAUSADO;
    std::cout << "[t=" << escalonador_.getTempoAtual() << "] Simulacao pausada.\n";
}

/**
 * @brief Retoma a simulacao pausada.
 *
 * Transiciona de PAUSADO para EM_EXECUCAO.
 */
void Simulador::retomar() {
    if (estado_ != EstadoSimulacao::PAUSADO) {
        std::cout << "[ERRO] Simulacao nao esta pausada.\n";
        return;
    }
    estado_ = EstadoSimulacao::EM_EXECUCAO;
    std::cout << "[t=" << escalonador_.getTempoAtual() << "] Simulacao retomada.\n";
}

/**
 * @brief Coleta metricas de todos os nos e enlaces para o coletor interno.
 *
 * Reseta o coletor antes de coletar para evitar acumulo duplo entre
 * chamadas consecutivas.
 */
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

/**
 * @brief Encerra a simulacao, exibe metricas finais e muda o estado.
 *
 * Coleta e imprime o resumo de metricas antes de sair do loop
 * principal. Apos este metodo o REPL encerra.
 */
void Simulador::encerrar() {
    coletarMetricas();
    coletor_.exibirResumo();
    estado_ = EstadoSimulacao::ENCERRADO;
    std::cout << "\n[t=" << escalonador_.getTempoAtual() << "] Simulacao encerrada.\n";
}

/**
 * @brief Avanca a simulacao em um passo de tempo.
 *
 * Delega ao EscalonadorDeEventos, que despacha todos os eventos
 * cujo instante seja <= ao novo tempo atual.
 */
void Simulador::avancarPasso() {
    if (estado_ != EstadoSimulacao::EM_EXECUCAO) {
        std::cout << "[ERRO] Simulacao nao esta em execucao.\n";
        return;
    }
    escalonador_.avancarPasso();
}

// ── Injetar pacote ────────────────────────────────────────────────────────────

/**
 * @brief Injeta um pacote na rede e agenda os eventos de transmissao.
 *
 * Calcula o caminho via ProtocoloDeRoteamento e agenda um evento
 * por salto no EscalonadorDeEventos. Cada evento, ao ser despachado,
 * chama Enlace::transmitir() para o salto correspondente.
 *
 * Se nao houver caminho disponivel (por falta de conectividade ou
 * falha de enlace), o pacote e descartado e registrado no coletor.
 *
 * @throws ExcecaoRede    Nos invalidos ou enlace nao encontrado.
 * @throws ExcecaoEstado  Simulacao nao iniciada.
 */
void Simulador::injetarPacote(const std::string& origem,
                              const std::string& destino)
{
    try {
        Validacao::validarNaoVazio(origem, "Origem");
        Validacao::validarNaoVazio(destino, "Destino");

        if (origem == destino) {
            throw ExcecaoRede("Origem e destino nao podem ser iguais");
        }

        if (estado_ != EstadoSimulacao::EM_EXECUCAO) {
            throw ExcecaoEstado("Inicie a simulacao antes de injetar pacotes");
        }

        No* noOrigem  = topologia_.getNo(origem);
        No* noDestino = topologia_.getNo(destino);

        if (!noOrigem) {
            throw ExcecaoRede("No de origem '" + origem + "' nao existe");
        }
        if (!noDestino) {
            throw ExcecaoRede("No de destino '" + destino + "' nao existe");
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

            Enlace* enlaceUsado = nullptr;
            for (Enlace* e : topologia_.getVizinhos(de)) {
                if (!e) continue;

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
                                " nao encontrado");
            }

            std::string deCopy   = de;
            std::string paraCopy = para;
            Enlace* elCopy       = enlaceUsado;

            escalonador_.agendarEvento(
                instante,
                "",
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

/**
 * @brief Ativa ou desativa um enlace pelo ID.
 *
 * Ao desativar, exibe aviso de recalculo de rotas (o recalculo
 * efetivo ocorre na proxima chamada a calcularCaminho()).
 */
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

/**
 * @brief Interpreta e despacha um comando digitado pelo usuario.
 *
 * Tokeniza a linha, identifica o comando e chama o metodo
 * correspondente. Captura todas as excecoes do simulador e exibe
 * mensagens de erro formatadas sem interromper o REPL.
 *
 * Comandos suportados: add_no, add_enlace, iniciar, pausar,
 * retomar, proximo, injetar, falha, recuperar, metricas,
 * exportar, encerrar, ajuda.
 */
void Simulador::processarComando(const std::string& linha) {
    try {
        auto tokens = tokenizar(linha);
        if (tokens.empty()) return;

        const std::string& cmd = tokens[0];

        if (cmd == "ajuda") {
            exibirAjuda();

        } else if (cmd == "add_no") {
            if (tokens.size() < 3) {
                throw ExcecaoEntrada("Uso: add_no <tipo> <id>");
            }
            const std::string& tipo = tokens[1];
            const std::string& id   = tokens[2];

            Validacao::validarNaoVazio(id, "ID do no");

            std::unique_ptr<No> no;
            if (tipo == "host") {
                no = std::make_unique<Host>(id, id + ".local");
            } else if (tipo == "roteador") {
                no = std::make_unique<Roteador>(id, id + ".local");
            } else if (tipo == "switch") {
                no = std::make_unique<Switch>(id, id + ".local");
            } else {
                throw ExcecaoEntrada("Tipo invalido. Use: host, roteador ou switch");
            }

            if (topologia_.adicionarNo(std::move(no))) {
                std::cout << "[OK] No " << id << " (" << tipo << ") adicionado.\n";
            } else {
                throw ExcecaoRede("No '" + id + "' ja existe ou ID invalido");
            }

        } else if (cmd == "add_enlace") {
            if (tokens.size() < 5) {
                throw ExcecaoEntrada("Uso: add_enlace <id> <noA> <noB> latencia=<ms>");
            }
            const std::string& idE = tokens[1];
            const std::string& idA = tokens[2];
            const std::string& idB = tokens[3];

            Validacao::validarNaoVazio(idE, "ID do enlace");
            Validacao::validarNaoVazio(idA, "ID do no A");
            Validacao::validarNaoVazio(idB, "ID do no B");

            if (idA == idB) {
                throw ExcecaoRede("Enlace nao pode conectar um no a si mesmo");
            }

            double latencia = 1.0;
            for (size_t i = 4; i < tokens.size(); ++i) {
                if (tokens[i].rfind("latencia=", 0) == 0) {
                    std::string valorStr = tokens[i].substr(9);
                    latencia = Validacao::parseDouble(valorStr);
                    if (latencia < 0.0) {
                        throw ExcecaoEntrada("Latencia nao pode ser negativa");
                    }
                }
            }

            No* noA = topologia_.getNo(idA);
            No* noB = topologia_.getNo(idB);
            if (!noA) throw ExcecaoRede("No '" + idA + "' nao existe");
            if (!noB) throw ExcecaoRede("No '" + idB + "' nao existe");

            auto enlace = std::make_unique<Enlace>(idE, noA, noB, 100.0, latencia);
            if (topologia_.adicionarEnlace(std::move(enlace))) {
                std::cout << "[OK] Enlace " << idE
                          << " adicionado: " << idA << " <-> " << idB << "\n";
            } else {
                throw ExcecaoRede("Nao foi possivel adicionar o enlace");
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
                std::cout << "[OK] Metricas exportadas para " << tokens[1] << "\n";
            } else {
                throw ExcecaoArquivo("Nao foi possivel exportar para " + tokens[1]);
            }

        } else if (cmd == "encerrar") {
            encerrar();

        } else if (cmd == "autores") {
            exibirAutores();

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
        std::cout << "[ERRO_FATAL] Excecao desconhecida (nao e std::exception)\n";
    }
}

// ── Ajuda ─────────────────────────────────────────────────────────────────────

/**
 * @brief Exibe a tabela de comandos disponiveis no terminal.
 *
 * Formata cada entrada com colunas alinhadas para facilitar a
 * leitura, separando o nome do comando, a sintaxe esperada e
 * uma descricao breve.
 */
void Simulador::exibirAjuda() const {
    imprimirSeparador();
    std::cout << "  Comandos disponiveis\n";
    imprimirSeparador();

    // Largura fixa para alinhamento das colunas
    const int W = 38;
    auto linha = [&](const std::string& cmd, const std::string& desc) {
        std::cout << "  " << std::left << std::setw(W) << cmd << desc << "\n";
    };

    std::cout << "  " << std::left << std::setw(W) << "COMANDO" << "DESCRICAO\n";
    imprimirSeparador();

    linha("add_no <tipo> <id>",               "Adiciona no (host/roteador/switch)");
    linha("add_enlace <id> <A> <B> lat=<ms>", "Conecta dois nos com latencia");
    linha("iniciar",                           "Valida topologia e inicia simulacao");
    linha("pausar",                            "Pausa a simulacao em execucao");
    linha("retomar",                           "Retoma a simulacao pausada");
    linha("proximo",                           "Avanca um passo de tempo");
    linha("injetar <origem> <destino>",        "Injeta pacote entre dois nos");
    linha("falha <idEnlace>",                  "Marca enlace como falho");
    linha("recuperar <idEnlace>",              "Restaura enlace falho");
    linha("metricas",                          "Exibe metricas no terminal");
    linha("exportar <arquivo.csv>",            "Exporta metricas para CSV");
    linha("encerrar",                          "Encerra a simulacao");
    linha("ajuda",                             "Exibe esta tabela");

    imprimirSeparador();
    std::cout << "\n";
}

// ── Autores ───────────────────────────────────────────────────────────────────

/**
 * @brief Exibe os autores do projeto com arte ASCII e abre o video de agradecimento.
 *
 * Imprime uma tela de creditos com carinhas em ASCII art para cada
 * integrante do grupo e tenta abrir o arquivo vid.mp4 localizado na
 * raiz do projeto usando o player padrao do sistema operacional.
 */
void Simulador::exibirAutores() const {
    imprimirSeparador(60);
    std::cout << "  Simulador de Rede | PDS2 TA2 2026/1 | Grupo 1\n";
    std::cout << "  Obrigado por usar nosso projeto!\n";
    imprimirSeparador(60);
    std::cout << "\n";

    std::cout
        << "   +----------+   Mateus Poelman Pinheiro\n"
        << "   |  O    O  |   Matricula: 2025019461\n"
        << "   |    --    |   \n"
        << "   |  \\____/ |   \n"
        << "   +----------+   \n"
        << "\n"
        << "   +----------+   Gabriel Figueiredo\n"
        << "   |  ^    ^  |   Matricula: 2025019569\n"
        << "   |    ..    |   \n"
        << "   |  (____)  |    \n"
        << "   +----------+   \n"
        << "\n"
        << "   +----------+   Lucas Fonseca\n"
        << "   |  *    *  |   Matricula: 2025019437\n"
        << "   |    ~~    |   \n"
        << "   |  /----\\ |   \n"
        << "   +----------+   \n"
        << "\n"
        << "   +----------+   Saul Gonzalez\n"
        << "   |  @    @  |   Matricula: 2025065358\n"
        << "   |    ==    |   \n"
        << "   |  <____>  |   \n"
        << "   +----------+   \n"
        << "\n";

    imprimirSeparador(60);
    std::cout << "  Abrindo video de agradecimento...\n";
    imprimirSeparador(60);
    std::cout << "\n";

    // Tenta abrir vid.mp4 na raiz do projeto com o player padrao do SO
#ifdef _WIN32
    std::system("start vid.mp4");
#elif __APPLE__
    std::system("open vid.mp4");
#else
    std::system("xdg-open vid.mp4 2>/dev/null || vlc vid.mp4 2>/dev/null || mpv vid.mp4 2>/dev/null");
#endif
}