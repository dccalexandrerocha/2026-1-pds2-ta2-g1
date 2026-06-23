CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude

# Flags de cobertura (usadas apenas no target test)
COV_FLAGS := --coverage -fprofile-arcs -ftest-coverage -O0

ifeq ($(OS),Windows_NT)
    GCOVR := python -m gcovr
else
    GCOVR := gcovr
endif

# Arquivos-fonte (todos exceto main.cpp)
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

# Detecta o sistema operacional
ifeq ($(OS),Windows_NT)
    MKDIR   = if not exist "build\tests" mkdir "build\tests"
    MKDIR  += & if not exist "coverage" mkdir "coverage"
    RMDIR   = if exist "build" rmdir /s /q "build"
    RMDIR  += & if exist "coverage" rmdir /s /q "coverage"
    RM      = del /f /q
    RUN     = $(TEST_TARGET)
    FIND_DEL = (for %%f in (build\tests\*.gcda) do del /f /q "%%f") 2>nul & exit /b 0
else
    MKDIR   = mkdir -p build/tests coverage
    RMDIR   = rm -rf build/ coverage/
    RM      = rm -f
    RUN     = ./$(TEST_TARGET)
    FIND_DEL = find build/tests -name "*.gcda" -delete 2>/dev/null; true
endif

# ─── Cria diretorios necessarios ──────────────────────────────────────────────
$(shell $(MKDIR))

# ─── Target padrao: compila o simulador ───────────────────────────────────────
all: $(TARGET)

$(TARGET): $(SRCS) src/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

# ─── Testes com cobertura ─────────────────────────────────────────────────────
test: $(TEST_TARGET)
	@$(FIND_DEL)
	@echo ""
	@echo "========================================"
	@echo "  Executando testes de unidade..."
	@echo "========================================"
	$(RUN) --duration=true
	@echo ""
	@echo "========================================"
	@echo "  Gerando relatorio de cobertura..."
	@echo "========================================"
	$(GCOVR) --root . \
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
	$(RMDIR)
	$(RM) $(TARGET)

.PHONY: all test clean