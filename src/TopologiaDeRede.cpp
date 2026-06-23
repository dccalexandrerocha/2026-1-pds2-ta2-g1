/**
 * @file TopologiaDeRede.cpp
 * @brief Implementacao da classe TopologiaDeRede.
 *
 * Gerencia o grafo da rede (nos e enlaces) usando mapas associativos
 * para acesso O(1) por ID. Fornece operacoes de insercao, remocao,
 * consulta e validacao de conectividade via BFS, alem de
 * serializacao para JSON.
 *
 * @author Grupo 1 - PDS2 TA2 2026/1
 */

#include "TopologiaDeRede.hpp"
#include <iostream>
#include <queue>
#include <unordered_set>
#include <fstream>

/**
 * @brief Insere um no no mapa interno da topologia.
 *
 * Rejeita ponteiros nulos, IDs vazios e IDs duplicados.
 * A propriedade do objeto e transferida para o unique_ptr interno.
 *
 * @return false se o no for nulo, o ID vazio ou ja existir.
 */
bool TopologiaDeRede::adicionarNo(std::unique_ptr<No> no) {
    if (!no) return false;

    const std::string id = no->getId();
    if (id.empty()) return false;
    if (nos_.count(id)) return false;

    nos_[id] = std::move(no);
    return true;
}

/**
 * @brief Remove um no e todos os enlaces a ele conectados.
 *
 * Percorre todos os enlaces e elimina os que referenciam o no
 * removido antes de apagar o no em si, garantindo consistencia
 * do grafo.
 */
bool TopologiaDeRede::removerNo(const std::string& id) {
    if (!nos_.count(id)) return false;

    std::vector<std::string> paraRemover;
    for (const auto& par : enlaces_) {
        Enlace* e = par.second.get();
        if ((e->getNoA() && e->getNoA()->getId() == id) ||
            (e->getNoB() && e->getNoB()->getId() == id))
        {
            paraRemover.push_back(par.first);
        }
    }
    for (const auto& eid : paraRemover) {
        enlaces_.erase(eid);
    }

    nos_.erase(id);
    return true;
}

/**
 * @brief Insere um enlace no mapa e o registra nos dois nos extremos.
 *
 * Verifica que o enlace nao e nulo, que seu ID e unico, e que ambos
 * os nos referenciados existem na topologia antes de inserir.
 * Chama adicionarEnlace() nos dois nos para que eles mantenham a
 * lista de adjacencia atualizada.
 */
bool TopologiaDeRede::adicionarEnlace(std::unique_ptr<Enlace> enlace) {
    if (!enlace) return false;
    const std::string id = enlace->getId();
    if (enlaces_.count(id)) return false;

    No* noA = enlace->getNoA();
    No* noB = enlace->getNoB();
    if (!noA || !noB) return false;
    if (!nos_.count(noA->getId())) return false;
    if (!nos_.count(noB->getId())) return false;

    noA->adicionarEnlace(enlace.get());
    noB->adicionarEnlace(enlace.get());

    enlaces_[id] = std::move(enlace);
    return true;
}

/**
 * @brief Remove um enlace do mapa pelo ID.
 *
 * Nao remove a referencia do enlace nos nos extremos (a lista de
 * adjacencia dos nos pode ficar desatualizada apos esta operacao).
 */
bool TopologiaDeRede::removerEnlace(const std::string& id) {
    if (id.empty()) return false;

    auto it = enlaces_.find(id);
    if (it == enlaces_.end()) {
        return false;
    }

    enlaces_.erase(it);
    return true;
}

/// @brief Busca um no pelo ID; retorna nullptr se nao existir.
No* TopologiaDeRede::getNo(const std::string& id) const {
    auto it = nos_.find(id);
    return (it != nos_.end()) ? it->second.get() : nullptr;
}

/// @brief Busca um enlace pelo ID; retorna nullptr se nao existir.
Enlace* TopologiaDeRede::getEnlace(const std::string& id) const {
    auto it = enlaces_.find(id);
    return (it != enlaces_.end()) ? it->second.get() : nullptr;
}

/// @brief Retorna vetor com todos os ponteiros de nos cadastrados.
std::vector<No*> TopologiaDeRede::getNos() const {
    std::vector<No*> resultado;
    resultado.reserve(nos_.size());
    for (const auto& par : nos_) {
        resultado.push_back(par.second.get());
    }
    return resultado;
}

/// @brief Retorna vetor com todos os ponteiros de enlaces cadastrados.
std::vector<Enlace*> TopologiaDeRede::getEnlaces() const {
    std::vector<Enlace*> resultado;
    resultado.reserve(enlaces_.size());
    for (const auto& par : enlaces_) {
        resultado.push_back(par.second.get());
    }
    return resultado;
}

/**
 * @brief Retorna os enlaces conectados a um no (lista de adjacencia).
 *
 * Delega a No::getEnlaces(). Retorna vetor vazio se o no nao existir.
 */
std::vector<Enlace*> TopologiaDeRede::getVizinhos(const std::string& idNo) const {
    No* no = getNo(idNo);
    if (!no) return {};
    return no->getEnlaces();
}

/**
 * @brief Verifica se todos os nos sao alcancaveis entre si (BFS).
 *
 * Executa uma BFS a partir do primeiro no cadastrado, percorrendo
 * apenas enlaces ativos. Nos nao visitados ao final sao exibidos
 * como isolados. Uma topologia vazia e considerada valida.
 *
 * @return true se a topologia estiver totalmente conectada (ou vazia).
 */
bool TopologiaDeRede::validarConectividade() const {
    if (nos_.empty()) {
        std::cout << "[AVISO] Topologia vazia.\n";
        return true;
    }

    std::unordered_set<std::string> visitados;
    std::queue<std::string> fila;

    const std::string& inicio = nos_.begin()->first;
    fila.push(inicio);
    visitados.insert(inicio);

    while (!fila.empty()) {
        std::string atual = fila.front();
        fila.pop();

        for (Enlace* e : getVizinhos(atual)) {
            if (!e) {
                std::cout << "[AVISO] Enlace nulo encontrado em validarConectividade()\n";
                continue;
            }
            if (!e->isAtivo()) continue;

            No* noA = e->getNoA();
            No* noB = e->getNoB();
            if (!noA || !noB) {
                std::cout << "[AVISO] Enlace com no nulo encontrado\n";
                continue;
            }

            std::string vizinho = (noA->getId() == atual)
                                  ? noB->getId()
                                  : noA->getId();

            if (!visitados.count(vizinho)) {
                visitados.insert(vizinho);
                fila.push(vizinho);
            }
        }
    }

    bool conectado = true;
    for (const auto& par : nos_) {
        if (!visitados.count(par.first)) {
            std::cout << "[AVISO] No isolado: " << par.first << "\n";
            conectado = false;
        }
    }
    return conectado;
}

/**
 * @brief Serializa a topologia para um arquivo JSON minimo.
 *
 * Gera um JSON com arrays "nos" e "enlaces", cada um com os
 * atributos relevantes. Nao usa bibliotecas externas; a
 * formatacao e feita manualmente.
 *
 * @return false se o arquivo nao puder ser aberto.
 */
bool TopologiaDeRede::salvarJSON(const std::string& caminho) const {
    std::ofstream arquivo(caminho);
    if (!arquivo.is_open()) return false;

    arquivo << "{\n  \"nos\": [\n";
    bool primeiro = true;
    for (const auto& par : nos_) {
        if (!primeiro) arquivo << ",\n";
        primeiro = false;
        No* n = par.second.get();
        std::string tipo;
        switch (n->getTipo()) {
            case TipoNo::HOST:     tipo = "host";     break;
            case TipoNo::ROTEADOR: tipo = "roteador"; break;
            case TipoNo::SWITCH:   tipo = "switch";   break;
        }
        arquivo << "    {\"id\": \"" << n->getId()
                << "\", \"tipo\": \"" << tipo
                << "\", \"endereco\": \"" << n->getEndereco() << "\"}";
    }
    arquivo << "\n  ],\n  \"enlaces\": [\n";

    primeiro = true;
    for (const auto& par : enlaces_) {
        if (!primeiro) arquivo << ",\n";
        primeiro = false;
        Enlace* e = par.second.get();
        arquivo << "    {\"id\": \"" << e->getId()
                << "\", \"noA\": \"" << e->getNoA()->getId()
                << "\", \"noB\": \"" << e->getNoB()->getId()
                << "\", \"banda\": " << e->getBanda()
                << ", \"latencia\": " << e->getLatencia() << "}";
    }
    arquivo << "\n  ]\n}\n";

    return true;
}

/**
 * @brief Carregamento de topologia via JSON (nao implementado).
 *
 * O parsing manual de JSON sem bibliotecas externas e complexo
 * e esta fora do escopo deste projeto. O metodo retorna false
 * e exibe um aviso informativo.
 *
 * @return Sempre false nesta versao.
 */
bool TopologiaDeRede::carregarJSON(const std::string& /*caminho*/) {
    std::cout << "[AVISO] carregarJSON nao implementado nesta versao.\n";
    return false;
}