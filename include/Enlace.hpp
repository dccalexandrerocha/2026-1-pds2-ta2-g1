#pragma once

#include <string>

class No;
class ColetorDeMetricas;

/**
 * @brief Representa a conexao fisica entre dois nos da rede.
 *
 * Um Enlace conhece seus dois nos extremos, largura de banda,
 * latencia base e estado (ativo ou falho). Registra contadores
 * de uso que sao reportados ao ColetorDeMetricas.
 */
class Enlace {
public:
    /**
     * @brief Constroi um Enlace entre dois nos.
     * @param id         Identificador unico do enlace.
     * @param noA        Ponteiro para o primeiro no.
     * @param noB        Ponteiro para o segundo no.
     * @param banda      Largura de banda maxima (Mbps).
     * @param latencia   Latencia base de propagacao (ms).
     */
    Enlace(const std::string& id,
           No* noA,
           No* noB,
           double banda,
           double latencia);

    /// @brief Retorna o ID do enlace.
    const std::string& getId() const;

    /// @brief Retorna ponteiro para o primeiro no.
    No* getNoA() const;

    /// @brief Retorna ponteiro para o segundo no.
    No* getNoB() const;

    /// @brief Retorna a largura de banda maxima.
    double getBanda() const;

    /// @brief Retorna a latencia base.
    double getLatencia() const;

    /// @brief Retorna true se o enlace esta ativo.
    bool isAtivo() const;

    /**
     * @brief Altera o estado do enlace.
     * @param ativo true para ativar, false para marcar como falho.
     */
    void setAtivo(bool ativo);

    /**
     * @brief Tenta transmitir um pacote pelo enlace.
     *
     * Se o enlace estiver falho, o pacote e descartado e uma
     * mensagem e exibida no terminal. Caso contrario, o pacote
     * e entregue ao no de destino.
     *
     * @param origem  ID do no de origem.
     * @param destino ID do no de destino.
     * @param tempo   Instante de tempo atual da simulacao.
     * @return true se o pacote foi transmitido com sucesso.
     */
    bool transmitir(const std::string& origem,
                    const std::string& destino,
                    int tempo);

    /// @brief Retorna total de pacotes transmitidos com sucesso.
    int getPacotesTransmitidos() const;

    /// @brief Retorna total de pacotes descartados.
    int getPacotesDescartados() const;

    /// @brief Retorna a taxa de utilizacao atual (0.0 a 1.0).
    double getUtilizacao() const;

    /**
     * @brief Reporta os contadores atuais ao ColetorDeMetricas.
     * @param coletor Referencia ao coletor de metricas da simulacao.
     */
    void reportarMetricas(ColetorDeMetricas& coletor) const;

private:
    std::string id_;
    No*         noA_;
    No*         noB_;
    double      banda_;
    double      latencia_;
    bool        ativo_;

    int    pacotesTransmitidos_;
    int    pacotesDescartados_;
    double utilizacaoAcumulada_;
    int    numPassos_;
};
