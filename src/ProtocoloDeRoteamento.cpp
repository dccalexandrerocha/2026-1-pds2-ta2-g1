#include "ProtocoloDeRoteamento.hpp"
#include "TopologiaDeRede.hpp"
#include "Enlace.hpp"
#include "No.hpp"
#include "Excecoes.hpp"
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
    try {
        // Validações defensivas
        if (origem.empty() || destino.empty()) {
            throw ExcecaoRede("Origem ou destino vazio em calcularCaminho()");
        }

        if (origem == destino) {
            // Caso trivial: origem e destino sao o mesmo no
            return {origem};
        }

        // Garante que os nós existem
        No* noOrigem = topologia.getNo(origem);
        No* noDestino = topologia.getNo(destino);
        
        if (!noOrigem) {
            throw ExcecaoRede("Nó de origem '" + origem + "' não existe");
        }
        if (!noDestino) {
            throw ExcecaoRede("Nó de destino '" + destino + "' não existe");
        }

        std::vector<No*> nos = topologia.getNos();
        if (nos.empty()) {
            throw ExcecaoRede("Topologia vazia (sem nós)");
        }

        // dist: menor custo acumulado até cada nó
        std::unordered_map<std::string, double> dist;
        // anterior: de onde viemos para chegar em cada nó
        std::unordered_map<std::string, std::string> anterior;

        for (No* no : nos) {
            if (!no) {
                std::cout << "[AVISO] Nó nulo encontrado em calcularCaminho()\n";
                continue;
            }
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

            // Validação: nó não deve estar no mapa de distâncias
            if (dist.find(atual) == dist.end()) {
                continue;  // Nó foi deletado ou é inválido
            }

            if (custoAtual > dist[atual]) continue;  // entrada obsoleta
            if (atual == destino) break;

            std::vector<Enlace*> vizinhos = topologia.getVizinhos(atual);
            for (Enlace* e : vizinhos) {
                // Validação defensiva: enlace nulo
                if (!e) {
                    std::cout << "[AVISO] Enlace nulo em calcularCaminho()\n";
                    continue;
                }

                if (!e->isAtivo()) continue;

                // Validação defensiva: nós nulos
                No* noA = e->getNoA();
                No* noB = e->getNoB();
                if (!noA || !noB) {
                    std::cout << "[AVISO] Enlace com nó nulo em calcularCaminho()\n";
                    continue;
                }

                // Validação: latência não negativa
                double latencia = e->getLatencia();
                if (latencia < 0.0) {
                    std::cout << "[AVISO] Enlace com latência negativa\n";
                    continue;
                }

                std::string vizinho;
                if (noA->getId() == atual) {
                    vizinho = noB->getId();
                } else {
                    vizinho = noA->getId();
                }

                // Verificar se vizinho está em dist
                if (dist.find(vizinho) == dist.end()) {
                    continue;  // Nó vizinho não existe
                }

                double novoCusto = dist[atual] + latencia;
                if (novoCusto < dist[vizinho]) {
                    dist[vizinho] = novoCusto;
                    anterior[vizinho] = atual;
                    pq.push({novoCusto, vizinho});
                }
            }
        }

        // Reconstrói o caminho do destino até a origem
        if (dist[destino] == std::numeric_limits<double>::infinity()) {
            return {};   // sem caminho
        }

        std::vector<std::string> caminho;
        std::string atual = destino;
        
        // Proteção contra loop infinito
        int tentativas = 0;
        const int maxTentativas = nos.size() + 10;
        
        while (atual != origem && tentativas < maxTentativas) {
            caminho.push_back(atual);
            
            if (anterior.find(atual) == anterior.end()) {
                // Caminho quebrado - não deveria acontecer
                throw ExcecaoRede("Caminho quebrado durante reconstrução");
            }
            
            atual = anterior[atual];
            tentativas++;
        }

        if (tentativas >= maxTentativas) {
            throw ExcecaoRede("Loop infinito detectado ao reconstruir caminho");
        }

        caminho.push_back(origem);
        std::reverse(caminho.begin(), caminho.end());

        // Armazena na tabela interna
        if (caminho.size() > 1) {
            tabelas_[origem][destino] = caminho[1];
        }

        return caminho;

    } catch (const ExcecaoRede& e) {
        std::cout << "[AVISO] " << e.what() << "\n";
        return {};
    } catch (const std::exception& e) {
        std::cout << "[AVISO] Exceção em Dijkstra: " << e.what() << "\n";
        return {};
    }
}

void RoteamentoDijkstra::exibirTabela(const std::string& idNo) const {
    if (idNo.empty()) {
        std::cout << "[AVISO] ID do no vazio ao exibir tabela de roteamento.\n";
        return;
    }

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