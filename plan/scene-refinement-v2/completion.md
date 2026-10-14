# Afinação audiovisual 0.12.0 — concluída

Direção confirmada pelo utilizador: uma batida ilumina os anéis pares em ciano,
a seguinte os ímpares em rosa. O grupo ativo muda com os ataques graves
detetados no MASTER; entre ataques conserva-se uma emissão reduzida.

Opening acompanha agora as 18706432 amostras por canal do MASTER a 44100 Hz:
424,1821315192744 segundos (7:04,18). O WAV original não foi alterado. O fade
integral começa 16 segundos antes do fim e termina em preto, incluindo contador,
reflexos, pontos e brilho axial. O fim é limitado pela duração do áudio selecionado.

O campo de artefactos conserva cerca de 30% da densidade anterior. Cada peça
roda rigidamente e deriva com fase e velocidade próprias. As rochas medem
25–70 cm, com escala uniforme, seis variantes normalizadas, bases no chão
e folga perante os cones. Há 224 cones preparados em vez de 160 (+40%),
com espaçamento reservado e três pares de cores: ciano/magenta, azul/branco,
violeta/dourado. A densidade visível mantém também os controlos existentes.

Mantidos: construção dos cones em oito nervuras e nove círculos, cor quente
dessaturada, reflexos, corredor livre, permanência 60 m atrás e dissolução
nos 30 m seguintes; alinhamento dos anéis e dispersão das outras peças aos 290–306 s.

## Verificação

- Regressões demonstradas antes da implementação: paridade dos ataques, movimento
  independente, altura das rochas, quantidade dos cones e duração final.
- 17/17 testes CTest aprovados (`ctest.log`), incluindo GPU D3D11: ambos os
  grupos ativos comparados com ganho zero e preto integral com contador ligado.
- 45/45 testes de integração Python aprovados (`python-tests.log`).
- 13 instantes capturados e 7 comparações GPU aprovadas (`gpu-validation.json`):
  densidade, deriva, escala rochosa, paletas, pulsação e fade. Luminância média
  final medida: 0,0. Capturas em `captures/`.
- Quatro vídeos com o excerto correspondente do MASTER e descodificação sem
  erros (`previews.json`): `landscape.mp4`, `artifacts.mp4`, `rings.mp4`, `ending.mp4`.
  Câmara fixa nos dois primeiros; avanço normal nos últimos. Anéis a 24 fps;
  restantes a 12 fps. O vídeo final inclui um breve preto após o fim do áudio.
- Revisão independente de código aprovada. Corrigidos os dois avisos: preservação
  dos 224 cones após o corte e altura real por normalização uniforme das variantes.
- Pacote extraído numa pasta temporária com espaços e acentos: hashes,
  lançamentos no monitor/OpenXR simulado, cliente PowerShell e 12 instantes VR
  aprovados. Preto integral no pacote medido no monitor (`package-check.json`).
  O runtime simulado exige imagens luminosas diferentes por olho; não admite
  o preto final como cena estéreo. Validação física no Quest 3 não realizada.

## Ficheiros e entrega

Alterados: relógio e composição em `src/app/main.cpp` e `src/render/art_direction.*`;
detetor em `src/audio/beat_envelope.*`; geometrias e renderer em `src/render/`;
configuração em `src/config/` e `visual-config.json`; shaders; testes; guias;
scripts de captura, codificação e empacotamento; versão CMake.

Pacote: `dist/Fantasynt-PCVR-Win11-x64-0.12.0.zip`.

SHA-256: `c725b8835c9d11101ffaf07daa4afe9baccb136fd82f1c08c38b0f362f393dee`.

A entrega anterior permanece em `dist/`. Backup dos ficheiros anteriores
em `backup/`. Sem repositório Git; sem commit. As pastas temporárias criadas
pelos validadores e pelo empacotamento foram removidas automaticamente.

Para testar: `Fantasynt.cmd` na árvore atual ou `Launch-VR.cmd` depois de extrair
o pacote inteiro. Confirmar 0.12.0 no título da janela e manter Opening (tecla O).
As teclas N/L/T/F e `--scene sequence` selecionam vistas/montagem anteriores.
