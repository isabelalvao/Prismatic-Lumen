# Auditoria da reconstrução — 0.4.0

12 de outubro de 2026. Versão pronta para avaliação pelo utilizador; fidelidade integral não concluída.

## Resultado

Uma aplicação própria apresenta campo noturno, paisagem colorida e túnel numa sequência visual de 424 segundos. As cenas também são selecionáveis individualmente. Mantém OpenXR/D3D11 por olho e o contrato JSON v1. A sequência altera a composição e a atmosfera; não submete comandos ao SceneController nem sobrescreve parâmetros, destinos ou revisões do utilizador.

Pacote: `dist/Fantasynt-PCVR-Win11-x64-0.4.0.zip`, acompanhado de SHA-256. Extrair inteiro, abrir Launch-VR.cmd e usar Espaço para iniciar a sequência. N/L/T/F selecionam uma cena; Q reinicia a sequência. Termina com escurecimento aos 424 s. `--start-time` permite começar numa fase e `--fixed-time` fixa animação/câmara para comparação. ZIPs 0.2.0 e 0.3.0 conservados.

## Evidência e reconstrução

| Propriedade | Evidência | Aplicação e limite |
| --- | --- | --- |
| Duração 424 s | PlaybackRange em full-reference/timeline.json | Duração visual; sem áudio. |
| Forest aos 80 s | DirectorSection index 3 e ForestSceneSequencer Start | Inferência de associações estáticas; entrada gradual 80–112 s própria. |
| Intro sai aos 112 s | DirectorSection index 4 e IntroSceneSequencer Stop | O campo noturno representa esta fase; não reproduz integralmente o dome/intro original. |
| Tunnel aos 224 s | DirectorSection index 8 e TunnelSceneSequencer Start | Transição 224–232 s própria. |
| Cor de névoa | FogInscatteringColor em timeline.json | Valores recuperados; interpolação suave e exposição próprias, sem equivalência dos shaders UE4. |
| Malhas e distribuição | Capturas Steam e scatter-summary/mesh-bounds | Leques, cones, placas, diamantes e postes procedurais próprios; não são buffers originais descodificados. |

## Comparação visual

| Fase | Captura do produto | Referência | Diferenças restantes |
| --- | --- | --- | --- |
| Campo noturno | [40 s](sequence-night-40.png) | reference/steam-4.jpg | Leques e contornos curvos, placas, postes e reflexos presentes. Arranjo ainda regular; linhas mais contínuas, profundidade/névoa e materiais menos ricos; chão/reflexos demasiado uniformes. |
| Paisagem | [180 s](sequence-landscape-180.png) | reference/steam-1.jpg e steam-3.jpg | Cones divididos em sete volumes, padrões de cor, superfícies inferiores luminosas e suspensão presentes. Formas menos variadas; falta o terreno facetado completo, nuvens volumétricas e distribuição irregular da referência. |
| Túnel | [300 s](sequence-tunnel-300.png) | reference/steam-0.jpg e steam-2.jpg | Mantém famílias radiais, painéis, arcos e partículas. Recursos/padrões sintéticos; variedade, densidade e coreografia continuam diferentes. |
| Transições | [96 s](sequence-sequence-96.png) e [228 s](sequence-sequence-228.png) | Associações estáticas da sequência | Sobreposição por crescimento/atenuação de geometria, chão com transparência progressiva; não demonstra os movimentos/transições exatos do original. |

Não foi observada a experiência original completa em execução nesta etapa. Os cinco fotogramas oficiais não permitem certificar uma reconstrução integral. Também não se substituiu o alvo de 2017 pela aplicação web de 2026.

## Verificação

- Compilação Release: tools/build.cmd -Test; **6 CTest e 23 testes Python aprovados** em sequence-final-test.log.
- Geometria: distribuição determinística, corredor, índices, valores finitos, normais unitárias e ortogonalidade às tangentes dos leques.
- Sequência: pesos contínuos normalizados em toda a duração, fases/fronteiras, proteção contra tempos inválidos e escurecimento final.
- GPU: capturas reais das três cenas e das transições, parâmetros visuais, redimensionamento, minimização/restauro e regresso determinístico ao túnel.
- Continuidade 231,99 → 232 s: comparação própria, média RGB <0,02 níveis de 8 bits e menos de 0,5% dos píxeis com diferença >2. O teste falhou com o chão temporariamente opaco e passou após restaurar a transparência; registos sequence-opaque-regression-red.log e sequence-boundary-green.log. A igualdade rigorosa mantém-se para restauração da mesma cena.
- JSON: comandos reais durante a entrada no túnel conservam intensidade/velocidade anteriores e revisões; rejeição atómica, mensagens fragmentadas e recuperação de UTF-8 inválido continuam testadas.
- VR: imagens D3D11 reais no runtime simulado para ambas as vistas, novas cenas/transições, pausa inicial, recuperação de tracking e falhas explícitas. **0.4.0 não foi validada num headset físico.**
- Pacote extraído numa pasta temporária com espaços e acentos: hashes verificados, desktop, estéreo simulado, cliente PowerShell JSON e passagem de 224 s aprovados; sequence-package-check.json. O runtime simulado não é distribuído.
- Revisão independente identificou e confirmou correções de oclusão do chão, seleção ativa no relatório e normais dos leques; sem problemas críticos/elevados remanescentes.

## Desempenho local

Cinco execuções de 300 frames, janela oculta, 1920×1080, sem captura e com tempo/câmara fixos, mediram 59,986–59,999 FPS médios na GPU NVIDIA identificada nos registos da aplicação. Cobrem cenas isoladas e duas sobreposições. São cerca de cinco segundos por cenário, limitados a 60 FPS pelo programa; não constituem medição prolongada, latência física ou desempenho no Quest. Dados: sequence-performance.json. As capturas de 12 frames incluem escrita PNG e não são usadas para declarar desempenho.

## Trabalho pendente

Avaliar 0.4.0 no Quest 3/Virtual Desktop; aprofundar referência integral em movimento; reconstruir terreno, distribuição, dome/intro, variedade de recursos e movimentos/coreografia; implementar reprodução/análise de áudio e integração do LLM. A conclusão técnica desta versão não fecha o objetivo de fidelidade integral.

## Atualização0.4.1 —12 de outubro

A auditoria acima descreve0.4.0 e permanece histórica. Na0.4.1, interpolação da névoa e posição/avanço da câmara usam curvas e modos recuperados; não se aplicam já as qualificações de interpolação própria à névoa. A transição de pesos80–112/224–232, exposição final, malhas, instâncias e shaders continuam próprios. A pausa congela animação na sequência. Pacote0.4.1 conservando0.4.0;6 CTest/25 Python e pacote extraído aprovados; headset físico não validado. Evidência/limites em rea-investigation/curve-development.md.

## Atualização0.4.2

Reprodução do MASTER implementada; sequência guiada pelo consumo de amostras SDL. A ausência de áudio descrita acima é histórica das versões anteriores. Pulsação sintética, materiais, formas e distribuição próprias mantêm-se.7CTest/30Python e pacote portátil aprovados; saídaWASAPI abriu e consumiu o áudio, sem medição de latência física nem avaliação noQuest. Ver audio-integration/findings.md.

## Referência integral fornecida pelo utilizador

VídeoTopz0NkGIus analisado em pontos das fases, menu ignorado, pose livre do gravador diferenciada da animação do mundo. Confirmadas diferenças de introdução, forma, padrões, composição, paredes do túnel e cores.67malhas selecionadas recuperadas/validadas em visual-resource-bank; ainda não integradas no produto. O critério visual substitui a mera presença de cenas. Detalhes em video-reference/findings.md.
