CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude

# Flags de cobertura (usadas apenas no target test)
COV_FLAGS := --coverage -fprofile-arcs -ftest-coverage -O0

# Arquivos-fonte (todos exceto main.cpp, que nao entra nos testes)
SRCS := src/No.cpp \
        src/Enlace.cpp \
        src/ColetorDeMetricas.cpp \
        src/EscalonadorDeEventos.cpp \
        src/TopologiaDeRede.cpp \
        src/ProtocoloDeRoteamento.cpp \
        src/Simulador.cpp

# Executavel principal
TARGET := simulador

# Executavel de testes
TEST_TARGET := build/tests/teste_runner

# ─── Cria diretorios necessarios ──────────────────────────────────────────────
$(shell mkdir -p build/tests coverage)

# ─── Target padrao: compila o simulador ───────────────────────────────────────
all: $(TARGET)

$(TARGET): $(SRCS) src/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

# ─── Testes com cobertura ─────────────────────────────────────────────────────
test: $(TEST_TARGET)
	@find build/tests -name "*.gcda" -delete 2>/dev/null; true
	@echo ""
	@echo "========================================"
	@echo "  Executando testes de unidade..."
	@echo "========================================"
	./$(TEST_TARGET) --duration=true
	@echo ""
	@echo "========================================"
	@echo "  Gerando relatorio de cobertura..."
	@echo "========================================"
	gcovr --root . \
	      --exclude tests/ \
	      --exclude src/main.cpp \
	      --print-summary \
	      --html-details coverage/index.html \
	      --xml coverage/coverage.xml \
	      --object-directory build/tests
	@echo ""
	@echo "Relatorio HTML gerado em: coverage/index.html"

$(TEST_TARGET): $(SRCS) tests/testes.cpp
	$(CXX) $(CXXFLAGS) $(COV_FLAGS) \
	    -Itests \
	    -o $@ \
	    $(SRCS) tests/testes.cpp

# ─── Limpeza ──────────────────────────────────────────────────────────────────
clean:
	rm -f $(TARGET)
	rm -f $(TEST_TARGET)
	rm -f build/tests/*.gcda build/tests/*.gcno
	rm -f src/*.gcda src/*.gcno
	rm -rf coverage/

.PHONY: all test clean
