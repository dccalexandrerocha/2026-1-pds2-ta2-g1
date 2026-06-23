/**
 * @file ColetorDeMetricas.cpp
 * @brief Implementacao do coletor e exportador de metricas.
 *
 * Acumula contadores de enlaces e nos ao longo da simulacao
 * e os disponibiliza para exibicao no terminal ou exportacao
 * em formato CSV.
 *
 * @author Grupo 1 - PDS2 TA2 2026/1
 */

#include "ColetorDeMetricas.hpp"
#include "Excecoes.hpp"
#include "Validacao.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>

/**
 * @brief Acumula os contadores de um enlace.
 *
 * Valida os parametros antes de somar aos acumuladores. Atualiza
 * o pico de utilizacao se o valor atual for maior que o maximo
 * registrado ate entao.
 */
void ColetorDeMetricas::registrarEnlace(const std::string& idEnlace,
                                        int transmitidos,
                                        int descartados,
                                        double utilizacao)
{
    Validacao::validarNaoVazio(idEnlace, "id do enlace");
    Validacao::validarIntervalo(transmitidos, 0.0, 1.0e9, "pacotes transmitidos");
    Validacao::validarIntervalo(descartados, 0.0, 1.0e9, "pacotes descartados");
    Validacao::validarIntervalo(utilizacao, 0.0, 1.0, "utilizacao do enlace");

    DadosEnlace& d = enlaces_[idEnlace];
    d.transmitidos += transmitidos;
    d.descartados += descartados;
    d.utilizacaoSoma += utilizacao;
    d.numRegistros++;

    if (utilizacao > d.utilizacaoMax) {
        d.utilizacaoMax = utilizacao;
    }
}

/**
 * @brief Acumula os contadores de um no.
 *
 * Nao exige validacao extra alem do que e garantido pelo chamador;
 * simplesmente soma aos acumuladores internos.
 */
void ColetorDeMetricas::registrarNo(const std::string& idNo,
                                    int recebidos,
                                    int enviados,
                                    int descartados)
{
    DadosNo& d = nos_[idNo];
    d.recebidos   += recebidos;
    d.enviados    += enviados;
    d.descartados += descartados;
}

/**
 * @brief Imprime o resumo de metricas no terminal.
 *
 * Para cada enlace exibe transmitidos, descartados, utilizacao
 * media e utilizacao maxima. Para cada no exibe recebidos,
 * enviados e descartados. Por fim, exibe os totais globais.
 */
void ColetorDeMetricas::exibirResumo() const {
    std::cout << "\n=== Metricas por Enlace ===\n";
    for (const auto& par : enlaces_) {
        const std::string& id = par.first;
        const DadosEnlace& d  = par.second;
        double media = (d.numRegistros > 0)
                       ? (d.utilizacaoSoma / d.numRegistros)
                       : 0.0;
        std::cout << id
                  << " | transmitidos: " << d.transmitidos
                  << " | descartados: "  << d.descartados
                  << " | util.media: "   << media
                  << " | util.max: "     << d.utilizacaoMax
                  << "\n";
    }

    std::cout << "\n=== Metricas por No ===\n";
    for (const auto& par : nos_) {
        const std::string& id = par.first;
        const DadosNo& d      = par.second;
        std::cout << id
                  << " | recebidos: "  << d.recebidos
                  << " | enviados: "   << d.enviados
                  << " | descartados: "<< d.descartados
                  << "\n";
    }

    std::cout << "\n=== Totais Globais ===\n"
              << "Pacotes entregues: " << getTotalEntregues() << "\n"
              << "Pacotes perdidos:  " << getTotalPerdidos()  << "\n";
}

/**
 * @brief Grava as metricas em um arquivo CSV.
 *
 * Gera duas secoes no CSV: uma para enlaces (com utilizacao media
 * e maxima) e outra para nos. Propaga ExcecaoArquivo em caso de
 * falha de I/O para que o chamador possa exibir mensagem adequada.
 *
 * @throws ExcecaoArquivo se o arquivo nao puder ser aberto ou gravado.
 */
bool ColetorDeMetricas::exportarCSV(const std::string& caminho) const {
    try {
        if (caminho.empty()) {
            throw ExcecaoArquivo("Caminho do arquivo nao pode estar vazio");
        }

        std::ofstream arquivo(caminho);

        if (!arquivo.is_open()) {
            throw ExcecaoArquivo("Nao foi possivel abrir arquivo '" + caminho +
                               "' para escrita (verifique permissoes)");
        }

        arquivo << "tipo,id,transmitidos,descartados,util_media,util_max\n";
        if (!arquivo.good()) {
            throw ExcecaoArquivo("Erro ao escrever cabecalho no arquivo");
        }

        for (const auto& par : enlaces_) {
            const DadosEnlace& d = par.second;
            double media = (d.numRegistros > 0)
                           ? (d.utilizacaoSoma / d.numRegistros)
                           : 0.0;
            arquivo << "enlace,"
                    << par.first      << ","
                    << d.transmitidos << ","
                    << d.descartados  << ","
                    << media          << ","
                    << d.utilizacaoMax<< "\n";

            if (!arquivo.good()) {
                throw ExcecaoArquivo("Erro ao escrever dados de enlace no arquivo");
            }
        }

        arquivo << "tipo,id,recebidos,enviados,descartados,,\n";
        if (!arquivo.good()) {
            throw ExcecaoArquivo("Erro ao escrever cabecalho de nos no arquivo");
        }

        for (const auto& par : nos_) {
            const DadosNo& d = par.second;
            arquivo << "no,"
                    << par.first  << ","
                    << d.recebidos<< ","
                    << d.enviados << ","
                    << d.descartados << ",,\n";

            if (!arquivo.good()) {
                throw ExcecaoArquivo("Erro ao escrever dados de no no arquivo");
            }
        }

        arquivo.close();
        if (!arquivo) {
            throw ExcecaoArquivo("Erro ao fechar arquivo de saida");
        }

        return true;

    } catch (const ExcecaoArquivo&) {
        throw;
    } catch (const std::exception& e) {
        throw ExcecaoArquivo(std::string("Excecao ao exportar CSV: ") + e.what());
    }
}

/**
 * @brief Soma os pacotes enviados (processados) de todos os nos.
 *
 * Representa o total de pacotes que chegaram ao seu destino final.
 */
int ColetorDeMetricas::getTotalEntregues() const {
    int total = 0;
    for (const auto& par : nos_) {
        total += par.second.enviados;
    }
    return total;
}

/**
 * @brief Soma os pacotes descartados em enlaces e nos.
 *
 * Inclui descartes por falha de enlace e por nos que nao
 * eram o destino correto do pacote.
 */
int ColetorDeMetricas::getTotalPerdidos() const {
    int total = 0;
    for (const auto& par : enlaces_) {
        total += par.second.descartados;
    }
    for (const auto& par : nos_) {
        total += par.second.descartados;
    }
    return total;
}

/**
 * @brief Zera todos os contadores acumulados.
 *
 * Chamado antes de cada coleta para evitar acumulo duplo entre
 * chamadas sucessivas ao metodo metricas do Simulador.
 */
void ColetorDeMetricas::reset() {
    if (enlaces_.empty() && nos_.empty()) {
        return;
    }
    enlaces_.clear();
    nos_.clear();
}