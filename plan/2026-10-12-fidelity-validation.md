# Validação visual 0.3.0

O utilizador confirmou que foi possível ver a versão 0.2.0 em VR no Quest 3 via Virtual Desktop e que os comandos funcionam. Este trabalho concentra-se nos visuais da versão 0.3.0; não pressupõe aprovação da nova apresentação nem medições físicas de desempenho.

## Alterações e referências

A cena inicial passou do campo com volumes semelhantes a prédios para um túnel radial. Mantém 4096 elementos preparados, repartidos por painéis facetados com bordos arredondados, formas arredondadas com LED, arcos, fitas curvas e pequenas luzes. Todas as malhas são geradas no projeto. Os pontos, bandas e traços luminosos usam padrões próprios animados, com transições pela paleta JSON. A predefinição energética passou a azul/laranja-vermelho; calma e equilibrada conservam paletas frias.

Iluminação difusa e reflexo de cor estilizado tornam visíveis as carcaças escuras, separadas das regiões emissivas. A atmosfera e a luz quente axial usam direção/posição do olho em espaço do mundo. A seleção T/F e --scene tunnel|field conserva a cena anterior para comparação. O contrato JSON e a implementação OpenXR foram preservados.

Referências inspecionadas: cinco screenshots oficiais Steam, captura do trailer oficial de 40 s (contact sheet descarregada e convertida para PNG para inspeção) e capturas do protótipo. Fontes e manifesto em reference; os recursos de referência não são distribuídos no executável. A primeira captura desta integração ficou preta devido à associação do uniforme de orientação ao draw errado; foi corrigida antes da entrega. As iterações seguintes reduziram brilho, separaram carcaças/painéis e variaram os desenhos luminosos.

A análise estática aprofundada leu 19 recursos do PAK UE4, validando hashes e propriedades. Materiais do painel referenciam detalhe denso, render target de mapping e carcaça metálica escura; nevoeiro tem parâmetros próprios de opacidade e dimensão. [Evidência](fidelity-assets/findings.md). Shaders originais, conteúdo dos render targets, buffers das malhas e bytecode Blueprint continuam desconhecidos. Formas arredondadas e arcos são escolhas de reconstrução baseadas nas imagens, não meshes recuperadas do original.

## Comparação visual

[Captura final energética](fidelity-tunnel-final.png): profundidade axial e elementos em redor do observador, contornos/pontos azuis próximos, cor quente no fundo, superfícies escuras e emissão localizada. Corrige a composição de avenida da primeira versão. [Captura calma](fidelity-tunnel-calm.png): reduz emissão, pulso e densidade, mantendo o túnel. [Campo anterior](fidelity-field.png): disponível para comparação, sem ser apresentado como fase fiel reconstruída.

Diferenças ainda visíveis face a reference/steam-0.jpg: formas mais regulares e menos diversas, materiais e iluminação mais simples, luz central aproximada, ausência dos painéis de vídeo originais, coreografia da música e das transições entre os ambientes. Não foi demonstrada reprodução integral do Fantasynth; a avaliação estética final cabe à comparação no headset.

## Verificação

- Geometria: falha observada antes da implementação em fidelity-geometry-red.log. Verifica distribuição à volta da cabeça, espaço central, semente reproduzível, limite de elementos, normais, índices e costuras de superfícies fechadas.
- Renderer: redimensionamento, dimensão zero, comandos visuais, avanço e alternância entre cenas. A comparação após restaurar o túnel encontrou seis canais a diferir um nível em 691200 canais RGB; o teste passou a comparar pixels descodificados com tolerância máxima de um nível e média inferior a 0,001. Não se usa igualdade de ficheiros PNG nem esse teste como prova de fidelidade.
- Revisão independente: nenhum bloqueio no produto; corrigida leitura além de um payload na ferramenta de investigação. Novos testes negativos falharam antes da correção (fidelity-inspector-red.log) e passaram depois. Os 19 recursos foram reanalisados com limites estritos, zero erros de propriedades e hashes válidos (fidelity-assets-recheck.log).
- CTest 5/5 e Python 18/18: registos fidelity-ctest.log e fidelity-python-final.log. As seis verificações OpenXR continuam a usar texturas D3D11 reais inicialmente pretas, com conteúdo e hashes diferentes nos dois olhos.
- Pacote 0.3.0: extraído numa pasta com espaços/acentos, testado em monitor e OpenXR simulado, incluindo o cliente JSON PowerShell e hashes do manifesto. Evidência em fidelity-package-check.json. ZIP 0.2.0 mantido intacto para comparação.

Não há teste físico desta nova cena nesta máquina. Não converter FPS do runtime simulado em desempenho VR real. A próxima avaliação deve verificar legibilidade dos pontos LED, contraste/brilho, escala das formas e estabilidade no Quest 3, além da semelhança visual.

Pacote final: 2850474 bytes; SHA-256 `71791331be3ae24a37f448025ebaeaef545e08637c25cbf4e0df671967ddac8f`, conferido contra o ficheiro .zip.sha256. Backup de fontes em plan/backups/fidelity-0.2.0. A remoção da ferramenta temporária de descodificação do trailer em temp/fidelity-tools foi recusada pela revisão automática com «blocked by policy», tanto na operação recursiva como na alternativa por ficheiro; a pasta foi conservada. Não entra no ZIP nem é uma dependência do produto.
