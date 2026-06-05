#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "No.hpp"
#include "Enlace.hpp"
#include "Excecoes.hpp"

/**
 * @brief Armazena e gerencia o grafo completo da rede simulada.
 *
 * Mantem a colecao de nos e enlaces, fornece consultas de adjacencia
 * e cuida da serializacao/desserializacao em JSON.
 */
class TopologiaDeRede {
public:
    TopologiaDeRede() = default;

    /**
     * @brief Adiciona um no a topologia.
     * @param no Ponteiro para o no a ser inserido.
     * @return true se inserido com sucesso; false se o ID ja existe.
     */
    bool adicionarNo(std::unique_ptr<No> no);

    /**
     * @brief Remove um no e todos os enlaces a ele conectados.
     * @param id ID do no a ser removido.
     * @return true se removido com sucesso.
     */
    bool removerNo(const std::string& id);

    /**
     * @brief Adiciona um enlace entre dois nos existentes.
     * @param enlace Ponteiro para o enlace a ser inserido.
     * @return true se inserido; false se algum no nao existir.
     */
    bool adicionarEnlace(std::unique_ptr<Enlace> enlace);

    /**
     * @brief Remove um enlace pelo ID.
     * @param id ID do enlace a ser removido.
     * @return true se removido com sucesso.
     */
    bool removerEnlace(const std::string& id);

    /// @brief Retorna ponteiro para um no pelo ID (nullptr se nao existir).
    No* getNo(const std::string& id) const;

    /// @brief Retorna ponteiro para um enlace pelo ID (nullptr se nao existir).
    Enlace* getEnlace(const std::string& id) const;

    /// @brief Retorna todos os nos da topologia.
    std::vector<No*> getNos() const;

    /// @brief Retorna todos os enlaces da topologia.
    std::vector<Enlace*> getEnlaces() const;

    /// @brief Retorna os enlaces conectados a um no especifico.
    std::vector<Enlace*> getVizinhos(const std::string& idNo) const;

    /**
     * @brief Valida se todos os nos estao conectados.
     *
     * Exibe no terminal a lista de nos isolados, se houver.
     *
     * @return true se a topologia esta totalmente conectada.
     */
    bool validarConectividade() const;

    /**
     * @brief Serializa a topologia para um arquivo JSON.
     * @param caminho Caminho do arquivo de saida.
     * @return true se gravado com sucesso.
     */
    bool salvarJSON(const std::string& caminho) const;

    /**
     * @brief Carrega a topologia a partir de um arquivo JSON.
     * @param caminho Caminho do arquivo de entrada.
     * @return true se carregado com sucesso.
     */
    bool carregarJSON(const std::string& caminho);

private:
    std::unordered_map<std::string, std::unique_ptr<No>>     nos_;
    std::unordered_map<std::string, std::unique_ptr<Enlace>> enlaces_;
};
