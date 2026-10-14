# Opening até 240 s — proposta musical e visual

## Recomendação

Manter a floresta como continuidade espacial depois dos 120 s e desenvolver quatro movimentos: aquecer a paleta aos 144 s, revelar módulos baixos e camadas em altura aos 176 s, rarefazer a emissão entre 208 e 224 s, e abrir uma nova arquitectura no regresso dos ataques aos 224 s. São propostas de encenação para a extensão, apoiadas em recursos e tempos recuperados; não são uma afirmação de que o original fazia exactamente estas transições.

| Âncora musical | Transição proposta | Formas e paleta | Duração de leitura |
| --- | --- | --- | --- |
| 144 s | 136–148 s, 12 s | Ganhar presença dos diamantes, pirâmides ocas e padrões circulares já recuperados; azul escuro de fundo, laranja e branco nos desenhos. O aquecimento dos materiais pode concentrar-se em 142–144 s para conservar a chave original, com a entrada de formas mais lenta. | Sustentar a composição até 172 s. |
| 176 s | 172–184 s, 12 s | Dar protagonismo às malhas SM_Lower_1..6, com separação em altura e faces inferiores luminosas. Retomar o azul nas superfícies e manter laranja nas faces internas; acrescentar um halo quente discreto, inspirado na névoa original que fica vermelha. | Sustentar módulos/camadas até 208 s, com mudanças pequenas orientadas pelos ataques. |
| 208 s | 208–220 s, 12 s | Retirar progressivamente luz das faces internas e diminuir a densidade luminosa; preservar silhuetas, chão e alguns pontos de orientação. A forma deve permanecer legível enquanto a música perde ataques e energia. | Entre 220 e 224 s, quase só contornos e luz residual, sem corte instantâneo. |
| 224 s | 224–232 s, 8 s | Regresso dos ataques: revelar arcos, anéis e volumes facetados em profundidade, azul/preto/branco com pequenos acentos laranja. Preferir malhas recuperadas e a continuidade do espaço existente. Se for usada a cena de túnel anterior, declarar que essa composição conserva limitações anteriores. | Manter esta arquitectura visível 232–240 s; não terminar com outra mudança abrupta. |

As formas propostas devem alternar protagonismo, não desaparecer todas na mesma imagem. A pulsação pode actuar no brilho e em pequenos deslocamentos, mas a silhueta geral deve permanecer estável durante várias batidas. As formas em si já existem entre as malhas integradas de Opening; a revelação gradual de famílias constitui nova direcção artística. Uma arquitectura nova pode reutilizar o banco recuperado sem reclamar que a sua distribuição é original.

## Evidência recuperada

- `plan/full-reference/timeline.json` e `findings.md`: DirectorSection tem chaves em 112, 144, 176, 208 e 224 s. Os consumidores não foram totalmente traçados, pelo que as chaves são âncoras temporais, não demonstração de todos os efeitos.
- MappingColorController: RGB azul `(0.25, 0.45, 1)` em 142 s, laranja `(1, 0.45, 0.25)` em 144 e 176 s, azul em 178 s. São valores do controlador, não códigos sRGB para copiar directamente como cor final do monitor.
- InsideColorController: branco em 176 s, laranja `(1, 0.437, 0.25)` em 178 s, preto em 208 e 224 s. A interpretação como emissão das faces internas continua a depender do consumidor/material.
- FogInscatteringColor: `(0, 0.122, 0.247)` em 144/175 s, `(0.319, 0.001, 0.003)` em 177 s, `(0.991, 0.033, 0.015)` em 207 s, preto em 208/224 s. Usar a tendência frio→quente como orientação, não copiar a intensidade UE4 para o renderizador próprio.
- `plan/video-reference/findings.md`: no vídeo fornecido, aos 2:35 aparecem faixas concêntricas, centros escuros e branco/laranja sobre azul escuro; aos 3:05 há módulos separados em altura e faces inferiores luminosas; aos 3:35 existe uma vista negra cuja causa não foi demonstrada. Estes são tempos de vídeo e têm deslocamento candidato de aproximadamente 6.1 s relativamente ao MASTER; não foram usados como alinhamento exacto.
- `assets/opening/scene.json`: contém diamantes, pirâmides ocas, SM_Lower_1..6 e tubos; `plan/visual-resource-bank/manifest.json` permite localizar as malhas recuperadas adicionais. A geometria e as transformações da tabela são evidência; texturas dinâmicas, materiais finais e coreografia integral não foram recuperados.

## Medição do MASTER, 110–240 s

Fonte: `assets/audio/ChezNous_MASTER.wav`, 44100 Hz, estéreo PCM16, duração medida de 424.1821315 s. O ficheiro não foi alterado. `metrics.json` conserva janelas de 100 ms e resumos de 2 s.

Foi medido RMS estéreo em janelas consecutivas de 100 ms; o nível apresentado é a média dos valores dBFS dessas janelas, não LUFS. Para descrever as mudanças de distribuição espectral, usou-se FFT mono de 8192 amostras, janela Hann e avanço de 100 ms; a fracção de graves corresponde a 30–180 Hz e o fluxo corresponde à média das diferenças positivas de potência logarítmica entre espectros. O fluxo serve apenas como indicador relativo de mudanças/ataques; não é um detector validado de batidas. Não houve escuta perceptiva nesta análise.

| Intervalo | RMS médio das janelas | Fracção média 30–180 Hz | Fluxo médio | Consequência defensável |
| --- | ---: | ---: | ---: | --- |
| 112–144 s | -7.59 dBFS | 0.823 | 0.362 | Energia e perfil relativamente estáveis; manter a continuidade. |
| 144–172 s | -7.31 dBFS | 0.810 | 0.377 | Mudança de energia pequena; a mudança de cor aos 144 s deve ser gradual. |
| 176–208 s | -7.38 dBFS | 0.652 | 0.417 | Nível semelhante, menos concentração de graves e mais variação espectral; enriquecer camadas/detalhes. |
| 208–216 s | -10.60 dBFS | 0.966 | 0.117 | Quebra dos ataques e predominância de graves; começar a rarefacção visual. |
| 216–224 s | -24.57 dBFS | 0.977 | 0.073 | Descida pronunciada para luz residual. |
| 224–240 s | -8.83 dBFS | 0.924 | 0.383 | Regresso claro da actividade; nova arquitectura durante oito segundos, seguida de permanência. |

Entre 206–208 s o nível médio é -7.40 dBFS; desce para -9.80 em 208–210, -14.60 em 216–218, -20.22 em 218–220, -26.94 em 220–222 e -36.53 em 222–224. A janela mínima observada é 223.9 s, -42.79 dBFS: **não há silêncio digital demonstrado**. Em 224–226 s regressa a -9.57 dBFS e o fluxo sobe de 0.001 em 222–224 para 0.373. Esta quebra e reentrada justificam muito melhor a última mudança de formas que um corte em 240 s.

## Limites para implementação

As rampas de 8–12 s e a escolha de protagonismo das famílias são decisões artísticas propostas. A duração de Opening, o áudio até 240 s e a continuidade da câmara devem ser implementados e verificados separadamente pelo responsável pelo produto. Esta análise não altera código, configurações de execução, recursos, testes nem pacotes; só acrescenta esta proposta e `metrics.json`.
