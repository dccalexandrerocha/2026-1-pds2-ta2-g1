# SIMULADOR DE REDE E TELECOMUNICACAO — USER STORIES

---

## US01 — Configuração de Topologia de Rede

**Sistema:** Simulador de Rede e Telecomunicacao

**Descrição:**
Como Usuário, quero definir a topologia de rede via terminal para configurar os
nós e enlaces do experimento antes de iniciar a simulação.

**Classes relacionadas:** `TopologiaDeRede`, `No`, `Enlace`

**Critérios de aceitação:**

1. Permitir adicionar nós de tipos diferentes (host, roteador, switch), cada um
   identificado por um ID único e um endereço IP, via comandos de texto.
2. Permitir conectar dois nós com um enlace, informando largura de banda e
   latência base.
3. Validar a conectividade da topologia antes de iniciar a simulação, listando
   no terminal quais nós estão isolados, caso existam.
4. Permitir salvar e carregar a topologia em formato JSON.

---

## US02 — Execução e Controle da Simulação

**Sistema:** Simulador de Rede e Telecomunicacao

**Descrição:**
Como Usuário, quero controlar o ciclo de vida da simulação para observar a
evolução da rede passo a passo, sem precisar reiniciá-la do zero a cada
consulta.

**Classes relacionadas:** `Simulador`, `EscalonadorDeEventos`

**Critérios de aceitação:**

1. Permitir iniciar, pausar, retomar e encerrar a simulação por comandos de
   texto no terminal.
2. Avançar a simulação um passo de tempo por vez (pressionando Enter) ou em
   modo contínuo até o encerramento.
3. Exibir no terminal o estado atual da simulação (parado, em execução, pausado,
   encerrado) e o instante de tempo global a cada passo.
4. Registrar em arquivo de texto um log sequencial dos eventos ocorridos, com o
   instante de tempo de cada um.

---

## US03 — Transmissão de Pacotes e Roteamento

**Sistema:** Simulador de Rede e Telecomunicacao

**Descrição:**
Como Usuário, quero injetar pacotes na rede simulada e observar seu
encaminhamento pelos nós para verificar se o algoritmo de roteamento está
funcionando corretamente.

**Classes relacionadas:** `No`, `Enlace`, `ProtocoloDeRoteamento`,
`TopologiaDeRede`

**Critérios de aceitação:**

1. Permitir injetar um pacote em um nó de origem, especificando o nó de
   destino, via comando de texto.
2. Exibir no terminal o caminho percorrido pelo pacote (sequência de nós e
   enlaces), hop a hop.
3. Calcular a rota pelo algoritmo de Dijkstra, usando os pesos dos enlaces como
   métrica.
4. Descartar o pacote e exibir mensagem informativa quando o enlace no caminho
   estiver no estado falho.
5. Registrar no log cada evento de transmissão, encaminhamento e descarte.

---

## US04 — Simulação de Falhas em Enlace

**Sistema:** Simulador de Rede e Telecomunicacao

**Descrição:**
Como Usuário, quero marcar um enlace como falho durante a simulação para
observar como o roteamento se adapta à indisponibilidade daquele caminho.

**Classes relacionadas:** `Enlace`, `EscalonadorDeEventos`,
`ProtocoloDeRoteamento`

**Critérios de aceitação:**

1. Permitir alterar o estado de um enlace para falho ou ativo via comando de
   texto, em qualquer instante da simulação.
2. Recalcular automaticamente as rotas afetadas após a mudança de estado do
   enlace.
3. Registrar no log o instante em que a falha foi introduzida e, se aplicável,
   em que o enlace foi recuperado.
4. Agendar a recuperação automática de um enlace após N passos de tempo,
   inserindo o evento na fila do EscalonadorDeEventos.

---

## US05 — Coleta e Exibição de Métricas

**Sistema:** Simulador de Rede e Telecomunicacao

**Descrição:**
Como Usuário, quero consultar métricas de desempenho ao fim da simulação para
avaliar o comportamento da rede no experimento realizado.

**Classes relacionadas:** `ColetorDeMetricas`, `Enlace`, `No`

**Critérios de aceitação:**

1. Exibir no terminal, ao encerrar a simulação ou sob comando do usuário, as
   métricas por enlace: total de pacotes transmitidos, descartados e taxa de
   utilização média.
2. Exibir métricas globais: total de pacotes injetados, entregues e perdidos na
   rede.
3. Calcular e exibir a ocupação média e máxima por enlace ao longo da
   simulação.
4. Exportar as métricas coletadas em arquivo CSV, com uma linha por enlace ou
   nó.

---

## US06 — Visualização da Tabela de Roteamento

**Sistema:** Simulador de Rede e Telecomunicacao

**Descrição:**
Como Usuário, quero consultar a tabela de roteamento de um nó específico para
verificar quais são os próximos saltos calculados pelo algoritmo de Dijkstra
para cada destino na rede.

**Classes relacionadas:** `ProtocoloDeRoteamento`, `TopologiaDeRede`

**Critérios de aceitação:**

1. Permitir exibir a tabela de roteamento de um nó informado via comando de
   texto no terminal.
2. Mostrar, para cada destino conhecido, o próximo salto (next hop) calculado
   pelo Dijkstra.
3. Exibir mensagem informativa caso o nó não possua tabela calculada ainda.
4. Atualizar a tabela automaticamente após qualquer alteração de estado de
   enlace (falha ou recuperação).

*Total: 6 User Stories — Simulador de Rede e Telecomunicacao*
