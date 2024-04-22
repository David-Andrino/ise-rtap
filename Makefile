SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -Command 

.PHONY: all format lint

all: format lint

format: 
	@Get-ChildItem -Path . -Recurse -File -Include *.c,*.h | Where-Object { $$_.FullName -notlike '*\RTE\*' } | ForEach-Object { clang-format -i $$_.FullName }
	@echo "Aplicado formato a todos los ficheros"

lint:
	@echo "TODO: Lint target"