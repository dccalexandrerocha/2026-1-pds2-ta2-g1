#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class TopologiaDeRede;

/**
 * @brief Classe base abstrata para algoritmos de roteamento.
 *
 * Define a interface comum que qualquer protocolo de roteamento
 * deve implementar. A subclasse concreta desta versao do projeto
 * e RoteamentoDijkstra.
 */
class ProtocoloDeRoteamento {
public:
    virtual ~ProtocoloDeRoteamento() = default;

    /**
     * @brief Calcula o caminho otimo entre dois nos.
     *
     * Metodo abstrato implementado por cada subclasse.
     *
     * @param topologia Referencia a topologia atual da rede.
     * @param origem    ID do no de origem.
     * @param destino   ID do no de destino.
     * @return Sequencia ordenada de IDs de nos do caminho encontrado.
     *         Retorna vetor vazio se nao houver caminho disponivel.
     */
    virtual std::vector<std::string> calcularCaminho(
        const TopologiaDeRede& topologia,
        const std::string& origem,
        const std::string& destino) = 0;

    /**
     * @brief Exibe a tabela de roteamento de um no no terminal.
     * @param idNo ID do no cuja tabela sera exibida.
     */
    virtual void exibirTabela(const std::string& idNo) const = 0;

protected:
    /// @brief Tabelas de roteamento: no -> (destino -> proximo salto)
    std::unordered_map<std::string,
        std::unordered_map<std::string, std::string>> tabelas_;
};


/**
 * @brief Implementacao do roteamento pelo algoritmo de Dijkstra.
 *
 * Calcula o caminho de menor custo entre dois nos usando a latencia
 * dos enlaces ativos como metrica. Enlaces falhos sao ignorados.
 */
class RoteamentoDijkstra : public ProtocoloDeRoteamento {
public:
    /**
     * @brief Calcula o menor caminho entre origem e destino via Dijkstra.
     *
     * @param topologia Referencia a topologia atual da rede.
     * @param origem    ID do no de origem.
     * @param destino   ID do no de destino.
     * @return Sequencia ordenada de IDs de nos do caminho encontrado.
     *         Retorna vetor vazio se nao houver caminho disponivel.
     */
    std::vector<std::string> calcularCaminho(
        const TopologiaDeRede& topologia,
        const std::string& origem,
        const std::string& destino) override;

    /**
     * @brief Exibe no terminal a tabela de roteamento de um no.
     * @param idNo ID do no cuja tabela sera exibida.
     */
    void exibirTabela(const std::string& idNo) const override;
};
