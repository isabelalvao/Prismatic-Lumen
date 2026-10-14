# Evidência estática de materiais e geometria do Fantasynth

Fonte autorizada: `C:\Program Files (x86)\Steam\steamapps\common\Fantasynth\Fantasynth\Content\Paks\Fantasynth-WindowsNoEditor.pak`.
19 recursos seleccionados, sem copiar recursos para o produto. Inspector: `tools/inspect_fantasynth_assets.py`; evidência: `selected-assets.json` (17) e `tunnel-materials.json` (2). Todos os payloads tiveram SHA1 validado contra o inventário prévio do índice PAK. Cada propriedade conserva o offset relativo ao início do `.uasset`; o relatório conserva também o offset absoluto do payload no PAK.

O inspector é limitado ao perfil observado de pacotes cooked com legacy version -6, tabelas de nomes com hashes e exports de 72 bytes. UE4 version=0 está serializada, portanto a identificação anterior UE4.13.2 não vem desse campo. Não descodifica shaders, buffers de vértices, bytecode Blueprint nem o corpo nativo de DataTables. A validação de limites, referências FName e coincidência do fim de tabelas foi bem sucedida nos recursos seleccionados; não houve erros de parsing de exports.

## Observações: túnel e painel emissivo

`ChezNous/Scatter/Meshes/tunnel_video_wall_lod1.uasset` tem export da classe importada StaticMesh e duas referências Materials: `M_VideoWall_Case` e `M_Theme_Map`. ExtendedBounds (propriedade offset6945): Origin=(0,-12.3518734,0), BoxExtent=(16.0000019,4.1487122,13.9093819), SphereRadius=21.3424129. Estes são bounds locais serializados; não demonstram dimensões do túnel completo, metros, escala de instância ou número de polígonos.

`ChezNous/Scatter/Materials/M_Theme_Map.uasset` é MaterialInstanceConstant; Parent aponta explicitamente para `M_Detail_Basic_Map`. Os overrides observados são:

| Parâmetro | Valor |
| --- | --- |
| Emission | 32 |
| GI Emission | 64 |
| Offset U / Offset V | 0.25 / 0.25 |
| Tiling U / Tiling V | 512 / 512 |
| Color RGBA | (0.13,0.13,0.13,1) |
| Detail | Texture2D `pattern_pixel_1` |
| Mapping | TextureRenderTarget2D `Theme_RenderTarget` |

Estas referências e valores são propriedades reais, não inferências de nomes. Não foi descodificado o shader do pai nem o conteúdo do render target; a fórmula UV, a aparência do padrão e a sequência dinâmica permanecem desconhecidas.

`M_VideoWall_Case` tem Parent=`M_Color_PBR`, Metallic=0.8, Roughness=0.2, Color=(0.1,0.1,0.1,1). Sugere uma carcaça escura metálica em torno do painel, mas o resultado visual exige shader e iluminação em execução.

`BP_Tunnel_Theme` contém templates InstancedStaticMeshComponent e HierarchicalInstancedStaticMeshComponent cuja propriedade StaticMesh referencia explicitamente `tunnel_video_wall_lod1`. ScatterSourceComponent referencia `SM_PoleDot`; StartCullDistance=4000, EndCullDistance=4000. Dois templates Collider_Box têm Rotation=(0,0,-35)/(0,0,35), HalfSize=(16,1,5), Offset=(0,-12,-10)/(0,-12,10); um terceiro HalfSize=(16,1,5), Offset=(0,-15.5,0). Não se inferiu a execução do script de construção a partir destes templates.

## Observações: pontos, emissão e névoa

`SM_PoleDot` referencia `M_PoleDot_Map`. BoxExtent=(50,50,~0), SphereRadius=70.7106781; é compatível com uma superfície plana, não demonstra uma esfera geométrica. BodySetup serializa BuildScale3D=(100,100,100). `M_PoleDot_Map` exporta parâmetros escalares Emission DefaultValue=20, GI Emission DefaultValue=40, ambos SliderMax=64. Estas são predefinições do material, não os valores efectivos de todas as instâncias.

`M_Color_EmissiveGI`: GI Emission=8 (SliderMax32), Emission=8 (SliderMax8), Color=(1,1,1,1); `bUseEmissiveForDynamicAreaLighting=true`. `M_Color_Emissive`: Emission=1 (SliderMax32), Color branco. `M_TextureColor_EmissiveBoost`: Emission=1 (SliderMax64), Color branco. Não se equiparam estas unidades de shader a intensidade física ou ganhos do renderizador de reconstrução.

`M_FogSheet_Tunnel` é MaterialInstanceConstant e Parent referencia `M_FogSheet_Color_Master`. Overrides escalares: DepthFadeDistance=1024; FadeDistance=1024; MaxOpacity=0.3; NoiseSize=24000; PanningSpeed=0.001; RefractionDepthBias=0. Overrides de cor: BaseColor=(0.7,0.7,0.7,1), Color=(0.38,0.38,0.36,1). O pai serializa BlendMode=`BLEND_Translucent`, ShadingModel=`MSM_Unlit`, TwoSided=true, bEnableSeparateTranslucency=false. O pai importa MaterialFunctions RadialGradient, ML_Raycast, ML_NormalMaskedVectors e textura T_TilingNoise03_M; existência/referência não prova toda a fórmula do shader.

## Observações: curvas e controlo de cor

`HelloSky/Curves/FogColor` exporta três FloatCurves (array_index0/1/2), cada uma com7 RichCurveKey. Parsing nativo limitado validado pelo tamanho exacto:189bytes=7×27. Tempos observados:0.23,0.26,0.32,0.5,0.68,0.74,0.77. Valores dos canais por posição, arredondados:

| Tempo | Canal0 | Canal1 | Canal2 |
| --- | --- | --- | --- |
| 0.23 | .215686 | .258824 | .301961 |
| 0.26 | .964687 | .679543 | .327778 |
| 0.32 | .982251 | .822786 | .564712 |
| 0.50 | .991102 | .871367 | .730461 |
| 0.68 | .976471 | .815686 | .564706 |
| 0.74 | .964687 | .679543 | .327778 |
| 0.77 | .215686 | .258824 | .301961 |

Todos os mode bytes e tangentes/pesos observados são0. A combinação é compatível com curva de cor fria nas extremidades e quente no centro; a correspondência canais→RGB e o significado temporal requerem confirmar o consumidor. Os tempos não foram convertidos em segundos da música nem em fases da viagem.

`DynamicColor` importa Function `SetVectorParameterValue` da classe MaterialInstanceDynamic, e o objecto Default__DynamicColor_C serializa ParameterColor branco. Isto confirma referências e predefinição de cor; não demonstra quando, nem como, a função é chamada. FogColorController e SpawnerColorController também foram inventariados; os seus bytecodes não foram interpretados.

## Estado das perguntas e limites

- Identidade/tabelas/propriedades: respondido para19 recursos seleccionados, com hash e offsets.
- Materiais de túnel/pontos/névoa: parcialmente respondido; parâmetros, pais e referências conhecidos, shader/render target desconhecidos.
- Geometria: parcialmente respondido; bounds e slots conhecidos, buffers de vértices/índices não interpretados.
- Paleta/curvas: uma curva parcialmente recuperada; ligação à coreografia não demonstrada.
- Coreografia de7min: não investigada nem reconstruída nesta análise. A referência visual fornecida pelo agente principal consiste num trailer oficial de40s e5screenshots; não demonstra cobertura da viagem completa.

Os resultados orientam escolhas independentes da reconstrução: painel com detalhe denso, carcaça escura reflectora, névoa translúcida pouco opaca e ampla variação de emissão. Formas de esferas LED, arcos, disposições radiais e paletas de secções adicionais são decisões próprias quando não corroboradas por evidência adicional.
