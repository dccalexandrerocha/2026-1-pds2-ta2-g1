#include "ColetorDeMetricas.hpp"
#include "Excecoes.hpp"
#include "Validacao.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>

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

bool ColetorDeMetricas::exportarCSV(const std::string& caminho) const {
    try {
        if (caminho.empty()) {
            throw ExcecaoArquivo("Caminho do arquivo não pode estar vazio");
        }

        std::ofstream arquivo(caminho);
        
        // Validação defensiva: verificar se arquivo foi aberto
        if (!arquivo.is_open()) {
            throw ExcecaoArquivo("Não foi possível abrir arquivo '" + caminho + 
                               "' para escrita (verifique permissões)");
        }

        // Verificar se a escrita foi bem-sucedida
        arquivo << "tipo,id,transmitidos,descartados,util_media,util_max\n";
        if (!arquivo.good()) {
            throw ExcecaoArquivo("Erro ao escrever cabeçalho no arquivo");
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
            throw ExcecaoArquivo("Erro ao escrever cabeçalho de nós no arquivo");
        }

        for (const auto& par : nos_) {
            const DadosNo& d = par.second;
            arquivo << "no,"
                    << par.first  << ","
                    << d.recebidos<< ","
                    << d.enviados << ","
                    << d.descartados << ",,\n";
            
            if (!arquivo.good()) {
                throw ExcecaoArquivo("Erro ao escrever dados de nó no arquivo");
            }
        }

        arquivo.close();
        if (!arquivo) {
            throw ExcecaoArquivo("Erro ao fechar arquivo de saída");
        }

        return true;

    } catch (const ExcecaoArquivo&) {
        throw;  // Re-lança para o chamador
    } catch (const std::exception& e) {
        throw ExcecaoArquivo(std::string("Exceção ao exportar CSV: ") + e.what());
    }
}

int ColetorDeMetricas::getTotalEntregues() const {
    int total = 0;
    for (const auto& par : nos_) {
        total += par.second.enviados;
    }
    return total;
}

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

void ColetorDeMetricas::reset() {
    enlaces_.clear();
    nos_.clear();
}
