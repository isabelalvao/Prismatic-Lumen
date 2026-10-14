# Banco de geometria recuperada

**67/67 StaticMesh seleccionadas foram recuperadas e validadas.** Há 66 exportações novas e uma cópia verificada do ensaio SM_Lower_1; não foi repetida essa exportação. O banco contém 134 ficheiros glTF/BIN, totalizando 1382485 bytes, com 19707 vértices de accessors de posição e 23345 triângulos. Não houve recursos não suportados, falhas de validação ou avisos de geometria.

[manifest.json](C:/ORCA/Fantasynt/plan/visual-resource-bank/manifest.json) permite localizar cada recurso pelo caminho original, nome, SHA1 do payload, glTF relativo, ficheiros BIN e SHA-256. [index.tsv](C:/ORCA/Fantasynt/plan/visual-resource-bank/index.tsv) é um índice compacto; [validation.json](C:/ORCA/Fantasynt/plan/visual-resource-bank/validation.json) contém as verificações de geometria. Os argumentos, códigos de saída e logs por recurso estão em `logs/`; `inputs.json` conserva os relatórios de entrada e o critério de selecção.

## Selecção e execução

Foram seleccionados apenas exports de classe `/Script/Engine.StaticMesh` dos relatórios `mesh-assets.json` e `geometry-controller-assets.json`. Quando este último não fornecia `class_reference`, a classe foi resolvida pelos índices e cadeia de outer dos imports/exports, em vez de inferida pelo nome do ficheiro. Os dois relatórios fornecem 67 recursos únicos: as 66 malhas do primeiro e `forest_content_green_fixed` adicional. Os oito restantes StaticMesh encontrados no segundo são duplicados já cobertos.

O perfil dos pacotes foi verificado como cooked UE4 legacy -6, versões UE4/licensee zero e sem custom versions serializadas. Todos os 67 payloads foram comparados ao SHA1 do índice do PAK **antes e depois** das exportações. Não foi executado o jogo.

Ferramenta: UModel oficial do repositório `gildor2/UEViewer`, gerador glTF identificado como build1590; executável local SHA-256 `13502e5a4d8f6b5f32252afebd6360f7302ccfaccf6b8dda65beff0be2d364a0`. O primeiro ensaio usou o pacote completo `/Game/_HelloEnjoy_/ChezNous/Scatter/Meshes/SM_Lower_2`, objecto `SM_Lower_2`, classe `StaticMesh`, com `-export -gltf -notex -game=ue4.13`, `-path` da instalação Steam e `-out` dentro deste banco. O ensaio e os limites de buffers/accessors passaram antes do lote; `qualified-test.json` e o respectivo log preservam essa prova. Todos os recursos seguintes usaram igualmente pacote, objecto e classe explícitos, evitando ambiguidades de basename.

SM_Lower_1 conserva os hashes do ensaio anterior: glTF `24e2f57aed999840cf20affc7c608bef97888bc3952e5ef9530a1fe60ec51c6d`, BIN `7d43c6dc6c9f092695a1e8d64bb060c03cad5e974d8f4bd6de52c8045d03eee1`. A identidade de pacote e payload SHA1 foi verificada antes de copiar os dois ficheiros para este banco. A cópia conserva a proveniência do export anterior.

## Validação

- glTF2.0, nome de objecto correspondente ao export escolhido e URI de buffer local confinada ao banco;
- comprimento de BIN, limites de bufferView/accessor, alinhamento, stride e formatos suportados;
- valores finitos em todos os accessors descodificados e transforms de nodes;
- triângulos com índices dentro do número de posições, contagem múltipla de três e formatos de índice válidos;
- UV, normais e tangentes presentes, com dimensões/contagens correspondentes às posições;
- comprimento unitário das normais/tangentes e sinal W das tangentes válido;
- **67/67 extents** calculados dos valores binários de POSITION correspondem aos bounds UE serializados, com tolerância `max(0.001 cm, |valor|×1e-5)`;
- **52/52 centros explicitamente serializados** também correspondem após a permutação dos eixos. Para os 15 bounds sem Origin explícito, não se assumiu zero nesta comparação.

A comparação usa a permutação UE `(X,Y,Z)` → coordenadas glTF `(X,Z,Y)` e centimetros → metros. Os valores da geometria binária, e não os min/max arredondados do JSON emitido pelo exportador, determinam os resultados. A concordância dos bounds confirma dimensões e centros disponíveis; não estabelece transforms globais, orientação de instâncias, winding original ou colocação no mapa.

## Exemplos de geometria real

| Recurso | Vértices | Triângulos | Extent em cm, nos eixos glTF |
|---|---:|---:|---|
| SM_Lower_1 | 121 | 216 | `[43.30126643180847, 12.50000074505806, 43.3012530207634]` |
| SM_Hex_Hole_1 | 148 | 144 | `[48.493701219558716, 7.999999821186066, 43.3012992143631]` |
| SM_Hex_Solid_1 | 74 | 70 | `[48.493701219558716, 7.999999821186066, 43.3012992143631]` |
| SM_Tube_Intro_0 | 27 | 30 | `[1.9999980926513672, 25.999999046325684, 1.7320513725280762]` |
| alpha32 | 168 | 144 | `[15.915507823228836, 50.0, 7.957743341103196]` |
| SM_Tree | 49 | 24 | `[24.9999962747097, 800.0, 24.999991059303284]` |
| forest_content_green_fixed | 331 | 257 | `[50.0, 50.0, 0.03744999994523823]` |

Estas são malhas provenientes dos recursos distribuídos, com posições/índices/atributos recuperados. Os nomes não foram usados para inventar ou classificar formas geométricas. Podem substituir hipóteses de primitivas na etapa de reconstrução, mas este banco não foi integrado no produto.

## Limites e integridade

Foi recuperado o LOD exportado por omissão; `-lods` não foi solicitado e não se declara recuperação de todos os LODs. `-notex` produz materiais de substituição, pelo que **materiais, shaders, texturas e aparência final do jogo não estão recuperados neste banco**. Geometria correcta, por si só, não demonstra fidelidade da cena, distribuição Scatter, iluminação, animação ou efeitos.

`verification.json` regista a nova comparação dos 67 payloads de entrada, a verificação dos hashes dos 134 outputs, a invariância do tamanho/data do PAK e do SHA-256 da ferramenta. Nenhum ficheiro do produto, UE instalado ou recursos originais foi modificado; não foram iniciados REA/Ghidra, browser ou jogo. Não foi escrito um guião persistente nem instalada qualquer dependência.
