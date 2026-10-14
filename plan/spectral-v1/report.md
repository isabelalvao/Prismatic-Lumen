# Cones espectrais e cristais — 0.10.0

Entrega concluída em 2026-10-14. O utilizador pediu substituir os cones e as folhas digitais pelas referências enviadas e confirmou a construção dos cones peça a peça, da base à ponta. Determinou também que nenhuma forma complexa pode ter animações de escala.

## Formas e coreografia

Os cones anteriores deixaram de ser preparados ou desenhados no runtime. Os novos têm base larga e ponta superior, com nove conjuntos de anéis e oito nervuras por conjunto, em ciano e magenta. São tubos tridimensionais fechados, não um padrão aplicado às superfícies dos cones anteriores. A construção começa aos 136 s, com intervalos de 2 s, fade de 0,8 s por peça e desfasamento por cone até 2 s. Cada peça nasce com a dimensão final. Depois de completar o cone começa a rotação comum de 2 graus/s; não há a broca, separação ou contrarrotação de módulos das versões anteriores.

As folhas deram lugar a seis variantes determinísticas de aglomerados de cristais, cada uma com nove prismas hexagonais facetados e pontiagudos. Variam altura, inclinação, orientação, posição e distribuição. Entram a partir dos 172 s, com intervalo de 0,65 s, fade de 1 s por prisma e desfasamento por aglomerado até 3 s. As novas malhas têm espessura volumétrica, considerada antes de desobstruir o corredor. Os materiais desenham facetas e arestas luminosas; não simulam refração física de vidro.

Os controlos de densidade, entradas e saídas passam a usar opacidade, mantendo as proporções das malhas. A montagem do túnel também deixou de fazer crescer as suas peças. Os mini-pilares movem-se rigidamente; uma extensão fixa de haste a dez metros abaixo do chão mantém o contacto com o piso, sendo recortada no passe principal e no reflexo. A altura mínima limita a translação, sem alongar a haste. Tamanho configurável e modelação estática não são animações de escala.

Mantêm-se 360 s de experiência, arranque imediato, pirâmides aos 80 s, contador desligado até C, fusão binocular da versão anterior, retirada 208–224 s, túnel 224–272 s, luz 272–284 s e dispersão 270–300 s. Não foi criada uma cena posterior aos anéis.

## Transparência

A revisão identificou uma regressão importante na primeira integração: peças em fade eram desenhadas por família antes do piso, que podia apagá-las. A implementação final desenha todas as malhas opacas e o piso antes de uma fila global de transparência, ordenada por olho e profundidade real da câmara. Só entradas consecutivas com o mesmo material/malha são agrupadas. Peças transparentes não escrevem profundidade; a classificação usa a mesma opacidade transportada ao shader, incluindo densidade e saída das malhas nativas.

O teste gráfico específico desliga reflexos e bloom, coloca cones com opacidade inferior a um sobre o piso opaco e verifica que alterar a emissão dos cones altera a imagem. Passou após a correção. Capturas transparency-red.png e transparency-green.png preservam o diagnóstico, aos 156 s.

## Configuração

Há 211 opções tipadas, 18 cores, os sete parâmetros globais e áudio. Foram acrescentados brilho dos cones, intervalo/revelação/desfasamento dos cristais, número de variantes, brilho de arestas e facetas, espessura das arestas e quatro cores. O guia documenta campos e valores; guardar o JSON ou F5 permite afiná-los sem recompilar.

As afinações existentes foram conservadas. Os campos cones.emerge_seconds, cones.separate_seconds, cones.drill_rotation_degrees_s e cones.module_gap_m são aceites por compatibilidade, mas ignorados nesta coreografia. cones.segment_interval_seconds voltou a controlar a construção das novas peças. O áudio MASTER e os recursos recuperados em disco não foram alterados.

## Verificação

- 17 testes CTest e 45 testes Python aprovados na versão com a fila global corrigida; registo transparency-verification.log. O ensaio gráfico foi repetido depois de acrescentar a regressão do piso e o modo de prévia; registo renderer-final.log.
- Testes de geometria verificam limites, determinismo, variantes distintas, superfícies fechadas, orientação exterior, normais, UV e ausência de triângulos degenerados. Cone: 567 vértices e 2304 índices por peça. Prisma: 61 vértices e 144 índices por peça.
- Testes dos postes demonstraram primeiro a deformação antiga e depois verificaram matriz preservada, distâncias constantes entre vértices, translação uniforme, limites de altura e continuidade na recarga de período/variação.
- Revisão independente aprovada após corrigir a transparência, sem problemas elevados ou críticos pendentes. Revisão estática e testes técnicos não substituem a avaliação artística no headset.
- ZIP extraído num caminho com caracteres portugueses passou em desktop, OpenXR simulado e cliente JSON PowerShell, incluindo fases dos novos cones, cristais e túnel. Hashes do manifesto válidos: package-check.json e package-check.log.

Foram inspecionados fotogramas da construção e do resultado dos cones e cristais. cones-preview.mp4 mostra 80 fotogramas a quatro imagens/s, durante 20 s, dos instantes 136–155,75 s. crystals-preview.mp4 mostra 48 fotogramas, durante 12 s, dos instantes 172–183,75 s. Ambas são renderizações reais, com câmara fixa, sem áudio/pulsos e sem acelerar o tempo. Os fotogramas ficam em cone-frames/ e crystal-frames/.

## Entrega e preservação

Pacote: dist/Fantasynt-PCVR-Win11-x64-0.10.0.zip.

SHA256: b7fa18890b67e23a2c0d082479b78b8ec18ce4dcefbded2cad0b9cf493704b31.

ZIP 0.9.1 preservado: 314f94b65ac7aa3c287d708a219c3d51845f2bfe57d356da898cf05a8230f566. MASTER preservado: 60dc74309ab1fbae42394228ddfc098a09a1e7a36ae739c7f5ee7e1adafb9fd9. Capturas do utilizador, recursos recuperados, Steam, UE 4.13 e outras entregas conservados. Cópias dos ficheiros alterados em backup/; artefactos da compilação isolada foram conservados em isolated-tests/.

Alterações em spectral_geometry, surface_choreography, renderer, shaders, movimento dos postes, configuração, testes, compilação, empacotamento e documentação. Não existe repositório Git e não foi inicializado. O Quest 3 físico não foi validado nesta entrega.
