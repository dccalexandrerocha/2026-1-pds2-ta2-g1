#pragma once

#include <string>
#include <vector>

class Enlace;

/**
 * @brief Tipos possiveis de no na rede.
 */
enum class TipoNo {
    HOST,
    ROTEADOR,
    SWITCH
};

/**
 * @brief Classe base abstrata que representa um no da rede.
 *
 * Um No conhece seu ID, tipo, endereco IP e os enlaces conectados
 * a ele. Subclasses concretas (Host, Roteador, Switch) definem o
 * comportamento especifico ao receber e encaminhar pacotes.
 */
class No {
public:
    /**
     * @brief Constroi um No com ID, tipo e endereco.
     * @param id       Identificador unico do no.
     * @param tipo     Tipo do no (HOST, ROTEADOR ou SWITCH).
     * @param endereco Endereco IP do no.
     */
    No(const std::string& id, TipoNo tipo, const std::string& endereco);

    /// @brief Destrutor virtual para permitir heranca correta.
    virtual ~No() = default;

    /// @brief Retorna o ID do no.
    const std::string& getId() const;

    /// @brief Retorna o tipo do no.
    TipoNo getTipo() const;

    /// @brief Retorna o endereco IP do no.
    const std::string& getEndereco() const;

    /// @brief Retorna true se o no esta ativo.
    bool isAtivo() const;

    /// @brief Adiciona um enlace conectado a este no.
    void adicionarEnlace(Enlace* enlace);

    /// @brief Retorna a lista de enlaces conectados ao no.
    const std::vector<Enlace*>& getEnlaces() const;

    /**
     * @brief Recebe um pacote vindo de um enlace e decide o que fazer.
     *
     * Metodo abstrato: cada subclasse implementa o comportamento
     * especifico (encaminhar, entregar ou ignorar).
     *
     * @param origem  ID do no de origem do pacote.
     * @param destino ID do no de destino do pacote.
     * @param tempo   Instante de tempo atual da simulacao.
     */
    virtual void receberPacote(const std::string& origem,
                               const std::string& destino,
                               int tempo) = 0;

    /// @brief Retorna contagem de pacotes recebidos.
    int getPacotesRecebidos() const;

    /// @brief Retorna contagem de pacotes enviados.
    int getPacotesEnviados() const;

    /// @brief Retorna contagem de pacotes descartados.
    int getPacotesDescartados() const;

protected:
    std::string          id_;
    TipoNo               tipo_;
    std::string          endereco_;
    bool                 ativo_;
    std::vector<Enlace*> enlaces_;

    int pacotesRecebidos_;
    int pacotesEnviados_;
    int pacotesDescartados_;
};


/**
 * @brief No do tipo Host.
 *
 * Ponto de origem ou destino de pacotes. Nao encaminha pacotes
 * para outros nos.
 */
class Host : public No {
public:
    Host(const std::string& id, const std::string& endereco);

    /**
     * @brief Recebe um pacote. Se for o destino, registra a entrega.
     *        Caso contrario, descarta.
     */
    void receberPacote(const std::string& origem,
                       const std::string& destino,
                       int tempo) override;
};


/**
 * @brief No do tipo Roteador.
 *
 * Encaminha pacotes com base na tabela de roteamento local,
 * atualizada pelo ProtocoloDeRoteamento.
 */
class Roteador : public No {
public:
    Roteador(const std::string& id, const std::string& endereco);

    /**
     * @brief Recebe um pacote e o encaminha pelo proximo salto
     *        indicado na tabela de roteamento.
     */
    void receberPacote(const std::string& origem,
                       const std::string& destino,
                       int tempo) override;
};


/**
 * @brief No do tipo Switch.
 *
 * Encaminha pacotes com base no endereco de destino dentro
 * da rede local (camada de enlace).
 */
class Switch : public No {
public:
    Switch(const std::string& id, const std::string& endereco);

    /**
     * @brief Recebe um pacote e o encaminha pela interface correta.
     */
    void receberPacote(const std::string& origem,
                       const std::string& destino,
                       int tempo) override;
};
