# Contador, pilares e extensão musical — 0.8.0

Entrega de 2026-10-13. O utilizador pediu contador alternável pelo teclado, movimento vertical lento e descoordenado dos pilares assinalados e correção das formas sem luz perto de 120 s. Confirmou incluir já uma proposta de extensão até 240 s.

## Resultado

O contador mostra `MM:SS.d` no monitor, nos dois olhos VR e no espelho. C mostra/oculta, Espaço congela com a reprodução e Q reinicia. Usa o relógio da música, não o tempo decorrido do processo. Começa visível e permite ajustar tamanho, posição e opacidade no JSON. Uma recarga de outros parâmetros conserva a escolha feita com C.

Os pilares dos materiais Pole/TreePoint oscilam ±0,7 m em ciclos de referência de 36 s, com fases e frequências individuais. Frequência, variação e amplitude são editáveis. A compensação de fase preserva as posições em alterações ao período, velocidade global e variação individual. Alterar amplitude pode mudar a altura de imediato. O corredor considera todo o alcance vertical da oscilação.

Formas com material sem emissão recuperada passam a acender linhas entre 104–116 s, com intensidade, quantidade e largura editáveis. As formas arredondadas da família Pulse conservam a regra de aparecer apenas durante pulsos graves; não se inventou iluminação permanente para contrariar essa orientação anterior.

Opening dura 240 s, com arranque imediato e pirâmides aos 80 s. A sequência de comparação conserva 424 s, transição legada 120–128 s e percurso original após a primeira fase. Cenas anteriores continuam disponíveis.

## Proposta artística implementada

| Intervalo | Evolução |
| --- | --- |
| 120–136 s | Continuidade do cenário nativo, com padrões antes ausentes já luminosos. |
| 136–148 s | Entrada de formas segmentadas, diamantes e padrões circulares; laranja/dourado. |
| 172–184 s | Estruturas curvas/camadas e passagem para azul/violeta. |
| 208–220 s | Redução gradual da luz até 25%, mantendo referências espaciais. |
| 224–232 s | Reentrada luminosa com anéis e arquitetura de túnel. |
| 232–240 s | Túnel sustentado, sem introduzir nova mudança no último instante. |

Os tempos 144/176/208/224 s vêm de controladores recuperados. A medição do MASTER confirma redução de nível e ataques entre 208–224 s e retoma aos 224 s. As rampas, paletas e misturas são propostas artísticas novas: não demonstram reprodução integral do original. Evidência e métricas: `music-proposal.md` e `metrics.json`. A proposta do analista sugeria malhas Lower recuperadas; esta primeira implementação usa também as geometrias das cenas anteriores para diversificar a extensão, conservando parte da floresta nativa até ao túnel.

A entrada conserva a dimensão das malhas e usa cobertura espacial progressiva para revelar superfícies com profundidade correta. O grão visível durante a mistura tem caráter LED e pode merecer afinação artística no headset. Não foi alegada validação física nesta versão. O contador fica fora dos reflexos, bloom, exposição e rarefação para continuar legível.

## Configuração e alterações

`visual-config.json` da raiz é usado por `Fantasynt.cmd`; o pacote usa o JSON junto do executável. Catálogo: 182 opções tipadas, 14 cores, parâmetros globais e áudio. Guia: `assets/visual-config-guide.md`. Guardar aplica automaticamente; F5 força. A paleta da extensão substitui gradualmente `parameters.palette`, sendo afinada em `extension.*_colour`.

Fontes alteradas: `src/render/time_display.*`, `renderer.*`, `art_direction.*`, `visual_uniforms.*`, `sequence.hpp`, `src/config/visual_config.hpp`, `visual_settings.inc`, `src/app/main.cpp` e shaders de composição, Opening e cenas. CMake, testes, scripts de compilação/empacotamento, configurações e documentação atualizados. Cópias anteriores em `backup/`.

## Verificação

- 12 testes CTest e 43 testes Python aprovados, incluindo contador, tecla C enviada à janela SDL real, áudio até 240 s, recarga, transições, controlador JSON e estéreo OpenXR simulado (`verification.log`).
- Contador em imagens reais: presença/ocultação, escala e posição, estabilidade no tempo fixo e alteração dos dígitos. Teste GPU falhou antes da implementação (`timer-renderer-red.log`) e passou depois.
- Os primeiros ensaios da extensão falharam por ausência da duração e do contador (`phase-red.log`).
- Uma entrada opaca das formas e a variação de frequência sem compensação foram detetadas em testes gráficos a falhar (`fade-red.log`). Após correção, imagens antes/depois das fronteiras 136/172/224 s são contínuas; recarga da variação dos pilares conserva pixels.
- Iluminação das formas anteriormente escuras comparada na GPU com emissão ativada/desativada aos 119 s.
- Capturas desktop inspecionadas aos 119/150/232/240 s; revisão independente aprovou código e capturas, sem CRITICAL/HIGH remanescentes.
- ZIP extraído passou desktop, OpenXR simulado, cliente PowerShell JSON e capturas VR adicionais aos 150/232/240 s; hashes do manifesto válidos (`package-check.json`).

## Entrega

`dist/Fantasynt-PCVR-Win11-x64-0.8.0.zip`

SHA-256: `b282e1e2286cc450944ad86e5ce18f0a055a41308e92da30b32d0781f11c31dc`.

A versão 0.7.0 permanece preservada: `0adffd18cfaa2ecf7cb952044a966299d445f69117a9af98b36795a592d0cbee`. O MASTER mantém `60dc74309ab1fbae42394228ddfc098a09a1e7a36ae739c7f5ee7e1adafb9fd9`. Malhas recuperadas, jogo Steam e UE 4.13 não foram alterados. Não existe repositório Git e não foi inicializado.

Os temporários do empacotamento foram limpos pelos scripts. Artefactos compilados dos ensaios RED/GREEN do contador foram conservados em `diagnostics/timer-test-build` em vez de apagados, retirando-os da pasta temporária. A avaliação artística e no headset físico cabe ao próximo teste do utilizador.
