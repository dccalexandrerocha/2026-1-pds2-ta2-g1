#include "TopologiaDeRede.hpp"
#include <iostream>
#include <queue>
#include <unordered_set>
#include <fstream>

bool TopologiaDeRede::adicionarNo(std::unique_ptr<No> no) {
    if (!no) return false;
    const std::string id = no->getId();
    if (nos_.count(id)) return false;   // ID ja existe
    nos_[id] = std::move(no);
    return true;
}

bool TopologiaDeRede::removerNo(const std::string& id) {
    if (!nos_.count(id)) return false;

    // Remove enlances conectados a este no
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

bool TopologiaDeRede::adicionarEnlace(std::unique_ptr<Enlace> enlace) {
    if (!enlace) return false;
    const std::string id = enlace->getId();
    if (enlaces_.count(id)) return false;

    // Verifica se os dois nos existem
    No* noA = enlace->getNoA();
    No* noB = enlace->getNoB();
    if (!noA || !noB) return false;
    if (!nos_.count(noA->getId())) return false;
    if (!nos_.count(noB->getId())) return false;

    // Registra o enlace nos dois nos
    noA->adicionarEnlace(enlace.get());
    noB->adicionarEnlace(enlace.get());

    enlaces_[id] = std::move(enlace);
    return true;
}

bool TopologiaDeRede::removerEnlace(const std::string& id) {
    if (!enlaces_.count(id)) return false;
    enlaces_.erase(id);
    return true;
}

No* TopologiaDeRede::getNo(const std::string& id) const {
    auto it = nos_.find(id);
    return (it != nos_.end()) ? it->second.get() : nullptr;
}

Enlace* TopologiaDeRede::getEnlace(const std::string& id) const {
    auto it = enlaces_.find(id);
    return (it != enlaces_.end()) ? it->second.get() : nullptr;
}

std::vector<No*> TopologiaDeRede::getNos() const {
    std::vector<No*> resultado;
    resultado.reserve(nos_.size());
    for (const auto& par : nos_) {
        resultado.push_back(par.second.get());
    }
    return resultado;
}

std::vector<Enlace*> TopologiaDeRede::getEnlaces() const {
    std::vector<Enlace*> resultado;
    resultado.reserve(enlaces_.size());
    for (const auto& par : enlaces_) {
        resultado.push_back(par.second.get());
    }
    return resultado;
}

std::vector<Enlace*> TopologiaDeRede::getVizinhos(const std::string& idNo) const {
    No* no = getNo(idNo);
    if (!no) return {};
    return no->getEnlaces();
}

bool TopologiaDeRede::validarConectividade() const {
    if (nos_.empty()) return true;

    // BFS a partir do primeiro no
    std::unordered_set<std::string> visitados;
    std::queue<std::string> fila;

    const std::string& inicio = nos_.begin()->first;
    fila.push(inicio);
    visitados.insert(inicio);

    while (!fila.empty()) {
        std::string atual = fila.front();
        fila.pop();

        for (Enlace* e : getVizinhos(atual)) {
            if (!e->isAtivo()) continue;
            std::string vizinho;
            if (e->getNoA()->getId() == atual) {
                vizinho = e->getNoB()->getId();
            } else {
                vizinho = e->getNoA()->getId();
            }
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

bool TopologiaDeRede::carregarJSON(const std::string& /*caminho*/) {
    // Implementacao simplificada: parsing manual de JSON e complexo
    // sem bibliotecas externas. Retorna false para indicar
    // que o carregamento via JSON nao esta disponivel nesta versao.
    std::cout << "[AVISO] carregarJSON nao implementado nesta versao.\n";
    return false;
}
