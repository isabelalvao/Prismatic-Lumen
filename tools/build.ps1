param(
    [switch]$CoreOnly,
    [switch]$Test,
    [ValidateSet('Release', 'Debug')][string]$Configuration = 'Release',
    [ValidateRange(1, 8)][int]$Jobs = 4
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $projectRoot
$vswherePath = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswherePath)) { throw 'Instala as Visual Studio Build Tools com o componente C++.' }
$vsInformation = & $vswherePath -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json
$cppInstallation = $vsInformation | ConvertFrom-Json | Select-Object -First 1
if (-not $cppInstallation) { throw 'Não foi encontrado um compilador Visual C++.' }
$majorVersion = [int]($cppInstallation.installationVersion.Split('.')[0])
if ($majorVersion -ge 18) { $generatorName = 'Visual Studio 18 2026' }
elseif ($majorVersion -eq 17) { $generatorName = 'Visual Studio 17 2022' }
else { throw 'Este projeto requer Visual Studio Build Tools 2022 ou mais recente.' }
$cmakePath = Join-Path $cppInstallation.installationPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
if (-not (Test-Path -LiteralPath $cmakePath)) {
    $cmakeCommand = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if (-not $cmakeCommand) { throw 'Instala as ferramentas CMake nas Visual Studio Build Tools.' }
    $cmakePath = $cmakeCommand.Source
}
$ctestPath = Join-Path (Split-Path -Parent $cmakePath) 'ctest.exe'
function Invoke-Checked([string]$Executable, [string[]]$CommandArgs) {
    & $Executable @CommandArgs
    if ($LASTEXITCODE -ne 0) { throw "Comando falhou com código $LASTEXITCODE : $Executable" }
}
$appMode = if ($CoreOnly) { 'OFF' } else { 'ON' }
Invoke-Checked $cmakePath @('-S', '.', '-B', 'build', '-G', $generatorName, '-A', 'x64', "-DFANTASYNT_BUILD_APP=$appMode")
$targets = @('fantasynt_control_tests', 'fantasynt_network_tests', 'fantasynt_geometry_tests', 'fantasynt_vr_math_tests', 'fantasynt_sequence_tests', 'fantasynt_opening_tests', 'fantasynt_direction_tests', 'fantasynt_visual_config_tests', 'fantasynt_time_display_tests', 'fantasynt_phase_tests', 'fantasynt_comfort_tests', 'fantasynt_six_minute_tests', 'fantasynt_choreography_tests', 'fantasynt_rotation_history_tests', 'fantasynt_spectral_geometry_tests')
$targets += @('fantasynt_observation_tests','fantasynt_session_recorder_tests')
$targets += @('fantasynt_hand_geometry_tests')
$targets += @('fantasynt_motif_tests')
$targets += @('fantasynt_hand_snap_tests')
$targets += @('fantasynt_light_pencil_tests')
if (-not $CoreOnly) { $targets += @('fantasynt', 'fantasynt_renderer_tests', 'fantasynt_mock_openxr', 'fantasynt_audio_tests') }
Invoke-Checked $cmakePath (@('--build', 'build', '--config', $Configuration, '--parallel', "$Jobs", '--target') + $targets)
if ($Test) {
    Invoke-Checked $ctestPath @('--test-dir', 'build', '-C', $Configuration, '--output-on-failure')
    $pythonCommand = Get-Command python.exe -ErrorAction SilentlyContinue
    if (-not $pythonCommand) { throw 'Python é necessário para os testes de integração.' }
    $pattern = if ($CoreOnly) { 'test_client.py' } else { 'test_*.py' }
    $savedTestExecutable = $env:FANTASYNT_EXE
    $savedTestAudioDriver = $env:SDL_AUDIODRIVER
    try {
        $env:FANTASYNT_EXE = Join-Path $projectRoot "build\$Configuration\fantasynt.exe"
        $env:SDL_AUDIODRIVER = 'dummy'
        Invoke-Checked $pythonCommand.Source @('-m', 'unittest', 'discover', '-s', 'tests', '-p', $pattern, '-v')
    } finally {
        [Environment]::SetEnvironmentVariable('FANTASYNT_EXE', $savedTestExecutable, 'Process')
        [Environment]::SetEnvironmentVariable('SDL_AUDIODRIVER', $savedTestAudioDriver, 'Process')
    }
}
