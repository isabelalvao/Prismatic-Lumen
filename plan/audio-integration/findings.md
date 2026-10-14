# Desenvolvimento 0.4.2 — reprodução do áudio original

Pedido: continuar a integração dos recursos recuperados após instalação do UE4.13. O MASTER deixou de ser apenas um ficheiro de investigação: está no produto e reproduz-se na sequência automática.

## Comportamento implementado

- SDL3 abre o dispositivo predefinido de reprodução. Lê PCM16 estéreo 44100Hz em audio/ChezNous_MASTER.wav, com 74,825,728bytes/18,706,432frames/424.182131519s. Usa o ficheiro inteiro em memória e uma cópia na fila SDL (cerca de 150MB de PCM no total).
- Relógio da sequência, curvas e animação derivado da quantidade de amostras consumidas da fila. Formato de entrada constante; este relógio não mede a saída audível nem a latência física/Virtual Desktop.
- Espaço pausa áudio e sequência; Q limpa a fila e reinicia ambos; --start-time posiciona também o áudio. Cenas manuais suspendem a reprodução. VR continua inicialmente pausado. --silent e --fixed-time usam execução sem áudio.
- Na posição 424s suspende reprodução, respeitando o fim do perfil visual de 424 s, apesar dos 424.182 s do ficheiro. Pausa também à saída do ciclo antes de terminar capturas, conservando relatório temporal coerente.
- JSON permanece v1, com parâmetros e revisões independentes. motion_speed modula o avanço da câmara e não a velocidade da música. Pulsação dos shaders continua sintética; análise de batidas e despacho completo dos diretores estão pendentes.
- --audio permite seleccionar outro WAV PCM16 estéreo 44100Hz, com duração>=424s. Ficheiro ausente, inválido ou dispositivo indisponível produzem erro explícito; --silent permite executar sem dispositivo.

## Proveniência

O Ogg foi recuperado por UModel com perfil ue4.13 da instalação Steam, com SHA1 do payload confirmado no PAK. O PCM foi integralmente descodificado com a DLL Vorbis instalada no UE4.13; nenhuma DLL do motor é distribuída. assets/audio/provenance.json guarda hashes do Ogg, WAV e PCM. O PCM não foi normalizado nem reamostrado. O produto continua C++/bgfx, sem depender da instalação Unreal para executar.

## Verificação concluída

- red.log conserva falhas reais da versão 0.4.1: ausência de --silent/--audio e diagnósticos de áudio.
- final-tests.log: 7 CTest e 30 testes Python aprovados. Novos testes verificam procura a 180 s, reprodução, congelamento, reset, limites e NaN, relógio da sequência, saída silenciosa, ficheiro inexistente e coerência após captura. Regressões VR simulado/JSON/cenas também passaram.
- environment-tests.log: execução directa de build.ps1 -Test conserva variáveis de ambiente anteriores mesmo após substituir o driver por dummy para os testes. O script usa finally para restauração.
- package-check.json:ZIP extraído numa pasta com espaços/acentos, hashes, desktop, estéreo OpenXR simulado e cliente PowerShell JSON aprovados. O pacote inclui o WAV e proveniência; exclui runtime simulado e ferramentas UE4/UModel.
- real-device.json/log/png: backend real WASAPI aberto, consumiu PCM e atravessou a entrada do túnel; posição final de áudio e sequência 226.06 s. A captura foi inspecionada visualmente. Não se mediu saída audível, latência física ou desempenho prolongado; FPS deste ensaio inclui a escrita da captura.
- Revisão independente identificou avanço do áudio durante espera da captura e persistência da variável SDL_AUDIODRIVER. Ambos corrigidos e verificados; ajuda CLI actualizada. Sem problema crítico ou elevado identificado.

## Estado e trabalho seguinte

Entrega dist/Fantasynt-PCVR-Win11-x64-0.4.2.zip, cerca de 76 MB, com SHA256. Extrair inteiro e usar Launch-VR.cmd; Espaço inicia música/viagem, Q reinicia. ZIPs anteriores preservados; backups em plan/backups/audio-0.4.1; sem Git; instalação original e UE4 preservadas.

Geometria/textura de amostra estão recuperadas em plan/ue413-recovery, mas o renderer ainda usa formas e padrões próprios. A reconstrução dos restantes recursos, materiais, distribuição e coreografia continua. Esta etapa resolve reprodução de áudio; não demonstra fidelidade integral nem valida 0.4.2 num Quest físico.
