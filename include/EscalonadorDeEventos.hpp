#pragma once

#include <string>
#include <queue>
#include <functional>

/**
 * @brief Representa um evento agendado na simulacao.
 */
struct Evento {
    int                   tempo;    ///< Instante de disparo do evento.
    std::string           descricao; ///< Descricao textual para o log.
    std::function<void()> acao;     ///< Funcao a ser executada no disparo.

    /// @brief Operador de comparacao para a fila de prioridade (menor tempo primeiro).
    bool operator>(const Evento& outro) const {
        return tempo > outro.tempo;
    }
};

/**
 * @brief Gerencia a fila de eventos ordenada por tempo de ocorrencia.
 *
 * O EscalonadorDeEventos mantem o relogio global da simulacao e
 * despacha eventos para os componentes responsaveis a cada passo.
 */
class EscalonadorDeEventos {
public:
    EscalonadorDeEventos();

    /**
     * @brief Insere um novo evento na fila.
     * @param tempo     Instante de tempo em que o evento deve ocorrer.
     * @param descricao Descricao textual do evento.
     * @param acao      Funcao a ser executada quando o evento for despachado.
     */
    void agendarEvento(int tempo,
                       const std::string& descricao,
                       std::function<void()> acao);

    /**
     * @brief Processa e despacha o proximo evento da fila.
     *
     * Avanca o relogio global para o instante do evento e executa
     * sua acao associada.
     *
     * @return true se havia evento para processar; false se a fila esta vazia.
     */
    bool processarProximo();

    /**
     * @brief Avanca o relogio em um passo de tempo e processa os
     *        eventos cujo instante seja igual ao tempo atual.
     */
    void avancarPasso();

    /// @brief Retorna o instante de tempo global atual.
    int getTempoAtual() const;

    /// @brief Retorna true se nao ha eventos pendentes na fila.
    bool filaVazia() const;

    /// @brief Remove todos os eventos pendentes da fila.
    void limpar();

private:
    int tempoAtual_;

    std::priority_queue<Evento,
                        std::vector<Evento>,
                        std::greater<Evento>> fila_;
};
