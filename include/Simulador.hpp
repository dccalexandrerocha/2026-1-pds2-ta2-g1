#pragma once

#include <string>
#include <memory>
#include <fstream>

#include "TopologiaDeRede.hpp"
#include "EscalonadorDeEventos.hpp"
#include "ColetorDeMetricas.hpp"
#include "ProtocoloDeRoteamento.hpp"
#include "Excecoes.hpp"

/**
 * @brief Estados possiveis do ciclo de vida da simulacao.
 */
enum class EstadoSimulacao {
    PARADO,
    EM_EXECUCAO,
    PAUSADO,
    ENCERRADO
};

/**
 * @brief Orquestrador principal da simulacao.
 *
 * O Simulador controla o ciclo de vida da simulacao, interpreta
 * comandos do usuario via terminal, delega o processamento de cada
 * passo ao EscalonadorDeEventos e aciona o ColetorDeMetricas ao
 * encerrar.
 */
class Simulador {
public:
    Simulador();

    /**
     * @brief Inicia o loop principal de leitura de comandos (REPL).
     *
     * Le comandos do terminal em loop ate que o usuario digite
     * "encerrar" ou o programa seja interrompido.
     */
    void executar();

    /**
     * @brief Inicia a simulacao a partir do estado PARADO.
     *
     * Valida a topologia antes de iniciar. Exibe erro se a
     * topologia nao estiver conectada.
     */
    void iniciar();

    /// @brief Pausa a simulacao em execucao.
    void pausar();

    /// @brief Retoma a simulacao pausada.
    void retomar();

    /**
     * @brief Encerra a simulacao e exibe as metricas finais.
     *
     * Aciona ColetorDeMetricas::exibirResumo() e oferece ao usuario
     * a opcao de exportar o CSV.
     */
    void encerrar();

    /**
     * @brief Avanca a simulacao um passo de tempo.
     *
     * Chama EscalonadorDeEventos::avancarPasso() e exibe o estado
     * atual no terminal.
     */
    void avancarPasso();

    /**
     * @brief Injeta um pacote na rede.
     * @param origem  ID do no de origem.
     * @param destino ID do no de destino.
     */
    void injetarPacote(const std::string& origem,
                       const std::string& destino);

    /**
     * @brief Altera o estado de um enlace.
     * @param idEnlace ID do enlace.
     * @param ativo    true para ativar; false para marcar como falho.
     */
    void alterarEstadoEnlace(const std::string& idEnlace, bool ativo);

    /// @brief Retorna o estado atual da simulacao.
    EstadoSimulacao getEstado() const;

    /**
     * @brief Interpreta e executa um comando digitado pelo usuario.
     * @param linha Linha de texto lida do terminal.
     */
    void processarComando(const std::string& linha);

private:

    /// @brief Exibe os comandos disponiveis no terminal.
    void exibirAjuda() const;

    /// @brief Exibe os autores do projeto com arte ASCII e abre o video de agradecimento.
    void exibirAutores() const;

    /// @brief Coleta metricas de todos os nos e enlaces para o coletor.
    void coletarMetricas();

    EstadoSimulacao               estado_;
    TopologiaDeRede               topologia_;
    EscalonadorDeEventos          escalonador_;
    ColetorDeMetricas             coletor_;
    std::unique_ptr<ProtocoloDeRoteamento> protocolo_;
    std::ofstream                 logFile_;
};