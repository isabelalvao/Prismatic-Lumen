# Desenvolvimento 0.4.1 — curvas recuperadas com REA

A versão 0.4.1 substitui aproximações temporais concretas da versão 0.4.0: a névoa passa a respeitar os segmentos constantes, lineares e cúbicos recuperados; a câmara começa na posição da curva correspondente ao tempo escolhido. A pausa congela também a animação da sequência. Mantém a rede, as transições JSON, os recursos próprios e o caminho OpenXR.

## Evidência e aplicação

O REA analisou o executável original com Ghidra e devolveu 22 registos completos de evidência nativa, indexados em `inline-evidence-index.json`. Usou-se o launcher oficial do commit main df2c08b546d4f40103e11b8b31dbfc3d41ee687b, com complemento nativo compilado das mesmas fontes e verificado. A receita Windows é experimental e está documentada separadamente; não se modificou o código upstream. O perfil desativa oito análises globais, conservadas nos dossiers; não representa cobertura integral do programa.

`native-richcurve-eval.json` observa a escolha do modo da chave esquerda e o Bézier construído com as tangentes de saída/entrada. `native-key-serialize.json` corrobora a ordem dos campos e os 27 bytes de conteúdo por chave. Os metadados da sequência preservam precisão float32, incluindo os tempos não inteiros. As 54 chaves usadas foram novamente comparadas com os bytes do PAK original em `native-interpretation/channels-camera-fog.json`. `tools/generate_sequence_curves.py --check` confirma que a tabela C++ corresponde ao JSON recuperado.

A curva CameraRig.Y usa seis chaves lineares. O deslocamento Y é aplicado ao eixo Z do renderizador próprio. Antes de 112 s mantém -6400.00048828125; aos 224 s vale 16000 e aos 424 vale 62400. A extrapolação constante deriva dos defaults do construtor e da ausência de overrides nos canais selecionados. O construtor não foi reconhecido como função no perfil REA: a prova dos bytes 4/4 é uma leitura local limitada, em `native-interpretation/richcurve-constructor-local.json`, identificada como tal. Alterações posteriores do original em execução não foram observadas.

Na sequência, motion_speed=0,3 conserva o avanço da curva; outros valores modulam os incrementos, e 0 imobiliza a câmara sem parar as fases. A retoma preserva a distância perdida e não salta para a posição absoluta. Cenas manuais mantêm o limite de 250 cm/s. O salto por --start-time/--fixed-time posiciona imediatamente a câmara. O renderer continua a repetir geometria a cada 41600 cm; esta alteração não recupera o posicionamento completo das instâncias nem a transformação mundial da câmara original.

## Verificação

- Falhas da versão anterior conservadas em curve-red.log e camera-app-red.log.
- Compilação Release e 6 CTest aprovados em curve-final-tests.log.
- 25 testes Python aprovados em curve-python-final.log; cobre posição no salto, pausa, VR simulado, controlo durante fases, paragem e retoma por JSON. A revisão identificou uma leitura de startup sem prazo no teste; substituída por espera TCP com prazo e novamente verificada.
- Segmento cúbico recuperado 207→208 e chaves sintéticas assimétricas verificados; não se usa smoothstep para névoa.
- Pacote extraído fora da árvore, hashes, desktop, OpenXR simulado e cliente PowerShell aprovados: curve-package-check.json. O runtime simulado não é distribuído.
- Seis capturas reais em curve-40.png, curve-120.png, curve-180.png, curve-207.5.png, curve-228.png e curve-300.png; duas inspecionadas visualmente. São imagens do produto e não observações do original.
- Revisão independente aprovada após corrigir as lacunas de testes. Cópias anteriores em ../backups/rea-curves-0.4.0; ZIPs 0.2.0/0.3.0/0.4.0 conservados. Sem Git.

## Questões da investigação

| Pergunta | Estado |
| --- | --- |
| Serialização e avaliação dos canais selecionados | Respondida por código nativo e recursos; herança dos defaults qualificada. |
| Percurso serializado da câmara | Aplicado; pose mundial e execução original ainda não observadas. |
| Diretores e ocultação de atores | Dois Blueprints completamente descodificados localmente; despacho/subscritores e instâncias continuam parciais. |
| Reprodução LevelSequence | Registo e wrapper corretos confirmados; cadeia interna completa pendente. |
| Geometria, materiais, terreno, distribuição e coreografia | Parcial: recursos e propriedades inventariados; buffers e consumidores ainda por recuperar. |
| Áudio e validação física no Quest | Reprodução/análise de áudio ausente; 0.4.1 apenas validada no runtime simulado. |

Entrega: `C:/ORCA/Fantasynt/dist/Fantasynt-PCVR-Win11-x64-0.4.1.zip`. A conclusão desta correção não fecha o objetivo de fidelidade integral. O próximo trabalho deve recuperar buffers geométricos e consumidores de distribuição/composição, usando o ledger existente para evitar repetir a preparação.
