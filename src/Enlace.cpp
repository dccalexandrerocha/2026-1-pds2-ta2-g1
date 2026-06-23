/**
 * @file Enlace.cpp
 * @brief Implementacao da classe Enlace.
 *
 * Gerencia o estado fisico da conexao entre dois nos, acumula
 * contadores de uso e executa a transmissao salto a salto de
 * pacotes, incluindo tratamento de falhas.
 *
 * @author Grupo 1 - PDS2 TA2 2026/1
 */

#include "Enlace.hpp"
#include "No.hpp"
#include "ColetorDeMetricas.hpp"
#include "Excecoes.hpp"
#include "Validacao.hpp"
#include <iostream>

/**
 * @brief Constroi um Enlace validando todos os parametros.
 *
 * Usa o namespace Validacao para garantir que ID, nos e
 * parametros numericos sejam validos antes de armazenar.
 */
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
{
    Validacao::validarNaoVazio(id_, "id do enlace");
    Validacao::validarNaoNulo(noA_, "noA do enlace");
    Validacao::validarNaoNulo(noB_, "noB do enlace");
    Validacao::validarIntervalo(banda_, 0.000001, 1.0e12, "banda do enlace");
    Validacao::validarIntervalo(latencia_, 0.0, 1.0e12, "latencia do enlace");
}

const std::string& Enlace::getId()       const { return id_; }
No*                Enlace::getNoA()      const { return noA_; }
No*                Enlace::getNoB()      const { return noB_; }
double             Enlace::getBanda()    const { return banda_; }
double             Enlace::getLatencia() const { return latencia_; }
bool               Enlace::isAtivo()    const { return ativo_; }

/**
 * @brief Altera o estado ativo/falho do enlace.
 *
 * Quando definido como falso, o enlace passa a descartar
 * todos os pacotes ate ser restaurado.
 */
void Enlace::setAtivo(bool ativo) {
    ativo_ = ativo;
}

/**
 * @brief Realiza a transmissao de um pacote entre dois nos.
 *
 * Determina o no de destino deste salto (pode ser noA ou noB,
 * dependendo de quem originou o pacote) e chama receberPacote()
 * no no destino. Se o enlace estiver falho, incrementa o contador
 * de descartados e retorna false sem chamar o no.
 *
 * A utilizacao e calculada como 1/banda por pacote transmitido,
 * servindo como proxy da taxa de ocupacao do enlace.
 */
bool Enlace::transmitir(const std::string& origem,
                        const std::string& destino,
                        int tempo)
{
    try {
        numPassos_++;

        if (origem.empty() || destino.empty()) {
            throw ExcecaoRede("Origem ou destino vazio em transmissao");
        }

        if (!ativo_) {
            pacotesDescartados_++;
            std::cout << "[t=" << tempo << "] Enlace " << id_
                      << " esta falho. Pacote " << origem
                      << " -> " << destino << " descartado.\n";
            return false;
        }

        if (!noA_ || !noB_) {
            throw ExcecaoMemoria("Enlace " + id_ + " tem no nulo");
        }

        // Determina o no de destino deste salto
        No* noDestino = nullptr;
        if (noA_->getId() == destino) {
            noDestino = noA_;
        } else if (noB_->getId() == destino) {
            noDestino = noB_;
        } else {
            // destino nao e extremidade direta: entrega ao no oposto ao de origem
            if (noA_->getId() == origem) {
                noDestino = noB_;
            } else {
                noDestino = noA_;
            }
        }

        if (!noDestino) {
            throw ExcecaoMemoria("No de destino calculado como nulo no enlace " + id_);
        }

        pacotesTransmitidos_++;

        double utilizacao = 0.0;
        if (banda_ > 0.0) {
            utilizacao = 1.0 / banda_;
        } else {
            std::cout << "[AVISO] Enlace " << id_ << " tem banda <= 0. Utilizando 0%.\n";
        }
        utilizacaoAcumulada_ += utilizacao;

        std::cout << "[t=" << tempo << "] " << origem
                  << " -> " << destino
                  << " (" << id_ << ", latencia=" << latencia_ << "ms) OK\n";

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
        std::cout << "[ERRO] Excecao em transmissao: " << e.what() << "\n";
        pacotesDescartados_++;
        return false;
    }
}

int    Enlace::getPacotesTransmitidos() const { return pacotesTransmitidos_; }
int    Enlace::getPacotesDescartados()  const { return pacotesDescartados_; }

/**
 * @brief Calcula a utilizacao media acumulada do enlace.
 *
 * Retorna zero se nenhum passo de tempo foi registrado ainda,
 * evitando divisao por zero.
 */
double Enlace::getUtilizacao() const {
    if (numPassos_ == 0) return 0.0;
    return utilizacaoAcumulada_ / numPassos_;
}

/**
 * @brief Envia os contadores acumulados ao ColetorDeMetricas.
 *
 * Chamado pelo Simulador ao final de cada coleta de metricas.
 */
void Enlace::reportarMetricas(ColetorDeMetricas& coletor) const {
    coletor.registrarEnlace(id_,
                            pacotesTransmitidos_,
                            pacotesDescartados_,
                            getUtilizacao());
}