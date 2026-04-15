.DEFAULT_GOAL := help
BUILD_DIR     := build
CMAKE_FLAGS   := -G Ninja -DCMAKE_CXX_COMPILER=clang++

.PHONY: help configure build test clean asan tsan lint format

help: ## Показать эту справку
	@awk 'BEGIN {FS = ":.*##"} /^[a-zA-Z_-]+:.*##/ \
		{printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

configure: ## Конфигурация (Debug)
	@cmake -B $(BUILD_DIR) $(CMAKE_FLAGS) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCF_BUILD_TESTS=ON

configure-release: ## Конфигурация (Release)
	@cmake -B $(BUILD_DIR) $(CMAKE_FLAGS) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCF_BUILD_TESTS=OFF

build: configure ## Сборка
	@cmake --build $(BUILD_DIR) --parallel

build-release: configure-release ## Сборка (Release)
	@cmake --build $(BUILD_DIR) --parallel

run: build ## Запуск
	@./$(BUILD_DIR)/cyberforge

test: build ## Запуск тестов
	@cd $(BUILD_DIR) && ctest --output-on-failure --parallel 4

test-verbose: build ## Тесты с подробным выводом
	@./$(BUILD_DIR)/cyberforge_tests --gtest_color=yes

asan: ## Сборка с AddressSanitizer
	@cmake -B $(BUILD_DIR) $(CMAKE_FLAGS) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCF_BUILD_TESTS=ON \
		-DCF_ENABLE_ASAN=ON
	@cmake --build $(BUILD_DIR) --parallel

tsan: ## Сборка с ThreadSanitizer
	@cmake -B $(BUILD_DIR) $(CMAKE_FLAGS) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCF_BUILD_TESTS=ON \
		-DCF_ENABLE_TSAN=ON
	@cmake --build $(BUILD_DIR) --parallel

lint: ## Статический анализ (cppcheck)
	@cppcheck --enable=all --error-exitcode=1 \
		--std=c++23 --suppress=missingInclude \
		engine/ kernel/

format: ## Форматирование (clang-format)
	@find engine kernel tests -name "*.cc" -o -name "*.hh" | \
		xargs clang-format -i --style=file

clean: ## Очистка
	@rm -rf $(BUILD_DIR)