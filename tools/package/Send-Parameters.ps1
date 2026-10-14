param(
    [string]$File = 'examples\energetic.json',
    [ValidateRange(1,65535)][int]$Port = 47631
)
$ErrorActionPreference = 'Stop'
$client = New-Object System.Net.Sockets.TcpClient
try {
    $payload = [System.IO.File]::ReadAllText((Join-Path $PSScriptRoot $File))
    $null = ConvertFrom-Json -InputObject $payload
    $encoding = New-Object System.Text.UTF8Encoding($false)
    $bytes = $encoding.GetBytes(($payload -replace '[\r\n]', '') + "`n")
    if ($bytes.Length -gt 16385) { throw 'Mensagem demasiado grande.' }
    $pending = $client.ConnectAsync('127.0.0.1', $Port)
    if (-not $pending.Wait(3000)) { throw 'Tempo de ligação excedido.' }
    $client.SendTimeout = 3000
    $client.ReceiveTimeout = 3000
    $stream = $client.GetStream()
    $stream.Write($bytes, 0, $bytes.Length)
    $response = New-Object System.IO.MemoryStream
    while ($response.Length -lt 65536) {
        $byte = $stream.ReadByte()
        if ($byte -lt 0) { throw 'Ligação terminada sem confirmação.' }
        if ($byte -eq 10) { break }
        $response.WriteByte([byte]$byte)
    }
    if ($response.Length -ge 65536) { throw 'Resposta demasiado grande.' }
    $receiptText = $encoding.GetString($response.ToArray())
    $receiptText
    $receipt = $receiptText | ConvertFrom-Json
    if ($receipt.status -ne 'accepted') { exit 2 }
} catch {
    Write-Error $_
    exit 1
} finally {
    $client.Dispose()
}
