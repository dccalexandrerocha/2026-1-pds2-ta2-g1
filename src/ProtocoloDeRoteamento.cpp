/**
 * @file ProtocoloDeRoteamento.cpp
 * @brief Implementacao do algoritmo de Dijkstra para roteamento.
 *
 * RoteamentoDijkstra calcula o caminho de menor custo (medido
 * em latencia acumulada) entre dois nos da rede, ignorando enlaces
 * marcados como falhos. O caminho e reconstituido via mapa de
 * predecessores e armazenado na tabela interna para consulta.
 *
 * @author Grupo 1 - PDS2 TA2 2026/1
 */

#include "ProtocoloDeRoteamento.hpp"
#include "TopologiaDeRede.hpp"
#include "Enlace.hpp"
#include "No.hpp"
#include "Excecoes.hpp"
#include <iostream>
#include <queue>
#include <limits>
#include <algorithm>

/**
 * @brief Executa o algoritmo de Dijkstra na topologia atual.
 *
 * Inicializa as distancias de todos os nos como infinito, exceto
 * a origem (distancia 0). A fila de prioridade processa o no de
 * menor custo acumulado a cada iteracao, relaxando os vizinhos
 * alcancaveis por enlaces ativos.
 *
 * Ao final, reconstroi o caminho percorrendo o mapa de predecessores
 * do destino ate a origem e inverte a sequencia. O proximo salto a
 * partir da origem e armazenado na tabela interna tabelas_ para uso
 * futuro.
 *
 * Falhas sao capturadas internamente; em caso de erro, retorna
 * vetor vazio (sem caminho disponivel).
 */
std::vector<std::string> RoteamentoDijkstra::calcularCaminho(
    const TopologiaDeRede& topologia,
    const std::string& origem,
    const std::string& destino)
{
    try {
        if (origem.empty() || destino.empty()) {
            throw ExcecaoRede("Origem ou destino vazio em calcularCaminho()");
        }

        if (origem == destino) {
            return {origem};
        }

        No* noOrigem  = topologia.getNo(origem);
        No* noDestino = topologia.getNo(destino);

        if (!noOrigem) {
            throw ExcecaoRede("No de origem '" + origem + "' nao existe");
        }
        if (!noDestino) {
            throw ExcecaoRede("No de destino '" + destino + "' nao existe");
        }

        std::vector<No*> nos = topologia.getNos();
        if (nos.empty()) {
            throw ExcecaoRede("Topologia vazia (sem nos)");
        }

        // Inicializa distancias como infinito
        std::unordered_map<std::string, double> dist;
        std::unordered_map<std::string, std::string> anterior;

        for (No* no : nos) {
            if (!no) {
                std::cout << "[AVISO] No nulo encontrado em calcularCaminho()\n";
                continue;
            }
            dist[no->getId()] = std::numeric_limits<double>::infinity();
        }
        dist[origem] = 0.0;

        // Fila de prioridade: (custo, id_no)
        using Par = std::pair<double, std::string>;
        std::priority_queue<Par, std::vector<Par>, std::greater<Par>> pq;
        pq.push({0.0, origem});

        while (!pq.empty()) {
            auto [custoAtual, atual] = pq.top();
            pq.pop();

            if (dist.find(atual) == dist.end()) continue;
            if (custoAtual > dist[atual]) continue;
            if (atual == destino) break;

            for (Enlace* e : topologia.getVizinhos(atual)) {
                if (!e) {
                    std::cout << "[AVISO] Enlace nulo em calcularCaminho()\n";
                    continue;
                }
                if (!e->isAtivo()) continue;

                No* noA = e->getNoA();
                No* noB = e->getNoB();
                if (!noA || !noB) {
                    std::cout << "[AVISO] Enlace com no nulo em calcularCaminho()\n";
                    continue;
                }

                double latencia = e->getLatencia();
                if (latencia < 0.0) {
                    std::cout << "[AVISO] Enlace com latencia negativa\n";
                    continue;
                }

                std::string vizinho = (noA->getId() == atual)
                                      ? noB->getId()
                                      : noA->getId();

                if (dist.find(vizinho) == dist.end()) continue;

                double novoCusto = dist[atual] + latencia;
                if (novoCusto < dist[vizinho]) {
                    dist[vizinho] = novoCusto;
                    anterior[vizinho] = atual;
                    pq.push({novoCusto, vizinho});
                }
            }
        }

        // Sem caminho disponivel
        if (dist[destino] == std::numeric_limits<double>::infinity()) {
            return {};
        }

        // Reconstroi o caminho do destino ate a origem
        std::vector<std::string> caminho;
        std::string atual = destino;

        int tentativas = 0;
        const int maxTentativas = static_cast<int>(nos.size()) + 10;

        while (atual != origem && tentativas < maxTentativas) {
            caminho.push_back(atual);
            if (anterior.find(atual) == anterior.end()) {
                throw ExcecaoRede("Caminho quebrado durante reconstrucao");
            }
            atual = anterior[atual];
            tentativas++;
        }

        if (tentativas >= maxTentativas) {
            throw ExcecaoRede("Loop infinito detectado ao reconstruir caminho");
        }

        caminho.push_back(origem);
        std::reverse(caminho.begin(), caminho.end());

        // Armazena proximo salto na tabela de roteamento
        if (caminho.size() > 1) {
            tabelas_[origem][destino] = caminho[1];
        }

        return caminho;

    } catch (const ExcecaoRede& e) {
        std::cout << "[AVISO] " << e.what() << "\n";
        return {};
    } catch (const std::exception& e) {
        std::cout << "[AVISO] Excecao em Dijkstra: " << e.what() << "\n";
        return {};
    }
}

/**
 * @brief Imprime no terminal a tabela de roteamento de um no.
 *
 * Exibe todos os destinos conhecidos e o proximo salto associado.
 * Se o no nao tiver tabela ainda (nenhum caminho calculado), exibe
 * uma mensagem indicando que a tabela esta vazia.
 */
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