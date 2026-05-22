#include "No.hpp"
#include "Enlace.hpp"
#include <iostream>

// ── No (base) ────────────────────────────────────────────────────────────────

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

void No::adicionarEnlace(Enlace* enlace) {
    enlaces_.push_back(enlace);
}

const std::vector<Enlace*>& No::getEnlaces() const {
    return enlaces_;
}

int No::getPacotesRecebidos()  const { return pacotesRecebidos_; }
int No::getPacotesEnviados()   const { return pacotesEnviados_; }
int No::getPacotesDescartados() const { return pacotesDescartados_; }

// ── Host ─────────────────────────────────────────────────────────────────────

Host::Host(const std::string& id, const std::string& endereco)
    : No(id, TipoNo::HOST, endereco)
{}

void Host::receberPacote(const std::string& origem,
                         const std::string& destino,
                         int tempo)
{
    pacotesRecebidos_++;
    if (destino == id_) {
        std::cout << "[t=" << tempo << "] Pacote entregue em " << id_
                  << " (origem: " << origem << ").\n";
        pacotesEnviados_++;   // conta como "processado com sucesso"
    } else {
        // Host nao roteia; descarta
        std::cout << "[t=" << tempo << "] Host " << id_
                  << " descartou pacote destinado a " << destino << ".\n";
        pacotesDescartados_++;
    }
}

// ── Roteador ─────────────────────────────────────────────────────────────────

Roteador::Roteador(const std::string& id, const std::string& endereco)
    : No(id, TipoNo::ROTEADOR, endereco)
{}

void Roteador::receberPacote(const std::string& origem,
                             const std::string& destino,
                             int tempo)
{
    pacotesRecebidos_++;
    // O encaminhamento real e feito pelo Simulador via ProtocoloDeRoteamento.
    // Aqui apenas registramos o recebimento e avisamos no log.
    std::cout << "[t=" << tempo << "] Roteador " << id_
              << " recebeu pacote de " << origem
              << " para " << destino << ".\n";
    pacotesEnviados_++;
}

// ── Switch ───────────────────────────────────────────────────────────────────

Switch::Switch(const std::string& id, const std::string& endereco)
    : No(id, TipoNo::SWITCH, endereco)
{}

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
