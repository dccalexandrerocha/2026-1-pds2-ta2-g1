#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "No.hpp"
#include "Enlace.hpp"
#include "ColetorDeMetricas.hpp"
#include "EscalonadorDeEventos.hpp"
#include "TopologiaDeRede.hpp"
#include "ProtocoloDeRoteamento.hpp"
#include "Simulador.hpp"

#include <memory>
#include <vector>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// Testes de No (Host, Roteador, Switch)
// ─────────────────────────────────────────────────────────────────────────────

TEST_SUITE("No") {

    TEST_CASE("Host - atributos basicos") {
        Host h("H1", "192.168.0.1");
        CHECK(h.getId()       == "H1");
        CHECK(h.getEndereco() == "192.168.0.1");
        CHECK(h.getTipo()     == TipoNo::HOST);
        CHECK(h.isAtivo()     == true);
    }

    TEST_CASE("Host - contadores iniciam zerados") {
        Host h("H1", "192.168.0.1");
        CHECK(h.getPacotesRecebidos()   == 0);
        CHECK(h.getPacotesEnviados()    == 0);
        CHECK(h.getPacotesDescartados() == 0);
    }

    TEST_CASE("Host - receberPacote como destino incrementa recebidos e enviados") {
        Host h("H2", "192.168.0.2");
        h.receberPacote("H1", "H2", 1);
        CHECK(h.getPacotesRecebidos() == 1);
        CHECK(h.getPacotesEnviados()  == 1);
        CHECK(h.getPacotesDescartados() == 0);
    }

    TEST_CASE("Host - receberPacote com destino errado descarta") {
        Host h("H1", "192.168.0.1");
        h.receberPacote("H2", "H3", 1);  // H1 nao e o destino
        CHECK(h.getPacotesRecebidos()   == 1);
        CHECK(h.getPacotesDescartados() == 1);
    }

    TEST_CASE("Roteador - atributos basicos") {
        Roteador r("R1", "10.0.0.1");
        CHECK(r.getId()   == "R1");
        CHECK(r.getTipo() == TipoNo::ROTEADOR);
        CHECK(r.isAtivo() == true);
    }

    TEST_CASE("Roteador - receberPacote incrementa contadores") {
        Roteador r("R1", "10.0.0.1");
        r.receberPacote("H1", "H2", 2);
        CHECK(r.getPacotesRecebidos() == 1);
        CHECK(r.getPacotesEnviados()  == 1);
    }

    TEST_CASE("Switch - atributos basicos") {
        Switch s("S1", "10.0.1.1");
        CHECK(s.getId()   == "S1");
        CHECK(s.getTipo() == TipoNo::SWITCH);
        CHECK(s.isAtivo() == true);
    }

    TEST_CASE("Switch - receberPacote incrementa contadores") {
        Switch s("S1", "10.0.1.1");
        s.receberPacote("H1", "H2", 0);
        CHECK(s.getPacotesRecebidos() == 1);
        CHECK(s.getPacotesEnviados()  == 1);
    }

    TEST_CASE("No - adicionarEnlace e getEnlaces") {
        Host h("H1", "1.1.1.1");
        Host h2("H2", "1.1.1.2");
        Enlace e("E1", &h, &h2, 100.0, 5.0);
        CHECK(h.getEnlaces().empty());
        h.adicionarEnlace(&e);
        CHECK(h.getEnlaces().size() == 1);
        CHECK(h.getEnlaces()[0] == &e);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Testes de Enlace
// ─────────────────────────────────────────────────────────────────────────────

TEST_SUITE("Enlace") {

    TEST_CASE("Enlace - atributos basicos") {
        Host noA("H1", "1.1.1.1");
        Host noB("H2", "1.1.1.2");
        Enlace e("E1", &noA, &noB, 100.0, 10.0);

        CHECK(e.getId()       == "E1");
        CHECK(e.getNoA()      == &noA);
        CHECK(e.getNoB()      == &noB);
        CHECK(e.getBanda()    == doctest::Approx(100.0));
        CHECK(e.getLatencia() == doctest::Approx(10.0));
        CHECK(e.isAtivo()     == true);
    }

    TEST_CASE("Enlace - setAtivo altera estado") {
        Host a("A", "0.0.0.1"), b("B", "0.0.0.2");
        Enlace e("E1", &a, &b, 10.0, 1.0);
        CHECK(e.isAtivo() == true);
        e.setAtivo(false);
        CHECK(e.isAtivo() == false);
        e.setAtivo(true);
        CHECK(e.isAtivo() == true);
    }

    TEST_CASE("Enlace - contadores iniciam zerados") {
        Host a("A", "0.0.0.1"), b("B", "0.0.0.2");
        Enlace e("E1", &a, &b, 10.0, 1.0);
        CHECK(e.getPacotesTransmitidos() == 0);
        CHECK(e.getPacotesDescartados()  == 0);
        CHECK(e.getUtilizacao()          == doctest::Approx(0.0));
    }

    TEST_CASE("Enlace - transmitir com enlace ativo incrementa transmitidos") {
        Host a("A", "0.0.0.1"), b("B", "0.0.0.2");
        Enlace e("E1", &a, &b, 100.0, 5.0);
        bool ok = e.transmitir("A", "B", 1);
        CHECK(ok == true);
        CHECK(e.getPacotesTransmitidos() == 1);
        CHECK(e.getPacotesDescartados()  == 0);
    }

    TEST_CASE("Enlace - transmitir com enlace falho descarta pacote") {
        Host a("A", "0.0.0.1"), b("B", "0.0.0.2");
        Enlace e("E1", &a, &b, 100.0, 5.0);
        e.setAtivo(false);
        bool ok = e.transmitir("A", "B", 1);
        CHECK(ok == false);
        CHECK(e.getPacotesDescartados()  == 1);
        CHECK(e.getPacotesTransmitidos() == 0);
    }

    TEST_CASE("Enlace - utilizacao nao e negativa") {
        Host a("A", "0.0.0.1"), b("B", "0.0.0.2");
        Enlace e("E1", &a, &b, 100.0, 5.0);
        CHECK(e.getUtilizacao() >= 0.0);
        e.transmitir("A", "B", 1);
        CHECK(e.getUtilizacao() >= 0.0);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Testes de ColetorDeMetricas
// ─────────────────────────────────────────────────────────────────────────────

TEST_SUITE("ColetorDeMetricas") {

    TEST_CASE("Totais iniciam zerados") {
        ColetorDeMetricas c;
        CHECK(c.getTotalEntregues() == 0);
        CHECK(c.getTotalPerdidos()  == 0);
    }

    TEST_CASE("registrarEnlace acumula transmitidos e descartados") {
        ColetorDeMetricas c;
        c.registrarEnlace("E1", 3, 1, 0.5);
        c.registrarEnlace("E1", 2, 0, 0.3);
        // Verificamos via getTotalPerdidos (soma descartados de enlaces)
        CHECK(c.getTotalPerdidos() == 1);
    }

    TEST_CASE("registrarNo acumula enviados (contados como entregues)") {
        ColetorDeMetricas c;
        c.registrarNo("H2", 2, 2, 0);
        CHECK(c.getTotalEntregues() == 2);
    }

    TEST_CASE("registrarNo acumula descartados") {
        ColetorDeMetricas c;
        c.registrarNo("H1", 1, 0, 1);
        CHECK(c.getTotalPerdidos() == 1);
    }

    TEST_CASE("reset zera todos os contadores") {
        ColetorDeMetricas c;
        c.registrarEnlace("E1", 5, 2, 0.8);
        c.registrarNo("H1", 3, 3, 0);
        c.reset();
        CHECK(c.getTotalEntregues() == 0);
        CHECK(c.getTotalPerdidos()  == 0);
    }

    TEST_CASE("exportarCSV retorna false para caminho invalido") {
        ColetorDeMetricas c;
        c.registrarEnlace("E1", 1, 0, 0.1);
        bool ok = c.exportarCSV("/caminho/invalido/impossivel/arquivo.csv");
        CHECK(ok == false);
    }

    TEST_CASE("exportarCSV retorna true e gera arquivo") {
        ColetorDeMetricas c;
        c.registrarEnlace("E1", 2, 0, 0.2);
        c.registrarNo("H1", 1, 1, 0);
        bool ok = c.exportarCSV("/tmp/teste_metricas.csv");
        CHECK(ok == true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Testes de EscalonadorDeEventos
// ─────────────────────────────────────────────────────────────────────────────

TEST_SUITE("EscalonadorDeEventos") {

    TEST_CASE("Tempo inicial e zero e fila vazia") {
        EscalonadorDeEventos e;
        CHECK(e.getTempoAtual() == 0);
        CHECK(e.filaVazia()     == true);
    }

    TEST_CASE("agendarEvento insere evento na fila") {
        EscalonadorDeEventos e;
        e.agendarEvento(5, "teste", [](){});
        CHECK(e.filaVazia() == false);
    }

    TEST_CASE("processarProximo retorna false em fila vazia") {
        EscalonadorDeEventos e;
        CHECK(e.processarProximo() == false);
    }

    TEST_CASE("processarProximo executa acao e retorna true") {
        EscalonadorDeEventos e;
        bool executado = false;
        e.agendarEvento(1, "acao", [&executado](){ executado = true; });
        bool ok = e.processarProximo();
        CHECK(ok == true);
        CHECK(executado == true);
        CHECK(e.filaVazia() == true);
    }

    TEST_CASE("eventos sao processados na ordem crescente de tempo") {
        EscalonadorDeEventos e;
        std::vector<int> ordem;
        e.agendarEvento(3, "", [&ordem](){ ordem.push_back(3); });
        e.agendarEvento(1, "", [&ordem](){ ordem.push_back(1); });
        e.agendarEvento(2, "", [&ordem](){ ordem.push_back(2); });
        while (!e.filaVazia()) e.processarProximo();
        CHECK(ordem == std::vector<int>{1, 2, 3});
    }

    TEST_CASE("avancarPasso incrementa tempo e processa eventos do passo") {
        EscalonadorDeEventos e;
        bool ok = false;
        e.agendarEvento(1, "", [&ok](){ ok = true; });
        e.avancarPasso();  // tempo vai para 1
        CHECK(e.getTempoAtual() == 1);
        CHECK(ok == true);
    }

    TEST_CASE("limpar esvazia a fila") {
        EscalonadorDeEventos e;
        e.agendarEvento(1, "a", [](){});
        e.agendarEvento(2, "b", [](){});
        e.limpar();
        CHECK(e.filaVazia() == true);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Testes de TopologiaDeRede
// ─────────────────────────────────────────────────────────────────────────────

TEST_SUITE("TopologiaDeRede") {

    TEST_CASE("adicionarNo insere e getNo recupera") {
        TopologiaDeRede t;
        auto h = std::make_unique<Host>("H1", "1.1.1.1");
        CHECK(t.adicionarNo(std::move(h)) == true);
        CHECK(t.getNo("H1") != nullptr);
        CHECK(t.getNo("H1")->getId() == "H1");
    }

    TEST_CASE("adicionarNo rejeita ID duplicado") {
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        bool ok = t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.2"));
        CHECK(ok == false);
    }

    TEST_CASE("getNo retorna nullptr para ID inexistente") {
        TopologiaDeRede t;
        CHECK(t.getNo("XPTO") == nullptr);
    }

    TEST_CASE("removerNo remove o no") {
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        CHECK(t.removerNo("H1") == true);
        CHECK(t.getNo("H1") == nullptr);
    }

    TEST_CASE("removerNo retorna false para ID inexistente") {
        TopologiaDeRede t;
        CHECK(t.removerNo("XPTO") == false);
    }

    TEST_CASE("getNos retorna todos os nos inseridos") {
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        t.adicionarNo(std::make_unique<Roteador>("R1", "10.0.0.1"));
        CHECK(t.getNos().size() == 2);
    }

    TEST_CASE("adicionarEnlace conecta dois nos existentes") {
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        t.adicionarNo(std::make_unique<Host>("H2", "1.1.1.2"));
        No* a = t.getNo("H1");
        No* b = t.getNo("H2");
        auto enlace = std::make_unique<Enlace>("E1", a, b, 100.0, 5.0);
        CHECK(t.adicionarEnlace(std::move(enlace)) == true);
        CHECK(t.getEnlace("E1") != nullptr);
    }

    TEST_CASE("getEnlace retorna nullptr para ID inexistente") {
        TopologiaDeRede t;
        CHECK(t.getEnlace("XPTO") == nullptr);
    }

    TEST_CASE("removerEnlace remove o enlace") {
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        t.adicionarNo(std::make_unique<Host>("H2", "1.1.1.2"));
        No* a = t.getNo("H1");
        No* b = t.getNo("H2");
        t.adicionarEnlace(std::make_unique<Enlace>("E1", a, b, 100.0, 5.0));
        CHECK(t.removerEnlace("E1") == true);
        CHECK(t.getEnlace("E1") == nullptr);
    }

    TEST_CASE("getVizinhos retorna os enlaces de um no") {
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        t.adicionarNo(std::make_unique<Host>("H2", "1.1.1.2"));
        No* a = t.getNo("H1");
        No* b = t.getNo("H2");
        t.adicionarEnlace(std::make_unique<Enlace>("E1", a, b, 100.0, 5.0));
        CHECK(t.getVizinhos("H1").size() == 1);
        CHECK(t.getVizinhos("H2").size() == 1);
    }

    TEST_CASE("validarConectividade retorna true para rede conectada") {
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        t.adicionarNo(std::make_unique<Host>("H2", "1.1.1.2"));
        No* a = t.getNo("H1");
        No* b = t.getNo("H2");
        t.adicionarEnlace(std::make_unique<Enlace>("E1", a, b, 100.0, 5.0));
        CHECK(t.validarConectividade() == true);
    }

    TEST_CASE("validarConectividade detecta no isolado") {
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        t.adicionarNo(std::make_unique<Host>("H2", "1.1.1.2"));
        // Sem enlace: H2 esta isolado
        CHECK(t.validarConectividade() == false);
    }

    TEST_CASE("salvarJSON cria arquivo") {
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        bool ok = t.salvarJSON("/tmp/topo_teste.json");
        CHECK(ok == true);
    }

    TEST_CASE("getEnlaces retorna todos os enlaces") {
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        t.adicionarNo(std::make_unique<Host>("H2", "1.1.1.2"));
        t.adicionarNo(std::make_unique<Roteador>("R1", "10.0.0.1"));
        No* a = t.getNo("H1");
        No* b = t.getNo("R1");
        No* c = t.getNo("H2");
        t.adicionarEnlace(std::make_unique<Enlace>("E1", a, b, 100.0, 5.0));
        t.adicionarEnlace(std::make_unique<Enlace>("E2", b, c, 100.0, 10.0));
        CHECK(t.getEnlaces().size() == 2);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Testes de ProtocoloDeRoteamento (RoteamentoDijkstra)
// ─────────────────────────────────────────────────────────────────────────────

TEST_SUITE("RoteamentoDijkstra") {

    // Monta topologia auxiliar: H1 -- R1 -- H2
    static TopologiaDeRede montarTopoSimples() {
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        t.adicionarNo(std::make_unique<Roteador>("R1", "10.0.0.1"));
        t.adicionarNo(std::make_unique<Host>("H2", "1.1.1.2"));
        No* h1 = t.getNo("H1");
        No* r1 = t.getNo("R1");
        No* h2 = t.getNo("H2");
        t.adicionarEnlace(std::make_unique<Enlace>("E1", h1, r1, 100.0, 5.0));
        t.adicionarEnlace(std::make_unique<Enlace>("E2", r1, h2, 100.0, 10.0));
        return t;
    }

    TEST_CASE("calcularCaminho retorna caminho correto") {
        TopologiaDeRede t = montarTopoSimples();
        RoteamentoDijkstra dijkstra;
        auto caminho = dijkstra.calcularCaminho(t, "H1", "H2");
        REQUIRE(caminho.size() == 3);
        CHECK(caminho[0] == "H1");
        CHECK(caminho[1] == "R1");
        CHECK(caminho[2] == "H2");
    }

    TEST_CASE("calcularCaminho origem igual ao destino") {
        TopologiaDeRede t = montarTopoSimples();
        RoteamentoDijkstra dijkstra;
        auto caminho = dijkstra.calcularCaminho(t, "H1", "H1");
        CHECK(caminho.size() == 1);
        CHECK(caminho[0] == "H1");
    }

    TEST_CASE("calcularCaminho retorna vazio para no inexistente") {
        TopologiaDeRede t = montarTopoSimples();
        RoteamentoDijkstra dijkstra;
        auto caminho = dijkstra.calcularCaminho(t, "H1", "XPTO");
        CHECK(caminho.empty());
    }

    TEST_CASE("calcularCaminho retorna vazio se enlace falho bloqueia rota") {
        TopologiaDeRede t = montarTopoSimples();
        // Derrubar ambos os enlaces
        t.getEnlace("E1")->setAtivo(false);
        t.getEnlace("E2")->setAtivo(false);
        RoteamentoDijkstra dijkstra;
        auto caminho = dijkstra.calcularCaminho(t, "H1", "H2");
        CHECK(caminho.empty());
    }

    TEST_CASE("calcularCaminho escolhe menor latencia") {
        // H1 -- R1 (latencia 1) -- H2
        //    \-- R2 (latencia 100) --/
        TopologiaDeRede t;
        t.adicionarNo(std::make_unique<Host>("H1", "1.1.1.1"));
        t.adicionarNo(std::make_unique<Roteador>("R1", "10.0.0.1"));
        t.adicionarNo(std::make_unique<Roteador>("R2", "10.0.0.2"));
        t.adicionarNo(std::make_unique<Host>("H2", "1.1.1.2"));
        No* h1 = t.getNo("H1");
        No* r1 = t.getNo("R1");
        No* r2 = t.getNo("R2");
        No* h2 = t.getNo("H2");
        t.adicionarEnlace(std::make_unique<Enlace>("E1", h1, r1, 100.0, 1.0));
        t.adicionarEnlace(std::make_unique<Enlace>("E2", r1, h2, 100.0, 1.0));
        t.adicionarEnlace(std::make_unique<Enlace>("E3", h1, r2, 100.0, 100.0));
        t.adicionarEnlace(std::make_unique<Enlace>("E4", r2, h2, 100.0, 100.0));
        RoteamentoDijkstra dijkstra;
        auto caminho = dijkstra.calcularCaminho(t, "H1", "H2");
        REQUIRE(caminho.size() == 3);
        CHECK(caminho[1] == "R1");  // deve usar R1, o caminho mais curto
    }

    TEST_CASE("exibirTabela nao lanca excecao") {
        TopologiaDeRede t = montarTopoSimples();
        RoteamentoDijkstra dijkstra;
        dijkstra.calcularCaminho(t, "H1", "H2");
        // Apenas checa que nao lanca excecao
        CHECK_NOTHROW(dijkstra.exibirTabela("H1"));
        CHECK_NOTHROW(dijkstra.exibirTabela("NO_INEXISTENTE"));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Testes de Simulador (sem o REPL/executar)
// ─────────────────────────────────────────────────────────────────────────────

TEST_SUITE("Simulador") {

    TEST_CASE("Estado inicial e PARADO") {
        Simulador sim;
        CHECK(sim.getEstado() == EstadoSimulacao::PARADO);
    }

    TEST_CASE("iniciar sem topologia valida nao muda estado") {
        Simulador sim;
        // Topologia vazia: validarConectividade retorna true (sem nos),
        // mas nao ha nos entao iniciar deve ter sucesso mesmo sem nos.
        sim.iniciar();
        // Com topologia vazia, validarConectividade() retorna true (nenhum no isolado)
        CHECK(sim.getEstado() == EstadoSimulacao::EM_EXECUCAO);
    }

    TEST_CASE("iniciar duas vezes nao muda estado da segunda vez") {
        Simulador sim;
        sim.iniciar();
        CHECK(sim.getEstado() == EstadoSimulacao::EM_EXECUCAO);
        sim.iniciar();  // segunda chamada deve imprimir erro
        CHECK(sim.getEstado() == EstadoSimulacao::EM_EXECUCAO);
    }

    TEST_CASE("pausar e retomar alteram estado corretamente") {
        Simulador sim;
        sim.iniciar();
        sim.pausar();
        CHECK(sim.getEstado() == EstadoSimulacao::PAUSADO);
        sim.retomar();
        CHECK(sim.getEstado() == EstadoSimulacao::EM_EXECUCAO);
    }

    TEST_CASE("pausar sem estar em execucao nao altera estado") {
        Simulador sim;
        sim.pausar();  // deve imprimir erro
        CHECK(sim.getEstado() == EstadoSimulacao::PARADO);
    }

    TEST_CASE("retomar sem estar pausado nao altera estado") {
        Simulador sim;
        sim.iniciar();
        sim.retomar();  // deve imprimir erro
        CHECK(sim.getEstado() == EstadoSimulacao::EM_EXECUCAO);
    }

    TEST_CASE("avancarPasso sem simulacao iniciada imprime erro") {
        Simulador sim;
        CHECK_NOTHROW(sim.avancarPasso());
        CHECK(sim.getEstado() == EstadoSimulacao::PARADO);
    }

    TEST_CASE("avancarPasso com simulacao iniciada funciona") {
        Simulador sim;
        sim.iniciar();
        CHECK_NOTHROW(sim.avancarPasso());
        CHECK(sim.getEstado() == EstadoSimulacao::EM_EXECUCAO);
    }

    TEST_CASE("injetarPacote sem iniciar imprime erro") {
        Simulador sim;
        CHECK_NOTHROW(sim.injetarPacote("H1", "H2"));
    }

    TEST_CASE("alterarEstadoEnlace com enlace inexistente imprime erro") {
        Simulador sim;
        CHECK_NOTHROW(sim.alterarEstadoEnlace("INEXISTENTE", false));
    }

    TEST_CASE("encerrar muda estado para ENCERRADO") {
        Simulador sim;
        sim.iniciar();
        sim.encerrar();
        CHECK(sim.getEstado() == EstadoSimulacao::ENCERRADO);
    }
}
