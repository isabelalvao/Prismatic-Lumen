# Revisão da integração de áudio 0.4.2

Data: 2026-10-12. Resultado: **aprovado**, sem problemas CRITICAL, HIGH ou MEDIUM pendentes no âmbito revisto.

Revisão estática de `src/audio/player.hpp`, `src/audio/player.cpp`, `src/app/main.cpp`, `CMakeLists.txt`, `tests/audio_tests.cpp`, `tests/test_audio.py`, `tools/build.ps1`, `tools/package_windows.py` e `tools/check_package.py`. Comparação com `plan/backups/audio-0.4.1` onde existe referência; consulta dos cabeçalhos e da implementação local do SDL3 para as garantias de pausa, fila, procura e destruição do dispositivo. Não foram compilados nem executados binários pelo revisor.

O leitor usa RAII para o PCM e o fluxo SDL, valida formato e limites numéricos, preserva caminhos Unicode e calcula a posição pelo consumo da fila de entrada. As cenas manuais suspendem o áudio; o reinício da sequência procura o início; os modos silencioso e de tempo fixo evitam a inicialização do leitor.

Os avisos encontrados foram corrigidos: o áudio é suspenso antes da espera pela captura, com atualização final coerente da sequência e da câmara; a ajuda inclui as novas opções; o script de testes guarda e repõe `SDL_AUDIODRIVER` e `FANTASYNT_EXE` em `finally`, incluindo o caminho de erro.

Evidência consultada: os registos finais indicam 7 testes CTest e 30 testes Python aprovados; `package-check.json` confirma hashes e os percursos desktop/OpenXR simulado com comandos JSON; `real-device.json` regista WASAPI ativo e tempos de áudio e sequência iguais a 226.06 segundos após 120 frames. A verificação efetiva da reposição de ambas as variáveis na mesma sessão PowerShell foi comunicada pelo agente principal.

Os testes SDL simulados verificam procura, reprodução, pausa, reinício, limites, NaN e coerência durante a captura. A abertura e o consumo de PCM pelo WASAPI não constituem uma medição da saída audível ou da latência física; o headset físico não foi validado nesta revisão.
