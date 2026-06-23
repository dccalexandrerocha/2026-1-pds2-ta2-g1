/**
 * @file main.cpp
 * @brief Ponto de entrada do simulador de rede de telecomunicacao.
 *
 * Instancia o Simulador e delega a ele o controle do loop principal
 * de leitura de comandos (REPL).
 *
 * @author Grupo 1 - PDS2 TA2 2026/1
 */

#include "Simulador.hpp"

/**
 * @brief Funcao principal do programa.
 * @return 0 em caso de execucao normal.
 */
int main() {
    Simulador sim;
    sim.executar();
    return 0;
}