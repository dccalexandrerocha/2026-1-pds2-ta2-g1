#include "ProtocoloDeRoteamento.hpp"
#include "TopologiaDeRede.hpp"
#include "Enlace.hpp"
#include "No.hpp"
#include <iostream>
#include <queue>
#include <limits>
#include <algorithm>

// ── RoteamentoDijkstra ────────────────────────────────────────────────────────

std::vector<std::string> RoteamentoDijkstra::calcularCaminho(
    const TopologiaDeRede& topologia,
    const std::string& origem,
    const std::string& destino)
{
    // Garante que os nos existem
    if (!topologia.getNo(origem) || !topologia.getNo(destino)) {
        return {};
    }

    // dist: menor custo acumulado ate cada no
    std::unordered_map<std::string, double> dist;
    // anterior: de onde viemos para chegar em cada no
    std::unordered_map<std::string, std::string> anterior;

    for (No* no : topologia.getNos()) {
        dist[no->getId()] = std::numeric_limits<double>::infinity();
    }
    dist[origem] = 0.0;

    // Fila de prioridade: (custo, id)
    using Par = std::pair<double, std::string>;
    std::priority_queue<Par, std::vector<Par>, std::greater<Par>> pq;
    pq.push({0.0, origem});

    while (!pq.empty()) {
        auto [custoAtual, atual] = pq.top();
        pq.pop();

        if (custoAtual > dist[atual]) continue;  // entrada obsoleta
        if (atual == destino) break;

        for (Enlace* e : topologia.getVizinhos(atual)) {
            if (!e->isAtivo()) continue;

            // Determina o vizinho do lado oposto
            std::string vizinho;
            if (e->getNoA()->getId() == atual) {
                vizinho = e->getNoB()->getId();
            } else {
                vizinho = e->getNoA()->getId();
            }

            double novoCusto = dist[atual] + e->getLatencia();
            if (novoCusto < dist[vizinho]) {
                dist[vizinho] = novoCusto;
                anterior[vizinho] = atual;
                pq.push({novoCusto, vizinho});
            }
        }
    }

    // Reconstroi o caminho do destino ate a origem
    if (dist[destino] == std::numeric_limits<double>::infinity()) {
        return {};   // sem caminho
    }

    std::vector<std::string> caminho;
    std::string atual = destino;
    while (atual != origem) {
        caminho.push_back(atual);
        atual = anterior[atual];
    }
    caminho.push_back(origem);
    std::reverse(caminho.begin(), caminho.end());

    // Armazena na tabela interna
    tabelas_[origem][destino] = caminho.size() > 1 ? caminho[1] : destino;

    return caminho;
}

void RoteamentoDijkstra::exibirTabela(const std::string& idNo) const {
    auto it = tabelas_.find(idNo);
    if (it == tabelas_.end()) {
        std::cout << "Tabela de roteamento de " << idNo << ": (vazia)\n";
        return;
    }
    std::cout << "Tabela de roteamento de " << idNo << ":\n";
    for (const auto& par : it->second) {
        std::cout << "  destino: " << par.first
                  << "  ->  proximo salto: " << par.second << "\n";
    }
}
