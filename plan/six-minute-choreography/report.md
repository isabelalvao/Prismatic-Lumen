# Entrega 0.9.0 — coreografia de seis minutos

Experiência dirigida prolongada para 360 segundos. Arranque imediato, pirâmides aos 80 segundos, pilares ancorados sem sapata e contador alternado pela tecla C mantidos. A formação do túnel em 48 segundos corresponde à escolha explícita do utilizador.

## Coreografia

Os cones têm sete segmentos: começam a revelar-se aos 136 s, com intervalo de 2 s, revelação de 0,8 s e desfasamento por cone até 2 s. Cada segmento começa a rodar depois de completar a sua entrada; os sentidos alternam, com velocidade inicial de 2 graus/s. A integração histórica da velocidade mantém a orientação contínua ao recarregar o JSON, mesmo quando a alteração ocorre antes do nascimento de outros segmentos.

| Intervalo | Ação |
|---|---|
| 208–224 s | Peças da fase anterior desaparecem individualmente. |
| 224–272 s | Túnel forma-se peça a peça durante 48 segundos. |
| 272–284 s | Luz axial, halo e brilho do fundo acendem progressivamente. |
| 284–360 s | Túnel e padrões continuam com a música até ao fim da experiência. |

Os pesos da fase anterior chegam a zero antes da montagem do túnel. A luz de fundo permanece apagada no início e durante a formação. As transições e as entradas das formas posteriores têm parâmetros editáveis.

## Fase vermelha e análise do vídeo

O vídeo enviado foi descodificado para inspeção, preservando o original: 60,11 s, 848 × 478, 30 imagens/s, H.264 e áudio AAC. A cobertura estocástica dos nossos materiais produzia grão durante as misturas; os padrões LED muito finos e a atmosfera vermelha intensa agravavam a leitura. A compressão pode amplificar esse efeito, mas não era a única causa.

A experiência dirigida deixou de usar essa cobertura estocástica. A densidade colorida passou a 0,25; o multiplicador da névoa vermelha é 0,35, o da atmosfera 0,15 e a densidade dos padrões LED 0,33, com filtragem das arestas no shader. O coeficiente de névoa é aplicado uma única vez. Capturas reais da fase vermelha foram inspecionadas; não se garante ausência de todos os artefactos no headset ou na transmissão comprimida.

## Configuração e ficheiros

As configurações incluem 196 opções tipadas, 14 cores, sete parâmetros globais e áudio. Guardar visual-config.json ou carregar F5 recarrega as opções suportadas; valores inválidos mantêm a configuração anterior. O guia documenta intervalos e dependências dos novos controlos. As afinações anteriores foram preservadas.

Alterações principais: src/config/visual_config.*, visual_settings.inc, src/sequence/art_direction.*, os módulos surface_choreography e rotation_history, renderer e shaders, testes, CMake/build, scripts de empacotamento e documentação. As cópias anteriores dos ficheiros alterados estão em backup/. Os sete segmentos usam malhas derivadas em memória; os recursos originais em disco não foram modificados.

## Verificação

- 16 testes CTest e 43 testes Python aprovados; resultados em verification.log.
- Ensaios de continuidade das entradas e da rotação após recarga, saída anterior ao túnel, limite dos 360 s e luz apagada durante a formação.
- Capturas das fases 137, 141, 151, 180, 198, 208, 216, 224, 240, 272, 284 e 360 s inspecionadas e revisão independente aprovada, sem problemas elevados ou críticos pendentes.
- Pacote extraído num caminho com caracteres portugueses verificado em desktop, estéreo OpenXR simulado e cliente JSON PowerShell; hashes do manifesto válidos. Ver package-check.json e package-check.log.
- Prévia cone-segments-preview.mp4: oito segundos de renderização real, câmara fixa, sem áudio, quatro imagens/s e reprodução à velocidade normal; corresponde aos instantes 156–164 s.

O Quest 3 físico não foi testado nesta versão. Os testes de áudio automatizados usam um dispositivo simulado, não uma avaliação auditiva do headset.

## Entrega e preservação

Pacote: dist/Fantasynt-PCVR-Win11-x64-0.9.0.zip.

SHA256: 48385a6150fe0ae3bef52031c139f0523781a7ea32a5ab7964665e6c5ac8cefd.

O MASTER original tem 424,1821315 s e foi preservado; a experiência termina aos 360 s sem alterar o ficheiro WAV. SHA256 do MASTER: 60dc74309ab1fbae42394228ddfc098a09a1e7a36ae739c7f5ee7e1adafb9fd9.

ZIPs anteriores, recursos recuperados, instalação Steam e UE 4.13 preservados. O utilitário de análise de vídeo foi guardado em tools/vendor/video-analysis-runtime e não integra as dependências distribuídas da aplicação. Não foi inicializado Git, nem criado commit ou PR. Esta entrega implementa a direção artística pedida; não constitui uma reprodução integral comprovada do Fantasynth original.
