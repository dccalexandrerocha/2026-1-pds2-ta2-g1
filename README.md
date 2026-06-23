> Simulador discreto de redes de computadores, executado no terminal, desenvolvido em C++.

## Integrantes

| Nome                    | Matrícula  |
|-------------------------|------------|
| Mateus Poelman Pinheiro | 2025019437 |
| Gabriel Figueiredo      | 2025019461 |
| Lucas Alves             | 2025019569 |
| Saul Gonzalez           | 2025065358 |

---

## O que é este projeto?

Redes de computadores são sistemas complexos cujo comportamento depende de
diversos fatores: topologia, protocolos de roteamento e capacidade dos enlaces.
Avaliar como uma rede responde a diferentes cenários — como falhas de enlaces
ou congestionamento — normalmente exige infraestrutura física cara ou
ferramentas de simulação sofisticadas.

Este projeto propõe um simulador simples e didático, que roda inteiramente no
terminal. O usuário monta uma rede digitando comandos, injeta pacotes entre os
nós e observa o encaminhamento hop a hop em tempo simulado. É possível derrubar
enlaces durante a simulação e ver como o roteamento se adapta em tempo real.

O objetivo principal é acadêmico: aplicar os conceitos de orientação a objetos
(herança, polimorfismo, encapsulamento) em um problema concreto, usando apenas
a STL do C++, sem dependências externas.

---

## Pré-requisitos

- `g++` com suporte a **C++17**
- `make`
- `gcovr` (apenas para o relatório de cobertura, gerado pelo `make test`):
  ```
  pip install gcovr
  ```

> O arquivo `tests/doctest.h` já está incluído no repositório — não é preciso instalar nada além do acima.

---

## Como compilar e executar

```
# Compilar o simulador
make

# Executar
./simulador
```

---

## Testes de unidade e cobertura (C7/C8)

O projeto usa o framework [doctest](https://github.com/doctest/doctest) para testes de unidade e [gcovr](https://gcovr.com/) para medir a cobertura.

### ⚠️ IMPORTANTE: execute `make test` no Prompt de Comando (CMD)

**O `make test` deve ser rodado no CMD do Windows, não no Git Bash, WSL ou PowerShell.**

O Git Bash e o WSL alteram a forma como caminhos e variáveis de ambiente são
resolvidos no Makefile, o que causa falhas na compilação do runner de testes
e na execução do gcovr. Use sempre o **Prompt de Comando nativo (`cmd.exe`)**.

Para abrir o CMD e navegar até o projeto:

```
cd caminho\para\o\projeto
make test
```

### Executar os testes

```
make test
```

O comando acima:
1. Compila o runner de testes com flags de cobertura (`--coverage`).
2. Executa todos os testes e exibe o resultado no terminal.
3. Gera o relatório de cobertura em `coverage/index.html` (HTML navegável) e `coverage/coverage.xml`.

### Resultado esperado

```
[doctest] test cases:  91 |  91 passed | 0 failed | 0 skipped
[doctest] assertions: 150 | 150 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Classes testadas

| Suite de testes        | Arquivo testado               | Casos |
|------------------------|-------------------------------|-------|
| `No`                   | `src/No.cpp`                  |   9   |
| `Enlace`               | `src/Enlace.cpp`              |  10   |
| `ColetorDeMetricas`    | `src/ColetorDeMetricas.cpp`   |  10   |
| `EscalonadorDeEventos` | `src/EscalonadorDeEventos.cpp`|   9   |
| `TopologiaDeRede`      | `src/TopologiaDeRede.cpp`     |  16   |
| `RoteamentoDijkstra`   | `src/ProtocoloDeRoteamento.cpp`|  10  |
| `Simulador`            | `src/Simulador.cpp`           |  18   |
| `SimuladorComandos`    | `src/Simulador.cpp`           |   9   |

---

## Limpar arquivos gerados

```
make clean
```

Remove o binário do simulador, o runner de testes, os arquivos `.gcda`/`.gcno` e a pasta `coverage/`.

---

## Exemplo de uso completo

A seguir, uma sessão típica do simulador, desde a montagem da rede até a
exportação das métricas.

### 1. Montar a topologia

Antes de iniciar, defina os nós e os enlaces da rede. A topologia deste
exemplo fica assim:

```
H1 ---[E1, 5ms]--- R1 ---[E2, 10ms]--- R2 ---[E3, 5ms]--- H2
```

```
> add_no host H1
[OK] No H1 (host) adicionado.

> add_no roteador R1
[OK] No R1 (roteador) adicionado.

> add_no roteador R2
[OK] No R2 (roteador) adicionado.

> add_no host H2
[OK] No H2 (host) adicionado.

> add_enlace E1 H1 R1 latencia=5
[OK] Enlace E1 adicionado: H1 <-> R1

> add_enlace E2 R1 R2 latencia=10
[OK] Enlace E2 adicionado: R1 <-> R2

> add_enlace E3 R2 H2 latencia=5
[OK] Enlace E3 adicionado: R2 <-> H2
```

### 2. Iniciar a simulação

```
> iniciar
[t=0] Topologia validada: 4 nos, 3 enlaces.
[t=0] Simulacao iniciada.
```

### 3. Injetar um pacote e avançar passo a passo

```
> injetar H1 H2
[t=0] Pacote injetado: H1 -> H2
[t=0] Caminho calculado: H1 -> R1 -> R2 -> H2

> proximo
[t=1] H1 -> R1 (E1, latencia=5ms) OK

> proximo
[t=2] R1 -> R2 (E2, latencia=10ms) OK

> proximo
[t=3] R2 -> H2 (E3, latencia=5ms) OK
[t=3] Pacote entregue em H2 (origem: H1).
```

### 4. Simular uma falha de enlace

```
> injetar H1 H2
[t=3] Pacote injetado: H1 -> H2
[t=3] Caminho calculado: H1 -> R1 -> R2 -> H2

> falha E2
[t=3] Enlace E2 marcado como falho.
[t=3] Recalculando rotas...

> proximo
[t=4] Enlace E2 esta falho. Pacote H1 -> H2 descartado.

> recuperar E2
[t=4] Enlace E2 restaurado.
```

### 5. Ver métricas e encerrar

```
> metricas

=== Metricas por Enlace ===
E1 | transmitidos: 2 | descartados: 0 | util.media: 0.01 | util.max: 0.01
E2 | transmitidos: 1 | descartados: 1 | util.media: 0.01 | util.max: 0.01
E3 | transmitidos: 1 | descartados: 0 | util.media: 0.01 | util.max: 0.01

=== Totais Globais ===
Pacotes entregues: 1
Pacotes perdidos:  1

> exportar resultados.csv
[OK] Metricas exportadas para resultados.csv

> encerrar
[t=4] Simulacao encerrada.
```

### 6. Easter egg: créditos do projeto

```
> autores
```

Exibe os integrantes do grupo com arte ASCII e abre um vídeo de agradecimento (`vid.mp4`) com o player padrão do sistema.

---

## Referência de comandos

| Comando                                     | Quando usar             | Descrição                                     |
| ------------------------------------------- | ----------------------- | --------------------------------------------- |
| `add_no <tipo> <id>`                        | Antes de `iniciar`      | Adiciona um nó. Tipos: host, roteador, switch |
| `add_enlace <id> <noA> <noB> latencia=<ms>` | Antes de `iniciar`      | Conecta dois nós com um enlace                |
| `iniciar`                                   | Após montar a topologia | Valida a rede e inicia a simulação            |
| `pausar`                                    | Durante a simulação     | Pausa a simulação                             |
| `retomar`                                   | Após pausar             | Retoma a simulação pausada                    |
| `proximo`                                   | Durante a simulação     | Avança um passo de tempo                      |
| `injetar <origem> <destino>`                | Durante a simulação     | Injeta um pacote entre dois nós               |
| `falha <idEnlace>`                          | Durante a simulação     | Marca um enlace como falho                    |
| `recuperar <idEnlace>`                      | Durante a simulação     | Restaura um enlace falho                      |
| `metricas`                                  | Qualquer momento        | Exibe métricas no terminal                    |
| `exportar <arquivo.csv>`                    | Qualquer momento        | Exporta métricas para um arquivo CSV          |
| `encerrar`                                  | Qualquer momento        | Encerra a simulação                           |
| `ajuda`                                     | Qualquer momento        | Lista os comandos disponíveis                 |
| `autores`                                   | Qualquer momento        | Exibe os integrantes e abre o video           |

---

## Estrutura do projeto

```
simulador-rede/
├── include/               # Cabeçalhos (.hpp)
│   ├── No.hpp                     # Classe base No e subclasses Host, Roteador, Switch
│   ├── Enlace.hpp                 # Conexão entre dois nós
│   ├── TopologiaDeRede.hpp        # Grafo da rede
│   ├── ProtocoloDeRoteamento.hpp  # Classe base abstrata + RoteamentoDijkstra
│   ├── EscalonadorDeEventos.hpp   # Fila de eventos ordenada por tempo
│   ├── ColetorDeMetricas.hpp      # Coleta e exporta métricas
│   ├── Simulador.hpp              # Loop principal e leitura de comandos
│   ├── Excecoes.hpp               # Hierarquia de excecoes do simulador
│   └── Validacao.hpp              # Utilitarios de validacao defensiva
├── src/                   # Implementações (.cpp)
│   ├── No.cpp
│   ├── Enlace.cpp
│   ├── TopologiaDeRede.cpp
│   ├── ProtocoloDeRoteamento.cpp
│   ├── EscalonadorDeEventos.cpp
│   ├── ColetorDeMetricas.cpp
│   ├── Simulador.cpp
│   └── main.cpp
├── tests/                 # Testes de unidade (doctest)
│   ├── doctest.h          # Framework doctest (single-header, sem instalacao)
│   └── testes.cpp         # Suites de testes para todas as classes
├── design/                # Diagramas CRC e documentos de design
│   ├── crc_01_Simulador.txt
│   ├── crc_02_TopologiaDeRede.txt
│   ├── crc_03_No.txt
│   ├── crc_04_Enlace.txt
│   ├── crc_05_EscalonadorDeEventos.txt
│   ├── crc_06_ProtocoloDeRoteamento.txt
│   ├── crc_07_ColetorDeMetricas.txt
│   └── user_stories_v2.md
├── build/                 # Binarios e arquivos de cobertura (.gcda/.gcno)
├── coverage/              # Relatorios HTML/XML de cobertura (gerados por make test)
├── vid.mp4                # Video de agradecimento (aberto pelo comando "autores")
├── Doxyfile               # Configuração do Doxygen
├── Makefile               # Automação: make / make test / make clean
└── README.md
```

---

## Gerar documentação (Doxygen)

Todos os arquivos `.hpp` e `.cpp` estão documentados com comentários Doxygen
(`@file`, `@brief`, `@param`, `@return`, `@throws`). Para gerar a documentação
navegável em HTML, com o [Doxygen](https://www.doxygen.nl/) instalado:

```
doxygen Doxyfile
```

Abra `docs/doxygen/html/index.html` no navegador para navegar pela
documentação de todas as classes e métodos.

---

## Arquitetura

O simulador segue o padrão de eventos discretos. Os principais componentes são:

| Classe                   | Responsabilidade                                                     |
|--------------------------|----------------------------------------------------------------------|
| `Simulador`              | REPL, ciclo de vida, orquestração geral                              |
| `TopologiaDeRede`        | Grafo de nós e enlaces; validação de conectividade (BFS)             |
| `No` / `Host` / `Roteador` / `Switch` | Hierarquia de nós; polimorfismo via `receberPacote()`   |
| `Enlace`                 | Transmissão salto a salto; contadores de uso                         |
| `EscalonadorDeEventos`   | Fila de prioridade de eventos por tempo; relógio global              |
| `RoteamentoDijkstra`     | Cálculo de menor caminho por latência; tabela de próximo salto       |
| `ColetorDeMetricas`      | Acúmulo e exportação (CSV) de métricas por enlace e por nó           |
| `Excecoes`               | Hierarquia de exceções tipadas para erros de rede, entrada e arquivo |
| `Validacao`              | Funções inline de validação defensiva reutilizadas em todo o projeto |

---

## Tecnologias

- **Linguagem:** C++17
- **Bibliotecas:** STL apenas (sem dependências externas)
- **Testes:** [doctest](https://github.com/doctest/doctest) (single-header)
- **Cobertura:** [gcovr](https://gcovr.com/)
- **Documentação:** Doxygen
- **Build:** Make