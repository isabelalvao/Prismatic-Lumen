# Lápis de luz e dispersão do estalo

Pedido do utilizador em 2026-10-15, após retorno positivo sobre a primeira versão do estalo: ampliar um pouco a explosão, com subtileza, e acrescentar um indicador que escreve luz quando só esse dedo está esticado. O retorno não incluiu novas medições físicas. Implementado na compilação Release de desenvolvimento; sem commit/push. Música, percurso, composição, P1 e afinação da raiz preservados.

## Testar os dois efeitos em VR

Na raiz `C:\DEV\Prismatic`, com hand tracking disponível/encaminhado no runtime OpenXR:

```powershell
.\build\Release\fantasynt.exe --config .\visual-config.json --vr --paused --hand-snap --hand-light-pencil
```

Pode testar imediatamente, mesmo com a música parada. Espaço inicia/pausa a experiência; Esc fecha. Cada mão funciona independentemente. O estalo mantém o gesto anterior. Para escrever, esticar o indicador e recolher polegar, médio, anelar e mínimo; mover a ponta no ar. Levantar outro dedo ou recolher o indicador interrompe a escrita e deixa o traço anterior desaparecer. Não é preciso ativar `--motifs-scheduled`.

`--hand-light-pencil` e `--hand-snap` são independentes; pode usar apenas um. Ambos exigem VR. Sem as opções, a experiência mantém o comportamento anterior. Com lápis ativo, cria automaticamente `logs/hand-effects-<id>.jsonl`, com nome novo; `--record-session` permite indicar outro caminho novo. Sem sobrescrita.

## Estalo ligeiramente maior

Dispersão radial **25% maior**, pontos **12% maiores**. Mantêm-se 14 pontos, cor/intensidade base, duração de 0,45 s e desvanecimento anteriores. Sem alterar o detetor de estalos, o intervalo entre disparos ou o som. Manifesto regista os fatores de escala. A especificação original continua em [hand-snap-2026-10-15](../hand-snap-2026-10-15/README.md).

## Lápis de luz

- Filamento fino azul/branco, com núcleo de cerca de 1,2 mm de raio e pequeno ponto na ponta. A cauda perde brilho e espessura progressivamente; cada amostra dura no máximo **0,65 s**. A ponta permanece acesa quando a pose está quieta, sem acumular pontos.
- Deteção geométrica `index-only-trail-1`: indicador alongado/alinhado, outros três dedos dobrados e polegar junto à palma/base do indicador. A regra usa distâncias e alinhamentos relativos à mão, independentemente da orientação no espaço. Não identifica intenção nem estados emocionais.
- Exige posições finitas, válidas **e seguidas** dos pontos usados. A pose deve manter-se pelo menos **60 ms** para iniciar. Não presume que dedos ausentes estejam recolhidos. Mão aberta, dois dedos esticados e polegar levantado ficam inelegíveis nos testes sintéticos.
- Amostras à cadência XR disponível; afastamento mínimo de **3 mm** entre pontos, filtro exponencial com constante de **12 ms** dentro do segmento válido e máximo de **128 pontos por mão**. Estes são parâmetros de implementação, não medidas de latência física.
- A emissão começa alguns milímetros à frente da ponta do indicador, de acordo com o raio da articulação, para não ficar escondida pela esfera que representa o dedo. Os pontos ficam no espaço físico LOCAL e não recebem o avanço da cena nem seguem a mão depois de escritos. Usam as mesmas conversões/offsets dos olhos e mãos.
- A mesma geometria entra nos dois olhos e no espelho, com profundidade, bloom e fade final existentes. Usa mistura aditiva sem escrever profundidade. Shader próprio permite desvanecer cada ponto ao longo do traço; não aplica fade uniforme a toda a linha.
- Mudar de pose encerra o segmento; uma nova pose inicia outro, sem ligar as duas posições. Perda de seguimento, origem nova, tempo repetido/regressivo, intervalo XR acima de **80 ms** ou salto da ponta acima de **12 cm por amostra** interrompem/limpam o rasto. Nenhuma interpolação atravessa a falha. Mãos/cabeça inválidas não deixam um lápis fantasma.
- O tempo vem de XR, por isso o efeito continua com a música em pausa. Não muda o relógio audiovisual, o MASTER ou os motivos P1.

As regras são uma aproximação inicial para ensaio, não uma classificação física validada no Quest. A mão pode ser parcialmente oculta quando os dedos estão recolhidos; dados sem seguimento impedem escrever. A sensibilidade/dose visual ainda precisam do teste real, sem calibrar com o registo histórico.

## Registo e verificação

Manifesto regista ativação, versão da pose, duração e capacidade do rasto; eventos `hand_light_pencil` registam início, fim ou reset, mão, motivo, origem e tempos XR/receção/cena. As observações P0 permanecem intactas; não se declara que a publicação a 20 Hz reconstitui todos os pontos visuais calculados à cadência XR. Escritor assíncrono e integridade existentes preservados.

Resultados finais em [verification.json](verification.json): **22/22 CTest**, **teste gráfico separado** e **93/93 testes Python** aprovados. Logs em [ctest.log](ctest.log), [renderer.log](renderer.log) e [python-tests.log](python-tests.log). Capturas sintéticas de comparação com/sem lápis e do estalo ampliado em `temp/hand-pencil-e82208d9/`, revistas visualmente; os quatro registos de captura passam o inspetor de integridade P0. Cópias delimitadas do estado anterior em `backup/`, [estado Git inicial](git-status-before.txt). Não foram substituídas imagens históricas.

Testes cobrem pose, mudança de orientação/translação, dedos adicionais, perda, NaN, mudança de origem/tempo, salto de ponta, segmentos independentes, expiração, ponta quieta sem acumulação, limite de pontos, geometria finita e índices. A integração D3D11/runtime simulado verifica imagem/ativação em pausa, fim do rasto, remoção após perda, mãos abertas/dois dedos e coexistência com estalo. **A fiabilidade e a qualidade visual físicas continuam por verificar no Quest.**
