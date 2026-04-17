.DEFAULT_GOAL := help
BUILD_DIR     := build
CMAKE_FLAGS   := -G Ninja -DCMAKE_CXX_COMPILER=clang++

# Цвета
GREEN  := \033[92m
YELLOW := \033[93m
CYAN   := \033[96m
RED    := \033[91m
RESET  := \033[0m

.PHONY: help configure build build-release run test test-verbose \
        asan tsan lint lint-fix format format-check clean \
        deps check-deps

help: ## Справка
	@echo ""
	@echo "  $(CYAN)CyberForge Framework$(RESET) — доступные команды:"
	@echo ""
	@awk 'BEGIN {FS = ":.*##"} /^[a-zA-Z_-]+:.*##/ \
		{printf "  $(GREEN)%-18s$(RESET) %s\n", $$1, $$2}' $(MAKEFILE_LIST)
	@echo ""


configure: ## Конфигурация Debug
	@cmake -B $(BUILD_DIR) $(CMAKE_FLAGS) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCF_BUILD_TESTS=ON \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

configure-release: ## Конфигурация Release
	@cmake -B $(BUILD_DIR) $(CMAKE_FLAGS) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCF_BUILD_TESTS=OFF \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

build: configure ## Сборка (Debug)
	@cmake --build $(BUILD_DIR) --parallel
	@echo "$(GREEN)[✓] Build complete$(RESET)"

build-release: configure-release ## Сборка (Release)
	@cmake --build $(BUILD_DIR) --parallel
	@echo "$(GREEN)[✓] Release build complete$(RESET)"

run: build ## Запуск
	@./$(BUILD_DIR)/cyberforge


test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure --parallel 4
	@echo "$(GREEN)[✓] Tests passed$(RESET)"

test-verbose: build
	@./$(BUILD_DIR)/cyberforge_tests \
		--gtest_color=yes \
		--gtest_print_time=1

test-filter: build
	@./$(BUILD_DIR)/cyberforge_tests \
		--gtest_filter=$(F) \
		--gtest_color=yes

asan: ## AddressSanitizer + UBSan
	@cmake -B $(BUILD_DIR) $(CMAKE_FLAGS) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCF_BUILD_TESTS=ON \
		-DCF_ENABLE_ASAN=ON
	@cmake --build $(BUILD_DIR) --parallel
	@cd $(BUILD_DIR) && ctest --output-on-failure

tsan: ## ThreadSanitizer
	@cmake -B $(BUILD_DIR) $(CMAKE_FLAGS) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCF_BUILD_TESTS=ON \
		-DCF_ENABLE_TSAN=ON
	@cmake --build $(BUILD_DIR) --parallel
	@cd $(BUILD_DIR) && ctest --output-on-failure

lint: build ## clang-tidy (статический анализ)
	@echo "$(YELLOW)[*] Running clang-tidy...$(RESET)"
	@find engine kernel -name "*.cc" | \
		xargs clang-tidy \
			-p $(BUILD_DIR)/compile_commands.json \
			--config-file=.clang-tidy \
			--warnings-as-errors='bugprone-*'
	@echo "$(GREEN)[✓] clang-tidy passed$(RESET)"

lint-fix: build ## clang-tidy с авто-исправлением
	@find engine kernel -name "*.cc" | \
		xargs clang-tidy \
			-p $(BUILD_DIR)/compile_commands.json \
			--fix \
			--fix-errors
	@echo "$(GREEN)[✓] clang-tidy fixes applied$(RESET)"

format: ## Форматировать весь код
	@echo "$(YELLOW)[*] Running clang-format...$(RESET)"
	@find engine kernel tests -name "*.cc" -o -name "*.hh" | \
		xargs clang-format -i --style=file
	@echo "$(GREEN)[✓] Code formatted$(RESET)"

format-check: ## Проверить форматирование (без изменений)
	@echo "$(YELLOW)[*] Checking format...$(RESET)"
	@find engine kernel tests -name "*.cc" -o -name "*.hh" | \
		xargs clang-format --dry-run --Werror --style=file
	@echo "$(GREEN)[✓] Format OK$(RESET)"

deps: ## Установить зависимости (Ubuntu/Debian)
	@echo "$(YELLOW)[*] Installing dependencies...$(RESET)"
	@sudo apt-get update && sudo apt-get install -y \
		cmake ninja-build \
		libboost-all-dev \
		clang-18 \
		clang-tidy-18 \
		clang-format-18 \
		clangd-18 \
		cppcheck \
		gcovr
	@echo "$(GREEN)[✓] Dependencies installed$(RESET)"

check-deps: ## Проверить наличие инструментов
	@echo "$(YELLOW)[*] Checking tools...$(RESET)"
	@which cmake       && echo "  $(GREEN)✓ cmake$(RESET)"       || echo "  $(RED)✗ cmake$(RESET)"
	@which ninja       && echo "  $(GREEN)✓ ninja$(RESET)"       || echo "  $(RED)✗ ninja$(RESET)"
	@which clang++     && echo "  $(GREEN)✓ clang++$(RESET)"     || echo "  $(RED)✗ clang++$(RESET)"
	@which clang-tidy  && echo "  $(GREEN)✓ clang-tidy$(RESET)"  || echo "  $(RED)✗ clang-tidy$(RESET)"
	@which clang-format && echo "  $(GREEN)✓ clang-format$(RESET)" || echo "  $(RED)✗ clang-format$(RESET)"
	@which clangd      && echo "  $(GREEN)✓ clangd$(RESET)"      || echo "  $(RED)✗ clangd$(RESET)"


clean:
	@rm -rf $(BUILD_DIR)
	@echo "$(GREEN)[✓] Cleaned$(RESET)"