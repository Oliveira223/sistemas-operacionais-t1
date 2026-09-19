# executar_testes.ps1
# Script para executar os casos de teste

$Executable = ".\simulador.exe"
$CasosDir = "casos"

if (-Not (Test-Path $Executable)) {
    Write-Host "Executável não encontrado. Execute .\compilar.ps1 primeiro." -ForegroundColor Red
    exit 1
}

Write-Host "Procurando por casos de teste na pasta $CasosDir..." -ForegroundColor Cyan

# Pega todos os arquivos processos.txt recursivamente na pasta casos
$TestFiles = Get-ChildItem -Path $CasosDir -Filter "processos.txt" -Recurse

if ($TestFiles.Count -eq 0) {
    Write-Host "Nenhum caso de teste encontrado em $CasosDir." -ForegroundColor Yellow
    exit 0
}

foreach ($File in $TestFiles) {
    Write-Host "`n========================================================" -ForegroundColor Blue
    Write-Host "Executando caso de teste: $($File.Directory.Name)" -ForegroundColor Green
    Write-Host "Arquivo: $($File.FullName)" -ForegroundColor DarkGray
    Write-Host "========================================================`n" -ForegroundColor Blue
    
    # Executa o simulador
    & $Executable $File.FullName
}

Write-Host "`nTestes concluídos." -ForegroundColor Cyan
