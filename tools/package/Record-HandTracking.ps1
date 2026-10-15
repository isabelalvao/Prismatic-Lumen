$ErrorActionPreference = 'Stop'
$forwardedArguments = @($args)
$packageRoot = $PSScriptRoot
Set-Location -LiteralPath $packageRoot
$logDirectory = Join-Path $packageRoot 'logs'
$null = New-Item -ItemType Directory -Force -Path $logDirectory
$sessionTag = (Get-Date -Format 'yyyyMMdd-HHmmss') + '-' + [guid]::NewGuid().ToString('N').Substring(0, 8)
$sessionFile = Join-Path $logDirectory ('hands-' + $sessionTag + '.jsonl')
$consoleFile = Join-Path $logDirectory ('hands-' + $sessionTag + '.log')
Write-Output 'Liga o Quest pelo Virtual Desktop. Seleciona VDXR e ativa o envio de tracking para o PC.'
Write-Output 'A experiência começa em pausa. Espaço inicia; Esc fecha e guarda o registo.'
Write-Output 'As mãos são adquiridas mas não são desenhadas neste build.'
Write-Output ('Registo: ' + $sessionFile)
$savedErrorPreference = $ErrorActionPreference
try {
    # PowerShell 5.1 converte stderr nativo em ErrorRecord; conservar o código do programa.
    $ErrorActionPreference = 'Continue'
    & (Join-Path $packageRoot 'fantasynt.exe') --vr --paused --record-session $sessionFile @forwardedArguments *> $consoleFile
    $applicationResult = $LASTEXITCODE
} finally {
    $ErrorActionPreference = $savedErrorPreference
}
if ($applicationResult -ne 0) {
    Get-Content -LiteralPath $consoleFile
    Write-Output 'Falha no teste. Executa Diagnose-VR.cmd e conserva a pasta logs.'
}
if (Test-Path -LiteralPath $sessionFile) {
    & (Join-Path $packageRoot 'Verify-HandTracking.ps1') -SessionFile $sessionFile
    if ($applicationResult -eq 0 -and $LASTEXITCODE -ne 0) { $applicationResult = $LASTEXITCODE }
}
Write-Output 'Envia a pasta logs para analisar os resultados.'
exit $applicationResult
