# Opening 0.5.0 — primeiros 120 segundos

Cena independente e predefinida, sem menu. Implementa a introdução e a entrada de Forest usando dados recuperados do original. A resolução VR predefinida é 1,0 na aplicação, na API OpenXR e no lançador. Os movimentos de cabeça da gravação não foram incorporados na animação; a orientação continua a vir do HMD.

## Recursos integrados e proveniência

- 55 malhas originais, com posições, normais, UV e índices convertidos de glTF para `.fmesh`; conversão metros→centímetros e eixos UE→renderer `(X,Z,Y)`.
- 68 grupos, 42 458 instâncias. As tabelas Intro (2 239 linhas) e Forest (40 258 linhas) foram consumidas integralmente. As 39 linhas não integradas referem fontes Pyro sem malha estática resolvida.
- Transformações de cada linha: posição, três vetores de orientação e escala. Acrescenta-se a `RelativeLocation` do scatter, sem aplicar a escala das fontes duas vezes. Não foi criada uma nova distribuição aleatória.
- Catálogo de 51 materiais; 11 texturas estáticas recuperadas. O renderer usa as máscaras de hexágono oco/cheio e conserva os restantes ficheiros no catálogo. As 24 referências a RenderTarget não equivalem a imagens recuperadas; 21 referências SubstanceTexture2D continuam sem conteúdo exportado.
- MASTER PCM original, preservado no WAV completo. Opening enfileira apenas os primeiros 120 s; Sequence enfileira 424 s. A fila de amostras SDL determina o relógio visual. A latência física do dispositivo e de Virtual Desktop não é compensada.

As fontes e hashes encontram-se em `source-data/`, `material-evidence/`, `assets/opening/scene.json`, `assets/opening/materials.json` e `assets/audio/provenance.json`. Os executáveis/PAK do Steam e a instalação UE 4.13 não foram alterados.

## Comportamento implementado

Opening apresenta Intro/Dome desde o início, Forest a partir de 80 s, o grupo ForestGround aos 112 s e remove Intro aos 112 s. O Dome é removido aos 80 s. Câmara e névoa seguem as curvas recuperadas, com interpolação Linear/Constant/Cubic confirmada por REA. A câmara conserva a posição inicial até aos 112 s.

`--scene opening` é o padrão e para áudio e relógio aos 120 s, mantendo o último cenário visível. `O` seleciona e reinicia Opening; `Q` reinicia a sequência selecionada; Espaço pausa áudio, animação e deslocamento. `--scene sequence` mantém Opening até aos 120 s, transita para as cenas anteriores entre 120 e 128 s e continua até aos 424 s. Esse segmento posterior conserva as limitações das versões anteriores.

JSON v1 mantém os parâmetros e transições. Na cena nova foram verificados efeitos gráficos de pulsação, segunda cor da paleta e densidade. Densidade zero exclui todas as instâncias de Opening; o chão e o céu mantêm-se. Postes e árvores usam limiares de emissão separados, coerentes com as alturas das malhas.

## Comparação visual e limites

A referência é o vídeo fornecido pelo utilizador: https://www.youtube.com/watch?v=Topz0NkGIus. Foram revistos os fotogramas aos 51, 75 e 125 s do vídeo; o deslocamento candidato para o relógio da experiência é aproximadamente 6,1 s e não foi calibrado por alinhamento áudio. As capturas locais estão em `captures/`, aos 40, 75, 96 e 119 s da experiência, com vista frontal do monitor. A pose da cabeça no vídeo difere e não permite equivalência pixel a pixel.

As capturas confirmam hexágonos, pontos e barras na introdução e formas triangulares com faixas brancas em Forest. A versão anterior representava este segmento com outras formas e cores. **As capturas também mostram diferenças ainda por resolver; não se declara fidelidade integral nem aceitação visual pelo utilizador.**

Padrões de emissão e pulsação são reconstruídos em shaders próprios. Não foi recuperado o resultado dos RenderTargets nem o processamento Substance, e a ligação completa entre a música, os mapas dinâmicos e cada objeto continua por implementar. Em particular, a máscara temporal das barras, os pontos sobre o solo e a intensidade relativa das luzes ainda diferem da referência. O chão/reflexos e o céu também são próprios.

A âncora herdada dos contentores é assumida identidade e o consumidor original local/world ainda não foi confirmado. Usa-se a variante de Preview quando difere do HISM herdado; essa escolha exige confirmação no runtime original. Intro Lower/Grass não têm linhas nas tabelas selecionadas e não foram inventadas. O catálogo de dependências não prova visibilidade a cada instante.

## Validação

`verification.log` regista compilação Release, oito testes C++ e 34 testes Python aprovados. Inclui renderização D3D11 real, pausa/relógio áudio, limites de Opening, JSON e imagens dos dois olhos num runtime OpenXR simulado. O modo de teste `native-resolution` rejeita swapchains que não sejam 640×360, a resolução recomendada pelo runtime simulado; assim valida a resolução efetiva de 1,0.

O teste do fim reproduziu primeiro um consumo excedente de 39 ms (`ending-red.log`). A correção limita a fila PCM ao intervalo selecionado, em vez de apenas travar o relógio visual. O teste verifica áudio e visual exatamente aos 120 s. Um timeout isolado do teste de integração passou na repetição (`integration-recheck.log`) e a suite completa voltou a passar.

A revisão independente aprovou as correções. A validação do ZIP extraído fica em `package-check.json`. O headset físico ainda não foi testado nesta versão. Os testes confirmam funcionamento e regressões; não confirmam reprodução idêntica do original.
