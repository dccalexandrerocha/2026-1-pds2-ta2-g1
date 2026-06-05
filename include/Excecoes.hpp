#pragma once

#include <stdexcept>
#include <string>

/**
 * @brief Exceção base para o simulador de rede.
 *
 * Todas as exceções do projeto herdam desta classe.
 */
class ExcecaoSimulador : public std::runtime_error {
public:
    explicit ExcecaoSimulador(const std::string& mensagem)
        : std::runtime_error(mensagem) {}
};

/**
 * @brief Exceção lançada quando uma operação de rede falha.
 *
 * Exemplos: nó não encontrado, enlace não existe, topologia inválida.
 */
class ExcecaoRede : public ExcecaoSimulador {
public:
    explicit ExcecaoRede(const std::string& mensagem)
        : ExcecaoSimulador("ERRO_REDE: " + mensagem) {}
};

/**
 * @brief Exceção lançada quando há erro de entrada do usuário.
 *
 * Exemplos: comando inválido, argumentos faltando, tipo de dado inválido.
 */
class ExcecaoEntrada : public ExcecaoSimulador {
public:
    explicit ExcecaoEntrada(const std::string& mensagem)
        : ExcecaoSimulador("ERRO_ENTRADA: " + mensagem) {}
};

/**
 * @brief Exceção lançada quando há erro de arquivo.
 *
 * Exemplos: arquivo não pode ser aberto, permissão negada, I/O falha.
 */
class ExcecaoArquivo : public ExcecaoSimulador {
public:
    explicit ExcecaoArquivo(const std::string& mensagem)
        : ExcecaoSimulador("ERRO_ARQUIVO: " + mensagem) {}
};

/**
 * @brief Exceção lançada quando há erro de estado.
 *
 * Exemplos: operação inválida para o estado atual, topologia vazia.
 */
class ExcecaoEstado : public ExcecaoSimulador {
public:
    explicit ExcecaoEstado(const std::string& mensagem)
        : ExcecaoSimulador("ERRO_ESTADO: " + mensagem) {}
};

/**
 * @brief Exceção lançada quando há erro de alocação de memória.
 *
 * Exemplos: ponteiro nulo, falha em unique_ptr, dangling pointer.
 */
class ExcecaoMemoria : public ExcecaoSimulador {
public:
    explicit ExcecaoMemoria(const std::string& mensagem)
        : ExcecaoSimulador("ERRO_MEMORIA: " + mensagem) {}
};
