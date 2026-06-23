/**
 * @file EscalonadorDeEventos.cpp
 * @brief Implementacao do escalonador de eventos da simulacao.
 *
 * Mantém uma fila de prioridade (min-heap por tempo) de eventos
 * futuros e avanca o relogio global a cada passo, despachando
 * todos os eventos cujo instante seja <= ao tempo atual.
 *
 * @author Grupo 1 - PDS2 TA2 2026/1
 */

#include "EscalonadorDeEventos.hpp"
#include <iostream>

/**
 * @brief Constroi o escalonador com relogio zerado e fila vazia.
 */
EscalonadorDeEventos::EscalonadorDeEventos()
    : tempoAtual_(0)
{}

/**
 * @brief Insere um evento na fila de prioridade.
 *
 * Eventos agendados para instantes anteriores ao tempo atual
 * sao ignorados com aviso, pois a simulacao e causal e nao
 * permite viagem ao passado.
 */
void EscalonadorDeEventos::agendarEvento(int tempo,
                                         const std::string& descricao,
                                         std::function<void()> acao)
{
    if (tempo < tempoAtual_) {
        std::cout << "[AVISO] Evento no passado ignorado: t=" << tempo << "\n";
        return;
    }

    Evento e;
    e.tempo = tempo;
    e.descricao = descricao;
    e.acao = acao;
    fila_.push(e);
}

/**
 * @brief Remove e executa o evento de menor tempo da fila.
 *
 * Avanca o relogio se o evento for futuro em relacao ao tempo
 * atual. Exibe a descricao do evento no log antes de executar
 * a acao associada.
 *
 * @return false se a fila estava vazia; true apos execucao.
 */
bool EscalonadorDeEventos::processarProximo() {
    if (fila_.empty()) return false;

    Evento e = fila_.top();
    fila_.pop();

    if (e.tempo > tempoAtual_) {
        tempoAtual_ = e.tempo;
    }

    if (!e.descricao.empty()) {
        std::cout << "[t=" << tempoAtual_ << "] " << e.descricao << "\n";
    }

    if (e.acao) {
        e.acao();
    }

    return true;
}

/**
 * @brief Incrementa o relogio e despacha todos os eventos do instante atual.
 *
 * Processa em ordem crescente de tempo todos os eventos cujo
 * instante agendado seja menor ou igual ao novo tempo atual.
 */
void EscalonadorDeEventos::avancarPasso() {
    tempoAtual_++;
    while (!fila_.empty() && fila_.top().tempo <= tempoAtual_) {
        processarProximo();
    }
}

int  EscalonadorDeEventos::getTempoAtual() const { return tempoAtual_; }
bool EscalonadorDeEventos::filaVazia()     const { return fila_.empty(); }

/**
 * @brief Esvazia a fila de eventos pendentes.
 *
 * Util ao encerrar a simulacao para liberar recursos.
 */
void EscalonadorDeEventos::limpar() {
    while (!fila_.empty()) fila_.pop();
}