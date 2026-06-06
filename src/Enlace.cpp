#include "Enlace.hpp"
#include "No.hpp"
#include "ColetorDeMetricas.hpp"
#include "Excecoes.hpp"
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
    try {
        numPassos_++;

        // Validação defensiva: origem e destino não vazios
        if (origem.empty() || destino.empty()) {
            throw ExcecaoRede("Origem ou destino vazio em transmissão");
        }

        if (!ativo_) {
            pacotesDescartados_++;
            std::cout << "[t=" << tempo << "] Enlace " << id_
                      << " está falho. Pacote " << origem
                      << " -> " << destino << " descartado.\n";
            return false;
        }

        // Validação defensiva: ponteiros não nulos
        if (!noA_ || !noB_) {
            throw ExcecaoMemoria("Enlace " + id_ + " tem nó nulo");
        }

        // Determina o nó de destino deste salto
        No* noDestino = nullptr;
        if (noA_->getId() == destino) {
            noDestino = noA_;
        } else if (noB_->getId() == destino) {
            noDestino = noB_;
        } else {
            // destino não é extremidade direta: entrega ao nó oposto ao de origem
            if (noA_->getId() == origem) {
                noDestino = noB_;
            } else {
                noDestino = noA_;
            }
        }

        // Validação defensiva: noDestino não deve ser nulo
        if (!noDestino) {
            throw ExcecaoMemoria("Nó de destino calculado como nulo no enlace " + id_);
        }

        pacotesTransmitidos_++;
        
        // Validação defensiva: evitar divisão por zero
        double utilizacao = 0.0;
        if (banda_ > 0.0) {
            utilizacao = 1.0 / banda_;
        } else {
            std::cout << "[AVISO] Enlace " << id_ << " tem banda <= 0. Utilizando 0%.\n";
        }
        
        utilizacaoAcumulada_ += utilizacao;

        std::cout << "[t=" << tempo << "] " << origem
                  << " -> " << destino
                  << " (" << id_ << ", latência=" << latencia_ << "ms) OK\n";

        // Chamada segura ao método virtual
        noDestino->receberPacote(origem, destino, tempo);

        return true;

    } catch (const ExcecaoMemoria& e) {
        std::cout << "[ERRO] " << e.what() << "\n";
        pacotesDescartados_++;
        return false;
    } catch (const ExcecaoRede& e) {
        std::cout << "[ERRO] " << e.what() << "\n";
        pacotesDescartados_++;
        return false;
    } catch (const std::exception& e) {
        std::cout << "[ERRO] Exceção em transmissão: " << e.what() << "\n";
        pacotesDescartados_++;
        return false;
    }
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
