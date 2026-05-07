#pragma once

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Acumula e calcula metricas de desempenho da simulacao.
 *
 * A cada passo de tempo, recebe contadores de nos e enlaces.
 * Ao fim da simulacao, exibe um resumo no terminal e exporta
 * os dados em formato CSV.
 */
class ColetorDeMetricas {
public:
    ColetorDeMetricas() = default;

    /**
     * @brief Registra os contadores de um enlace no passo atual.
     * @param idEnlace          ID do enlace.
     * @param transmitidos      Pacotes transmitidos neste passo.
     * @param descartados       Pacotes descartados neste passo.
     * @param utilizacao        Taxa de utilizacao atual (0.0 a 1.0).
     */
    void registrarEnlace(const std::string& idEnlace,
                         int transmitidos,
                         int descartados,
                         double utilizacao);

    /**
     * @brief Registra os contadores de um no no passo atual.
     * @param idNo         ID do no.
     * @param recebidos    Pacotes recebidos neste passo.
     * @param enviados     Pacotes enviados neste passo.
     * @param descartados  Pacotes descartados neste passo.
     */
    void registrarNo(const std::string& idNo,
                     int recebidos,
                     int enviados,
                     int descartados);

    /**
     * @brief Exibe no terminal as metricas por enlace e os totais globais.
     *
     * Mostra: pacotes transmitidos, descartados, utilizacao media
     * e utilizacao maxima por enlace; totais globais de pacotes
     * injetados, entregues e perdidos.
     */
    void exibirResumo() const;

    /**
     * @brief Exporta as metricas coletadas para um arquivo CSV.
     *
     * Gera uma linha por enlace e uma linha por no, com todos os
     * contadores acumulados.
     *
     * @param caminho Caminho do arquivo CSV de saida.
     * @return true se o arquivo foi gravado com sucesso.
     */
    bool exportarCSV(const std::string& caminho) const;

    /// @brief Retorna o total global de pacotes entregues.
    int getTotalEntregues() const;

    /// @brief Retorna o total global de pacotes perdidos.
    int getTotalPerdidos() const;

    /// @brief Zera todos os contadores acumulados.
    void reset();

private:
    struct DadosEnlace {
        int    transmitidos   = 0;
        int    descartados    = 0;
        double utilizacaoMax  = 0.0;
        double utilizacaoSoma = 0.0;
        int    numRegistros   = 0;
    };

    struct DadosNo {
        int recebidos   = 0;
        int enviados    = 0;
        int descartados = 0;
    };

    std::unordered_map<std::string, DadosEnlace> enlaces_;
    std::unordered_map<std::string, DadosNo>     nos_;
};
