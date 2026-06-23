/**
 * @file No.cpp
 * @brief Implementacao das classes No, Host, Roteador e Switch.
 *
 * Define o comportamento concreto de cada tipo de no da rede ao
 * receber pacotes. A classe base No gerencia os contadores e a
 * lista de enlaces; as subclasses especializam o metodo
 * receberPacote().
 *
 * @author Grupo 1 - PDS2 TA2 2026/1
 */

#include "No.hpp"
#include "Enlace.hpp"
#include "Validacao.hpp"
#include <iostream>

// ── No (base) ────────────────────────────────────────────────────────────────

/**
 * @brief Constroi um No com ID, tipo e endereco.
 *
 * Inicializa todos os contadores de pacotes com zero e o estado
 * do no como ativo.
 */
No::No(const std::string& id, TipoNo tipo, const std::string& endereco)
    : id_(id),
      tipo_(tipo),
      endereco_(endereco),
      ativo_(true),
      pacotesRecebidos_(0),
      pacotesEnviados_(0),
      pacotesDescartados_(0)
{}

const std::string& No::getId()       const { return id_; }
TipoNo             No::getTipo()     const { return tipo_; }
const std::string& No::getEndereco() const { return endereco_; }
bool               No::isAtivo()     const { return ativo_; }

/**
 * @brief Adiciona um enlace a lista de conexoes deste no.
 *
 * Valida que o ponteiro nao e nulo antes de inserir.
 */
void No::adicionarEnlace(Enlace* enlace) {
    Validacao::validarNaoNulo(enlace, "enlace");
    enlaces_.push_back(enlace);
}

const std::vector<Enlace*>& No::getEnlaces() const {
    return enlaces_;
}

int No::getPacotesRecebidos()   const { return pacotesRecebidos_; }
int No::getPacotesEnviados()    const { return pacotesEnviados_; }
int No::getPacotesDescartados() const { return pacotesDescartados_; }

// ── Host ─────────────────────────────────────────────────────────────────────

/**
 * @brief Constroi um Host com ID e endereco.
 *
 * Delega ao construtor de No com o tipo TipoNo::HOST.
 */
Host::Host(const std::string& id, const std::string& endereco)
    : No(id, TipoNo::HOST, endereco)
{}

/**
 * @brief Recebe um pacote e verifica se este host e o destino.
 *
 * Se o ID deste no corresponde ao destino, o pacote e considerado
 * entregue e o contador de enviados e incrementado (representando
 * o processamento bem-sucedido). Caso contrario, o Host nao
 * encaminha o pacote e o descarta, pois Hosts nao roteiam.
 */
void Host::receberPacote(const std::string& origem,
                         const std::string& destino,
                         int tempo)
{
    pacotesRecebidos_++;
    if (destino == id_) {
        std::cout << "[t=" << tempo << "] Pacote entregue em " << id_
                  << " (origem: " << origem << ").\n";
        pacotesEnviados_++;
    } else {
        std::cout << "[t=" << tempo << "] Host " << id_
                  << " descartou pacote destinado a " << destino << ".\n";
        pacotesDescartados_++;
    }
}

// ── Roteador ─────────────────────────────────────────────────────────────────

/**
 * @brief Constroi um Roteador com ID e endereco.
 *
 * Delega ao construtor de No com o tipo TipoNo::ROTEADOR.
 */
Roteador::Roteador(const std::string& id, const std::string& endereco)
    : No(id, TipoNo::ROTEADOR, endereco)
{}

/**
 * @brief Registra o recebimento do pacote e sinaliza no log.
 *
 * O encaminhamento real (proximo salto) e orquestrado pelo Simulador
 * via ProtocoloDeRoteamento antes de chamar este metodo; aqui apenas
 * incrementamos os contadores e emitimos a mensagem de log.
 */
void Roteador::receberPacote(const std::string& origem,
                             const std::string& destino,
                             int tempo)
{
    pacotesRecebidos_++;
    std::cout << "[t=" << tempo << "] Roteador " << id_
              << " recebeu pacote de " << origem
              << " para " << destino << ".\n";
    pacotesEnviados_++;
}

// ── Switch ───────────────────────────────────────────────────────────────────

/**
 * @brief Constroi um Switch com ID e endereco.
 *
 * Delega ao construtor de No com o tipo TipoNo::SWITCH.
 */
Switch::Switch(const std::string& id, const std::string& endereco)
    : No(id, TipoNo::SWITCH, endereco)
{}

/**
 * @brief Registra o recebimento do pacote e sinaliza no log.
 *
 * Nesta versao do simulador, o Switch atua como ponto de passagem
 * e registra o transito do pacote sem implementar logica de
 * encaminhamento por tabela MAC (simplificacao didatica).
 */
void Switch::receberPacote(const std::string& origem,
                           const std::string& destino,
                           int tempo)
{
    pacotesRecebidos_++;
    std::cout << "[t=" << tempo << "] Switch " << id_
              << " recebeu pacote de " << origem
              << " para " << destino << ".\n";
    pacotesEnviados_++;
}