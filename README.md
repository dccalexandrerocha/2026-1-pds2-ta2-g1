> Simulador discreto de redes de computadores, executado no terminal, desenvolvido em C++.

## Integrantes

- Mateus Poelman Pinheiro
- Gabriel Figueiredo
- Lucas Alves
- Saul Gonzalez

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

## Como compilar e executar

**Pré-requisitos:**

 g++ com suporte a C++17 e Make.

```bash
# Compilar
make

# Executar
./simulador
```

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
[t=3] Pacote entregue em H2.
```

### 4. Simular uma falha de enlace

```
> injetar H1 H2
[t=3] Pacote injetado: H1 -> H2
[t=3] Caminho calculado: H1 -> R1 -> R2 -> H2

> falha E2
[t=3] Enlace E2 marcado como falho.
[t=3] Recalculando rotas...
[t=3] Nenhuma rota disponivel. Pacote descartado.

> recuperar E2
[t=3] Enlace E2 restaurado.
```

### 5. Ver métricas e encerrar

```
> metricas

=== Metricas por Enlace ===
E1 | transmitidos: 2 | descartados: 0
E2 | transmitidos: 2 | descartados: 1
E3 | transmitidos: 2 | descartados: 0

=== Totais Globais ===
Pacotes injetados:  2
Pacotes entregues:  1
Pacotes perdidos:   1

> exportar resultados.csv
[OK] Metricas exportadas para resultados.csv

> encerrar
[t=3] Simulacao encerrada.
```

---

## Referência de comandos

| Comando                                      | Quando usar             | Descrição                                     |
|----------------------------------------------|-------------------------|-----------------------------------------------|
| `add_no <tipo> <id>`                         | Antes de `iniciar`      | Adiciona um nó. Tipos: host, roteador, switch |
| `add_enlace <id> <noA> <noB> latencia=<ms>`  | Antes de `iniciar`      | Conecta dois nós com um enlace                |
| `iniciar`                                    | Após montar a topologia | Valida a rede e inicia a simulação            |
| `proximo`                                    | Durante a simulação     | Avança um passo de tempo                      |
| `injetar <origem> <destino>`                 | Durante a simulação     | Injeta um pacote entre dois nós               |
| `falha <idEnlace>`                           | Durante a simulação     | Marca um enlace como falho                    |
| `recuperar <idEnlace>`                       | Durante a simulação     | Restaura um enlace falho                      |
| `metricas`                                   | Qualquer momento        | Exibe métricas no terminal                    |
| `exportar <arquivo.csv>`                     | Qualquer momento        | Exporta métricas para um arquivo CSV          |
| `encerrar`                                   | Qualquer momento        | Encerra a simulação                           |
| `ajuda`                                      | Qualquer momento        | Lista os comandos disponíveis                 |

---

## Estrutura do projeto

```
simulador-rede/
├── include/          # Cabeçalhos (.hpp)
│   ├── No.hpp                    # Classe base No e subclasses Host, Roteador, Switch
│   ├── Enlace.hpp                # Conexão entre dois nós
│   ├── TopologiaDeRede.hpp       # Grafo da rede
│   ├── ProtocoloDeRoteamento.hpp # Classe base abstrata + RoteamentoDijkstra
│   ├── EscalonadorDeEventos.hpp  # Fila de eventos ordenada por tempo
│   ├── ColetorDeMetricas.hpp     # Coleta e exporta métricas
│   └── Simulador.hpp             # Loop principal e leitura de comandos
├── src/              # Implementações (.cpp)
├── docs/doxygen/     # Documentação gerada pelo Doxygen
├── Doxyfile          # Configuração do Doxygen
├── Makefile
└── README.md
```

---

## Gerar documentação

Com o [Doxygen](https://www.doxygen.nl/) instalado:

```bash
doxygen Doxyfile
```

Abra `docs/doxygen/html/index.html` no navegador para navegar pela
documentação de todas as classes e métodos.

---

## Tecnologias

- **Linguagem:** C++17
- **Bibliotecas:** STL apenas (sem dependências externas)
- **Documentação:** Doxygen
- **Build:** Make