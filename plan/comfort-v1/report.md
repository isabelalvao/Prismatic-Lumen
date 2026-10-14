# Ajustes visuais e leitura do contador — 0.8.1

Pedido de 2026-10-13: remover as bases dos mini-pilares, manter contacto com o piso durante o movimento, reduzir a densidade da fase colorida e subir o contador para leitura no Quest 3.

## Alterações

SM_Pole utiliza uma malha derivada sem a sapata larga. A haste alonga e encurta a partir do piso y=0; a cabeça luminosa desloca-se inteira, mantendo as dimensões. Foram preservadas as fases e frequências individuais e a compensação na recarga. SM_Pole_Map já não tinha sapata e os pilares grandes conservam o movimento anterior. Os recursos recuperados em disco não foram alterados.

A fase colorida conserva cerca de 35% da geometria, com uma pequena faixa de atenuação gradual por instância. A seleção é estável e dispersa, incluindo grupos nativos pequenos que antes escapavam ao controlo de densidade. O fator entra na transição 136–148 s e regressa a 1 durante 224–232 s; a primeira metade e o túnel final conservam a densidade anterior. `extension.geometry_density_multiplier` permite afinar; zero remove a geometria dessa fase, mantendo céu, chão e contador.

O contador foi colocado em y=0,65 em vez de 0,88, com escala 1,5, opacidade 0,95 e traços mais espessos. C continua a alternar visibilidade. `timer.y/scale/opacity` permanecem editáveis sem recompilar. A leitura no headset físico será confirmada pelo utilizador; localmente foram ensaiados monitor e OpenXR simulado.

O novo campo `poles.minimum_height_m` tem valor inicial 0,35 m. Além desse mínimo solicitado, a espessura natural da cabeça impõe um limite: nem configurações extremas podem inverter a haste ou colocar a cabeça abaixo do chão.

## Ficheiros

- `src/render/mini_poles.*`: derivação sem sapata e deformação da haste.
- `opening_scene.hpp`, `art_direction.*`, `renderer.cpp`, `sequence.hpp` e shaders Opening: ancoragem, limites/culling, uniformes e redução de densidade.
- `time_display.*`, shader de composição e configuração: contador mais alto e legível; dois novos parâmetros, catálogo total de 184 opções tipadas e 14 cores.
- CMake, testes, scripts de compilação/empacotamento, JSON do projeto e runtime, README e guias atualizados.

Cópias anteriores em `backup/`. Apenas os ajustes pedidos foram atualizados nos JSON existentes; as restantes afinações foram conservadas.

## Verificação

13 testes CTest e 43 testes Python passaram (`verification.log`). Os ensaios verificam malha real sem sapata, contacto com o piso em múltiplos tempos, cabeça com tamanho constante, corredor livre, recarga, contador, teclas, áudio, fases e estéreo OpenXR simulado.

O primeiro ensaio RED confirmou ausência da ancoragem e valores antigos do contador (`red.log`). A revisão identificou um limite extremo: mínimo de 5 cm e amplitude elevada podiam inverter a haste. O teste com a malha real e escalas verticais 0,1/1/10 falhou antes da correção (`minimum-red.log`) e passou após limitar também o comprimento positivo da haste. Densidade zero foi comparada na GPU com geometria global desativada: falhou antes (`density-red.log`) e passou depois de corrigir a cauda de atenuação. O teste de movimento foi corrigido para observar vários instantes, pois duas amostras de uma sinusóide podem coincidir sem o objeto estar imóvel.

Capturas inspecionadas aos 60/180/232 s. A comparação dos 180 s com multiplicadores 0,35 e 1 mostra maior espaço livre e menos sobreposição (`captures/phase-180.png`, `phase-180-dense.png`). Contador maior e mais alto verificado; túnel final conservado. Revisão independente aprovada, sem CRITICAL/HIGH remanescentes.

O ZIP extraído passou desktop, estéreo OpenXR simulado, cliente PowerShell JSON e verificações adicionais aos 150/232/240 s. O catálogo do pacote contém os novos valores de contador e densidade; hashes do manifesto válidos (`package-check.json`).

## Entrega

ZIP: `dist/Fantasynt-PCVR-Win11-x64-0.8.1.zip`.

SHA-256: `c5365a160d421387877c426bbff0d7fc19d8ad465e4f2f6693df25cae0143dab`.

O ZIP 0.8.0 permanece preservado: `b282e1e2286cc450944ad86e5ce18f0a055a41308e92da30b32d0781f11c31dc`. O MASTER mantém `60dc74309ab1fbae42394228ddfc098a09a1e7a36ae739c7f5ee7e1adafb9fd9`. Steam, UE 4.13 e malhas recuperadas não foram alterados. Não existe repositório Git e não foi inicializado. Temporários de empacotamento foram removidos pelos respetivos scripts.
