# Referência visual — vídeo fornecido pelo utilizador

Fonte: https://www.youtube.com/watch?v=Topz0NkGIus — FANTASYNTH: Chez Nous - VR Music Experience - Oculus Rift, EkosVR. Vídeo 430.301s, reprodução observada a 1280×720 no navegador. Observações em observations.json; estas são leitura visual do vídeo, não REA Evidence.

## Método e limites

Menu inicial ignorado como alvo de reconstrução. Aos 5s ainda há PRESS ANY BUTTON TO START; aos 6.076s o menu já desapareceu. A diferença vídeo–MASTER é 6.119s: offset temporal candidato, coerente com o arranque mas não calibrado por alinhamento de áudio. Os tempos abaixo são do vídeo, não da LevelSequence. As observações cobrem pontos nas fases, não todas as imagens em reprodução contínua.

O utilizador vira a cabeça durante a gravação. Inclinação do horizonte, mudança do ponto de fuga, saída de objetos do enquadramento e vistas para cima/para o chão não devem ser transformadas automaticamente em animação do mundo. Posição/orientação exatas do HMD não foram recuperadas. Para comparação pixel a pixel seria necessária uma pose equivalente; a reconstrução VR deve conservar a liberdade do HMD atual.

Alguns seeks devolveram readyState1 e uma imagem anterior. Esses previews foram descartados; os pontos foram revisitados depois de carregar readyState4. Não se descarregou o vídeo, não se alterou o jogo e não se abriu o menu como referência de produto.

## Diferenças concretas da versão 0.4.2

| Tempo aproximado do vídeo | Observação | Correção exigida |
| --- | --- | --- |
| 00:10–01:15, vistas selecionadas | Introdução escura com pontos, barras, contornos hexagonais e módulos baixos; aos 1:15 o solo e os objetos próximos são visíveis. | Reconstruir IntroScene/IntroDome próprios; não substituir toda esta fase pelo campo de leques do protótipo. |
| 02:05 | Formas altas triangulares com chevrons brancos, faces escuras e reflexos. | Usar as malhas/fontes originais e seus padrões nesta fase; o protótipo já mostra cones com círculos RGB. |
| 02:35 | Padrões de faixas concêntricas e centros escuros, dominância branco/laranja, ambiente azul escuro. | Aplicar padrões/cores ligados à sequência; não manter a mesma paleta multicolorida durante toda a paisagem. |
| 03:05 | Módulos separados em altura, faces inferiores luminosas, faixas coloridas nas regiões baixas, barras brancas sobre faces azuis. | Recuperar composição e disposição; não representar todos os objetos como um cone redondo de sete anéis com círculos em toda a superfície. |
| 03:35 | Vista negra. | Verificar segmento contínuo/consumidores antes de atribuir o apagamento a uma transição global; a pose isolada não resolve a causa. |
| 04:35 | Túnel com superfície sólida irregular/facetada escura, anéis e blocos/padrões brancos. | Reconstruir paredes/terreno e módulos originais; a coleção de objetos luminosos do protótipo é insuficiente. |
| 05:05 e 05:35 | Túnel evolui para cores violeta/laranja/vermelho, com padrões largos e faces escuras. | Aplicar cores e padrões temporais aos materiais, além de apenas mudar a cor da névoa. |
| 06:05 e 06:35 | Vistas diferentes da região superior/túnel, azul/preto/branco com pontos e anéis. | Separar pose da fase; não concluir que a arquitetura mudou só porque o utilizador olha para cima. |
| 07:05 | Créditos CREATED BY HELLOENJOY sobre um túnel ainda visível e luminoso. | O escurecimento global inventado em 416–424s não está demonstrado por este fotograma; validar comportamento do final. |

A referência confirma diferenças de estrutura, composição e materiais no cenário principal. Não são diferenças explicadas apenas pelo ângulo da cabeça ou por calibração de cor.

## Recuperação adicional efectuada neste turno

Banco plan/visual-resource-bank: 67 StaticMesh selecionados, 67 glTF/BIN exportados/reutilizados e validados, 19,707 vértices/23,345 triângulos; 67 extents conferidos com os metadados UE4. UV, normais, tangentes, intervalos de buffers e índices verificados. 66 novos exports e 1 amostra reutilizada com hashes. Materiais são placeholders, LOD default apenas. Isto resolve a geometria destes recursos selecionados; não equivale a recuperar todas as instâncias, materiais ou o cenário final.

O produto 0.4.2 não foi alterado neste turno. O próximo passo visual deve integrar este banco e os materiais/composições associados, usando as fases e a referência deste vídeo como critério de aceitação. Não produzir outra versão apenas por passar testes funcionais.
