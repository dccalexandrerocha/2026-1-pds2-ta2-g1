#pragma once

#include <string>
#include <vector>
#include "Excecoes.hpp"

/**
 * @brief Namespace com utilitários de validação defensiva.
 *
 * Contém funções auxiliares para validar entradas, ponteiros e estado.
 */
namespace Validacao {

/**
 * @brief Valida se um ponteiro não é nulo.
 * @param ptr    Ponteiro a validar.
 * @param nome   Nome do objeto (para mensagem de erro).
 * @throws ExcecaoMemoria Se ptr é nullptr.
 */
template <typename T>
inline void validarNaoNulo(T* ptr, const std::string& nome) {
    if (!ptr) {
        throw ExcecaoMemoria(nome + " é nullptr");
    }
}

/**
 * @brief Valida se uma string não está vazia.
 * @param str   String a validar.
 * @param nome  Nome do campo (para mensagem de erro).
 * @throws ExcecaoEntrada Se str está vazia.
 */
inline void validarNaoVazio(const std::string& str, const std::string& nome) {
    if (str.empty()) {
        throw ExcecaoEntrada(nome + " não pode estar vazio");
    }
}

/**
 * @brief Valida se um vector não está vazio.
 * @param vec   Vector a validar.
 * @param nome  Nome do campo (para mensagem de erro).
 * @throws ExcecaoEstado Se vec está vazio.
 */
template <typename T>
inline void validarNaoVazio(const std::vector<T>& vec, const std::string& nome) {
    if (vec.empty()) {
        throw ExcecaoEstado(nome + " está vazio");
    }
}

/**
 * @brief Valida se um índice é válido para um vector.
 * @param indice Índice a validar.
 * @param tamanho Tamanho do vector.
 * @param nome   Nome do campo (para mensagem de erro).
 * @throws ExcecaoEntrada Se indice >= tamanho.
 */
inline void validarIndice(size_t indice, size_t tamanho, const std::string& nome) {
    if (indice >= tamanho) {
        throw ExcecaoEntrada(nome + ": índice " + std::to_string(indice) +
                           " inválido (tamanho: " + std::to_string(tamanho) + ")");
    }
}

/**
 * @brief Valida se um valor está dentro de um intervalo.
 * @param valor Valor a validar.
 * @param minimo Valor mínimo aceito.
 * @param maximo Valor máximo aceito.
 * @param nome   Nome do campo (para mensagem de erro).
 * @throws ExcecaoEntrada Se valor está fora do intervalo.
 */
inline void validarIntervalo(double valor, double minimo, double maximo,
                            const std::string& nome) {
    if (valor < minimo || valor > maximo) {
        throw ExcecaoEntrada(nome + ": valor " + std::to_string(valor) +
                           " fora do intervalo [" + std::to_string(minimo) +
                           ", " + std::to_string(maximo) + "]");
    }
}

/**
 * @brief Tenta fazer parsing de um valor double.
 * @param str String contendo o número.
 * @throws ExcecaoEntrada Se parsing falhar.
 * @return O valor double parseado.
 */
inline double parseDouble(const std::string& str) {
    try {
        size_t idx;
        double valor = std::stod(str, &idx);
        if (idx != str.length()) {
            throw ExcecaoEntrada("caracteres após o número: " + str);
        }
        return valor;
    } catch (const std::invalid_argument&) {
        throw ExcecaoEntrada("valor numérico inválido: " + str);
    } catch (const std::out_of_range&) {
        throw ExcecaoEntrada("valor numérico fora de alcance: " + str);
    }
}

/**
 * @brief Tenta fazer parsing de um valor int.
 * @param str String contendo o número.
 * @throws ExcecaoEntrada Se parsing falhar.
 * @return O valor int parseado.
 */
inline int parseInt(const std::string& str) {
    try {
        size_t idx;
        int valor = std::stoi(str, &idx);
        if (idx != str.length()) {
            throw ExcecaoEntrada("caracteres após o número: " + str);
        }
        return valor;
    } catch (const std::invalid_argument&) {
        throw ExcecaoEntrada("valor inteiro inválido: " + str);
    } catch (const std::out_of_range&) {
        throw ExcecaoEntrada("valor inteiro fora de alcance: " + str);
    }
}

/**
 * @brief Extrai um valor de uma string no formato "chave=valor".
 * @param str    String a processar (ex: "latencia=5.5").
 * @param chave  Chave esperada (ex: "latencia").
 * @throws ExcecaoEntrada Se formato está inválido.
 * @return A parte do valor (ex: "5.5").
 */
inline std::string extrairValor(const std::string& str, const std::string& chave) {
    const std::string prefixo = chave + "=";
    if (str.rfind(prefixo, 0) != 0) {
    throw ExcecaoEntrada("esperado formato '" + chave + "=<valor>'");
}

std::string valor = str.substr(prefixo.length());
    if (valor.empty()) {
        throw ExcecaoEntrada(chave + " não pode estar vazio");
    }
    return valor;
}

}  // namespace Validacao
