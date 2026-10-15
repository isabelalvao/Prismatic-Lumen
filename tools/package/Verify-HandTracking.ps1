param([string]$SessionFile)
$ErrorActionPreference = 'Stop'
if (-not $SessionFile) {
    $latest = Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'logs') -Filter '*.jsonl' -File |
        Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (-not $latest) { throw 'Não existe um registo de mãos na pasta logs.' }
    $SessionFile = $latest.FullName
}
$SessionFile = (Resolve-Path -LiteralPath $SessionFile).Path
$manifest = $null
$ending = $null
$observations = 0
$headValid = 0
$leftActive = 0
$rightActive = 0
$leftValid = 0
$rightValid = 0
$leftTracked = 0
$rightTracked = 0
$totalLines = 0
foreach ($line in [IO.File]::ReadLines($SessionFile, [Text.Encoding]::UTF8)) {
    $event = ConvertFrom-Json -InputObject $line
    if ($totalLines -eq 0 -and $event.type -ne 'session_manifest') { throw 'Manifesto de sessão ausente.' }
    if ($ending) { throw 'Há dados depois do encerramento da sessão.' }
    $totalLines++
    switch ($event.type) {
        'session_manifest' { $manifest = $event }
        'session_end' { $ending = $event }
        'observation' {
            if ($event.session_id -ne $manifest.session_id) { throw 'IDs de sessão incompatíveis.' }
            $observations++
            if ($event.xr.head.position_valid) { $headValid++ }
            if ($event.xr.hands[0].active) { $leftActive++ }
            if ($event.xr.hands[1].active) { $rightActive++ }
            $leftPalm = $event.xr.hands[0].joints[0]
            $rightPalm = $event.xr.hands[1].joints[0]
            if ($event.xr.hands[0].active -and $leftPalm.position_valid -and $null -ne $leftPalm.position_m) {
                $leftValid++
                if ($leftPalm.position_tracked) { $leftTracked++ }
            }
            if ($event.xr.hands[1].active -and $rightPalm.position_valid -and $null -ne $rightPalm.position_m) {
                $rightValid++
                if ($rightPalm.position_tracked) { $rightTracked++ }
            }
        }
    }
}
$complete = $null -ne $ending -and $ending.complete -and $ending.normal_exit -and
    -not $ending.write_failed -and $ending.dropped -eq 0 -and $ending.accepted -eq ($totalLines - 2)
$trackersReady = $null -ne $manifest.tracking -and $manifest.tracking.hand_trackers_ready
$report = [ordered]@{
    session_file = $SessionFile
    session_id = $manifest.session_id
    runtime = $manifest.runtime
    complete = [bool]$complete
    hand_trackers_ready = [bool]$trackersReady
    observations = $observations
    head_valid_samples = $headValid
    left_active_samples = $leftActive
    right_active_samples = $rightActive
    left_position_valid_samples = $leftValid
    right_position_valid_samples = $rightValid
    left_position_tracked_samples = $leftTracked
    right_position_tracked_samples = $rightTracked
    hand_acquisition_observed = [bool]($complete -and $trackersReady -and $headValid -gt 0 -and $leftTracked -gt 0 -and $rightTracked -gt 0)
    dropped_records = $ending.dropped
    scope = 'Acquisition and file integrity only; not perceptual validation or descriptor replay.'
}
$json = $report | ConvertTo-Json -Depth 4
$reportFile = [IO.Path]::ChangeExtension($SessionFile, '.inspection.json')
[IO.File]::WriteAllText($reportFile, $json, (New-Object Text.UTF8Encoding($false)))
Write-Output $json
Write-Output ('Relatório: ' + $reportFile)
if (-not $complete) { exit 1 }
if (-not $report.hand_acquisition_observed) { exit 2 }
exit 0
