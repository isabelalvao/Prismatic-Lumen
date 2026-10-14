# Afinação visual — túnel multicolorido

O utilizador confirmou em Quest 3/Virtual Desktop que a versão 0.2.0 mostra a cena em VR e que os comandos funcionam. É validação funcional reportada pelo utilizador; frequência, latência e fidelidade não foram medidas nessa máquina.

Objetivo desta iteração: corrigir a composição e os materiais da cena, mantendo o renderizador OpenXR e o contrato JSON. Prioridade assumida enquanto se aguarda preferência: túnel multicolorido. Referências: cinco imagens oficiais Steam e trailer oficial de 40 s (manifesto descarregado em reference). A investigação anterior só cobria inventário e imagens; esta passagem analisa propriedades de recursos cooked UE4.13.2 em fidelity-assets.

## Alterações delimitadas

- Acrescentar composição radial de painéis facetados, formas arredondadas, arcos e pequenas luzes; eliminar a leitura de avenida/prédios na cena principal.
- Gerar todas as malhas e shaders no projeto. Não distribuir meshes, texturas, música ou código extraídos do original.
- Padrões LED com pontos, faixas, máscaras e ondas locais; materiais escuros com iluminação difusa e reflexos estilizados. Cor fria próxima, cor quente em profundidade, controladas pelos dois canais da paleta existente.
- Ambiente e nevoeiro espacial, sem efeitos presos ao ecrã. Conservar tracking e escala VR; o movimento de avanço permanece inicialmente pausado.
- Túnel como cena inicial; conservar o campo anterior para comparação através de seleção local de cena. A seleção não altera o contrato JSON.
- Nova versão 0.3.0 com capturas verificadas, regressão desktop/VR/JSON e pacote portátil; manter ZIP 0.2.0 previamente testado pelo utilizador.

## Verificação

Testes das malhas: índices válidos, normais finitas/unitárias, costuras e limites. Testes da distribuição: elementos à volta da cabeça, corredor central livre, orçamento fixo e densidade contínua. Inspeção visual obrigatória das capturas em diferentes instantes/cores; não usar diferenças de PNG como prova de fidelidade. CTest, integração JSON/OpenXR simulado e pacote extraído. Revisão independente do código do produto e da ferramenta de investigação. Não declarar reconstrução integral nem desempenho VR físico desta nova versão sem evidência.

- [x] Propriedades do original e comparação de referências.
- [x] Geometria radial e malhas próprias.
- [x] Materiais/LED/cor/atmosfera e inspeção de capturas.
- [x] Regressão, revisão e pacote 0.3.0.

Entrega: dist/Fantasynt-PCVR-Win11-x64-0.3.0.zip e checksum SHA-256. CTest 5/5, Python 18/18 e pacote extraído verificado em desktop/OpenXR simulado com cliente PowerShell. 19 recursos reanalisados após correção dos limites internos do inspector. A nova composição ainda precisa de avaliação visual no headset; diferenças e limites em 2026-10-12-fidelity-validation.md. O ZIP 0.2.0 funcional previamente testado permanece disponível.

Backup das fontes alteradas: plan/backups/fidelity-0.2.0. Pasta sem Git; não criar repositório nesta tarefa.
