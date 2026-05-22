#include "EscalonadorDeEventos.hpp"
#include <iostream>

EscalonadorDeEventos::EscalonadorDeEventos()
    : tempoAtual_(0)
{}

void EscalonadorDeEventos::agendarEvento(int tempo,
                                         const std::string& descricao,
                                         std::function<void()> acao)
{
    Evento e;
    e.tempo     = tempo;
    e.descricao = descricao;
    e.acao      = acao;
    fila_.push(e);
}

bool EscalonadorDeEventos::processarProximo() {
    if (fila_.empty()) return false;

    Evento e = fila_.top();
    fila_.pop();

    // Avanca relogio se necessario
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

void EscalonadorDeEventos::avancarPasso() {
    tempoAtual_++;
    // Processa todos os eventos agendados para o tempo atual
    while (!fila_.empty() && fila_.top().tempo <= tempoAtual_) {
        processarProximo();
    }
}

int  EscalonadorDeEventos::getTempoAtual() const { return tempoAtual_; }
bool EscalonadorDeEventos::filaVazia()     const { return fila_.empty(); }

void EscalonadorDeEventos::limpar() {
    while (!fila_.empty()) fila_.pop();
}
