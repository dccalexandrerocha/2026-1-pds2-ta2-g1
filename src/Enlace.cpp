#include "Enlace.hpp"
#include "No.hpp"
#include "ColetorDeMetricas.hpp"
#include <iostream>

Enlace::Enlace(const std::string& id,
               No* noA,
               No* noB,
               double banda,
               double latencia)
    : id_(id),
      noA_(noA),
      noB_(noB),
      banda_(banda),
      latencia_(latencia),
      ativo_(true),
      pacotesTransmitidos_(0),
      pacotesDescartados_(0),
      utilizacaoAcumulada_(0.0),
      numPassos_(0)
{}

const std::string& Enlace::getId()       const { return id_; }
No*                Enlace::getNoA()      const { return noA_; }
No*                Enlace::getNoB()      const { return noB_; }
double             Enlace::getBanda()    const { return banda_; }
double             Enlace::getLatencia() const { return latencia_; }
bool               Enlace::isAtivo()    const { return ativo_; }

void Enlace::setAtivo(bool ativo) {
    ativo_ = ativo;
}

bool Enlace::transmitir(const std::string& origem,
                        const std::string& destino,
                        int tempo)
{
    numPassos_++;

    if (!ativo_) {
        pacotesDescartados_++;
        std::cout << "[t=" << tempo << "] Enlace " << id_
                  << " esta falho. Pacote " << origem
                  << " -> " << destino << " descartado.\n";
        return false;
    }

    // Determina o no de destino deste salto
    No* noDestino = nullptr;
    if (noA_ && noA_->getId() == destino) {
        noDestino = noA_;
    } else if (noB_ && noB_->getId() == destino) {
        noDestino = noB_;
    } else {
        // destino nao e extremidade direta: entrega ao no oposto ao de origem
        if (noA_ && noA_->getId() == origem) {
            noDestino = noB_;
        } else {
            noDestino = noA_;
        }
    }

    pacotesTransmitidos_++;
    // Simples: 1 pacote por passo usa 1/banda da capacidade
    double utilizacao = (banda_ > 0.0) ? (1.0 / banda_) : 0.0;
    utilizacaoAcumulada_ += utilizacao;

    std::cout << "[t=" << tempo << "] " << origem
              << " -> " << destino
              << " (" << id_ << ", latencia=" << latencia_ << "ms) OK\n";

    if (noDestino) {
        noDestino->receberPacote(origem, destino, tempo);
    }

    return true;
}

int    Enlace::getPacotesTransmitidos() const { return pacotesTransmitidos_; }
int    Enlace::getPacotesDescartados()  const { return pacotesDescartados_; }

double Enlace::getUtilizacao() const {
    if (numPassos_ == 0) return 0.0;
    return utilizacaoAcumulada_ / numPassos_;
}

void Enlace::reportarMetricas(ColetorDeMetricas& coletor) const {
    coletor.registrarEnlace(id_,
                            pacotesTransmitidos_,
                            pacotesDescartados_,
                            getUtilizacao());
}
