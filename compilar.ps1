# compilar.ps1
# Script para compilar o simulador de sistemas operacionais

$SrcFiles = "main/main.cpp", "main/parser.cpp", "main/executor.cpp", "main/escala.cpp", "main/simulador.cpp"
$OutputFile = "simulador.exe"
$CompilerFlags = "-std=c++11", "-Wall", "-Wextra"

Write-Host "Compilando o projeto..." -ForegroundColor Cyan

# Executa o g++
$Command = "g++"
$Args = $CompilerFlags + $SrcFiles + "-o" + $OutputFile

& $Command $Args

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilação concluída com sucesso! Executável gerado: $OutputFile" -ForegroundColor Green
} else {
    Write-Host "Erro durante a compilação." -ForegroundColor Red
    exit $LASTEXITCODE
}
