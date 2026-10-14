# Materiais da abertura: recuperação estática

Foram recuperadas as propriedades etiquetadas de 51 materiais e 11 texturas PNG originais. O catálogo segue as referências `Sources` dos emissores Intro, Pyramid e Forest e dos cinco emissores que compõem TubeIntro, bem como os materiais explícitos de ForestTerrain. A presença no grafo de dependências não prova visibilidade em cada instante dos primeiros 120 segundos.

`assets/opening/materials.json` conserva overrides escalares/vetoriais, pais, referências tipadas, valores predefinidos efetivamente serializados nas expressões e SHA1 de cada pacote. `semantic_materials` fornece aliases explícitos; `expression_defaults` distingue valores serializados de valores ausentes/desconhecidos. `metadata.json` contém 180 pacotes com payload SHA1 verificado a partir do PAK aberto exclusivamente para leitura. Não foi executado o jogo original.

| Papel | Material/alias | Recurso e parâmetros comprovados |
|---|---|---|
| Hexágono oco da introdução | `intro_hex_hollow` → `m_hex_hole_1_map` | `Detail` → `hex_hollow_pattern_256_2`, PNG 256×256: contorno hexagonal constituído por barras horizontais brancas. Tiling U/V=1, Color=(0.2,0.2,0.2,1), Mapping→RT_Hex_3 dinâmico. |
| Hexágonos sólidos da introdução | `intro_hex_solid_1/2` | `Detail` → `hex_pattern_128`, PNG 128×127: hexágono sólido constituído por barras horizontais brancas. Tiling U/V=1, Color=(0.303,0.303,0.303,1), Mapping→RT_Hex_1/2 dinâmicos. |
| Cinco superfícies TubeIntro | `tube_intro_0..4` | Material `M_Detail_Basic_Map`, Tiling U/V=64, Color=(0.1,0.1,0.1,1), Mapping→TubeIntro0..4_RenderTarget. O pai possui Detail→Circle_256_8; isto não identifica o padrão produzido pelo render target. |
| Composição do TubeIntro | `m_white_basecolor` | Emissores SpawnerTubeIntro0..4 usam BP_PlaneSolid_White_BaseColor. Pai M_BaseColor, Color predefinido=(1,1,1,1) serializado. |
| Chão piramidal | `pyramid_ground` | Detail→pattern_pixel_1 (2×2); Tiling U/V=512, Offset U/V=0.25, Map Tiling U/V=4, Emission=32, GI Emission=64; Mapping→Theme_RenderTarget dinâmico. |
| Terreno piramidal | `pyramid_terrain` | M_Pyramid_Theme referencia Theme_RenderTarget, pattern_pixel_1 e cinco SubstanceTexture2D não suportadas. Emission=20 e GI Emission=40 serializados; TileU/V e OffsetU/V ausentes/desconhecidos. |

O pai M_Detail_Basic_Map conserva Emission=20, GI Emission=40, Metallic=1, Roughness=0.2 e Color=(1,1,1,1) como valores serializados. Os overrides dos filhos prevalecem; não se infere a fórmula do programa de sombreamento a partir destes nomes.

Existem 24 TextureRenderTarget2D que geram composições durante a execução; não constituem PNG estáticos recuperáveis. As 21 SubstanceTexture2D têm referência e SHA1 conservados, mas UModel não suporta a classe e os seus caminhos PNG ficam nulos. O comportamento dos programas de sombreamento compilados e as alterações de parâmetros durante a execução permanecem desconhecidos. Não foram criadas texturas sintéticas com aparência de recursos originais.

O helper `tools/export_opening_materials.py` usa pacotes qualificados `/Game/...`, `-game=ue4.13`, `-dump` e `-export -png`, sem `-notex`. As 166 chamadas têm argumentos, códigos de saída e hashes dos registos em `umodel-calls.json`. A pasta de exportação conserva `.props.txt`/`.mat` como evidência de parâmetros, não como reconstrução dos programas de sombreamento UE4. Cada execução remove apenas os PNG esperados antes do lote e exige novas saídas Texture2D, impedindo que um ficheiro antigo seja contado como exportado.

Verificação: 11 PNG com assinatura, CRC de todos os blocos e descompressão zlib válidos; chaves únicas; aliases existentes; hashes de fontes, módulos auxiliares, catálogo, PNG, metadados e registos coincidentes. Inspeção visual efetuada nos dois padrões hexagonais. A revisão independente confirmou SHA1 dos 180 payloads diretamente no PAK; o aviso de reutilização de cache foi corrigido removendo `--skip-umodel` e invalidando os PNG esperados antes de cada lote.
