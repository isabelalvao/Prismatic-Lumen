# Afinação do cenário — entrega 0.11.0

14 de outubro de 2026. Direção aprovada a partir das cinco capturas do utilizador.

## Resultado

- Ambiente quente preservado: saturação da atmosfera 0,32; nevoeiro 28% e névoa
  luminosa 10% da intensidade de referência. Reflexos conservados.
- Cones em 17 malhas próprias: oito nervuras completas primeiro, desenhadas em
  altura, depois nove círculos da base à ponta. Entradas alinhadas a ataques graves
  detetados no MASTER, com fallback temporal quando faltam ataques.
- Triângulos sólidos nativos/procedurais e bases/plataformas Hex/Lower/procedurais
  removidos no percurso dirigido. Volume central desimpedido; pontos conservam
  emissão branca na fase quente.
- Peças conservadas até 60 m atrás do utilizador e dissolvidas nos 30 m seguintes.
  Distâncias editáveis: `layout.retain_behind_m`, `layout.behind_fade_m`. A extensão
  dos objetos é considerada e a reciclagem só acontece depois da dissolução.
- 64 anéis inicialmente dispersos alinham-se e crescem entre 290–306 s. Cada anel
  demora oito segundos, com desfasamentos de oito segundos ao longo do conjunto;
  o primeiro está formado aos 298 s. Raio interior 5 m, espaçamento 8 m, editáveis.
  O raio aumenta se o volume do corredor exigir mais folga.
- Artefactos restantes afastam-se individualmente entre 290–306 s e desaparecem.
  O brilho axial rosa #FF58CF acende entre 294–306 s, com cor/intensidade editáveis.

A posição da câmara no início da formação e a antecedência são capturadas para
conservar a posição do túnel durante recargas de velocidade. A reserva considera
os extremos da sequência e a velocidade máxima de motion_speed: 50 m efetivos no
perfil inicial. Uma aceleração extrema durante a montagem antecipa suavemente a
conclusão das peças próximas antes da margem de 10 m, sem deslocar a origem.

## Verificação

- Compilação Release concluída; 17/17 CTest e 45/45 testes Python aprovados em
  `verification-delivery.log`. Avisos existentes de alinhamento em bx e APIs
  desaconselhadas de capture/u8path permanecem; não há erros de compilação.
- Testes primeiro: falhas de malhas agrupadas, anéis já axiais, obstáculos e luz
  antecipada demonstradas antes da implementação. Teste GPU reproduziu o salto
  da recarga após 306 s antes da correção. Ensaios geométricos reproduziram a falta
  de folga com aumento de velocidade e com velocidade 10 m/s/espaçamento 1 m.
- 14 verificações GPU aprovadas: atmosfera, reflexos, obstáculos, dispersão,
  alinhamento, raio/espaçamento, batidas e permanência posterior em VR simulado.
  Evidência: `gpu-validation-final.log`, `gpu-validation.json`, `captures/`.
- Capturas principais nos segundos 136, 140, 148, 154, 160, 168, 174, 180, 207,
  240, 284, 290, 298, 306, 330 e 360. Inspecionadas montagem, ambiente quente,
  alinhamento, túnel formado e vista para trás com rotação de 180 graus.
- Revisão independente concluída sem defeitos críticos/altos pendentes.
- Pacote extraído num caminho com caracteres portugueses: hashes do manifesto,
  desktop, estéreo OpenXR, cliente JSON PowerShell e 11 fases temporais aprovados
  em `package-check.log` e `package-check.json`.
- Prévia: 160 capturas reais a quatro imagens/s, 40 s, câmara fixa e sem áudio,
  mostrando a montagem de 136–175,75 s em `cone-assembly.mp4`.

A observação no Quest 3 físico fica por confirmar; os ensaios OpenXR usam o runtime
simulado e imagens D3D11 reais. Não foi alterado o áudio nem os recursos recuperados.

## Ficheiros e entrega

Alterações em configuração tipada/JSON, geometria espectral/túnel, coreografia,
renderer, shaders de cena/composição/Opening, testes, README/guia e ferramentas
de empacotamento/validação. Cópias anteriores em `backup/`.

Pacote: `dist/Fantasynt-PCVR-Win11-x64-0.11.0.zip`.

SHA256: `84fb138d46ebee8bdfe2169d9ad5f80e877e221dc84fa4e491019fb40f399fd2`.

A entrega 0.10.0 conserva o SHA256
`b7fa18890b67e23a2c0d082479b78b8ec18ce4dcefbded2cad0b9cf493704b31`.
Não existe repositório Git neste diretório; não foi criado um.
