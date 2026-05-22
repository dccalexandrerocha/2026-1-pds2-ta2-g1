 - a formatação em txt ficou legal, mas em markdown ficaria ainda melhor. recomendo ver isso depois.
 - o uso de IDs é legal. espero ver esses IDs nas próximas entregas :)
 - US01: no critério de aceitação vocês falam de uma "área de desenho visual". vocês planejam implementar uma GUI?
 - user stories: temos como atores: Engenheiro de Redes, Pesquisador, Analista de Desempenho, Engenheiro de Confiabilidade, Professor / Coordenador de Projeto, e Estudante de Redes. para que implementar todos esses tipos de usuários diferentes?
 - US01: vocês querem salvar a topologia da rede em dois formatos diferentes: JSON e XML? ótimo para um sistema real, mas trabalhoso demais para este projeto.
 - US02: como vocês planejam controlar o tempo de simulação? a ideia seria um projeto simples, talvez com um "enter" por unidade de tempo.
 - US03: gráficos atualizados a cada 100ms? isso já está se tornando um sistema com multithreading e várias janelas, muito além do esperado para o projeto.
 - US04: critérios de aceitação 1 e 2 são basicamente o mesmo. além disso, vocês estão propondo a implementação de algo muito mais complexo do que o necessário. abrir o critério 1 seria suficiente.
 - US05: trabalhar na geração de bons gráficos para relatórios não é uma tarefa trivial, muito menos gerar um pdf de forma adequada (pesquise sobre latex). gerar um relatório em csv não faz muito sentido.
 - US06: vocês gostariam de implementar e testar 3 algoritmos de roteamento diferentes? novamente, muito interessante, porém acho bem difícil conseguir implementar isso até o fim do semestre.
 - US07: como vocês esperam fazer a comparação entre cenários diferentes? mais uma GUI? além disso, o arquivo de saída será comprimido?
 - crc: várias colaborações para classes que não existem: LogDeEventos, GeradorDeRelatório, GerenciadorDeFalhas, Pacote
 - crc: o sistema proposto nas user stories foi tão complexo que os crc parecem incompletos.
 - notas gerais: o sistema especificado pelas user stories parece ser muito interessante, mas pouco provável de ser implementado até o fim do semestre. já os crc não descrevem detalhes suficientes para a implementação do proposto nas user stories. adicionalmente, não consegui encontrar boas oportunidades de hierarquia no sistema proposto. dado que este documento deveria servir de base para a implementação do próximo checkpoint, acho muito difícil vocês conseguirem avançar de forma significativa no projeto com esta modelagem, com o fim de entregar o projeto em 2 meses. embora a entrega do checkpoint tenha sido considerada, recomendo fortemente que vocês se juntem e refaçam os dois documentos antes de começar a próxima entrega.

(Mateus Pinheiro)-> Obrigado pelo feedback professor! Revisamos as user stories e aplicamos as mudanças sugeridas por você. Realmente, o trabalho ficaria complexo demais para apenas dois meses de prazo. 
A nossa visão do projeto é que tudo seja feito via linha de comando. O programa abre um loop esperando comandos de texto, do tipo:
> add_no roteador R1
> add_no roteador R2
> add_no host H1
> add_enlace R1 R2 banda=100 latencia=10
> add_enlace H1 R1 banda=50 latencia=5
> iniciar
> injetar H1 R2
> proximo
> falha R1 R2
> metricas
> encerrar
Quando um pacote é injetado, o algoritmo calcula o menor caminho na topologia e o pacote percorre esse caminho hop a hop, com o terminal exibindo cada etapa. Se um enlace for derrubado no meio da simulação, as rotas são recalculadas e o pacote é descartado caso não haja caminho disponível.
Ao final, o simulador exibe métricas no terminal (pacotes entregues, descartados, ocupação dos enlaces) e grava um arquivo CSV com os dados coletados. A topologia pode ser salva e carregada em JSON.
O projeto não tem janela gráfica, mouse, gráficos dinâmicos nem bibliotecas externas além da STL do C++. É essencialmente um interpretador de comandos com simulação de gráficoss, com orientação a objetos aplicada nas classes de rede e no uso de herança para os tipos de nó e para o protocolo de roteamento.
Conto com o seu retorno nos próximos checkpoints! Valeu!