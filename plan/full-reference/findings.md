# Fantasynth: sequência completa e distribuição estática

## Resultado e fronteira de evidência

O pacote contém uma LevelSequence com intervalo de reprodução até **424 s (7 min 4 s)**; o áudio MASTER serializa **424.18212890625 s**, estéreo, 44100 Hz. `ChezNousProfile.EndTime=424` e referencia explicitamente o actor da sequência. Foram recuperadas as 42 pistas ligadas a objectos, as 9 pistas principais, 70 configurações Scatter, 82 ComposerSequencer, 142 linhas de DataTable e bounds de 66 StaticMesh. Isto recupera a configuração temporal completa serializada desta sequência, mas não equivale a observar a viagem completa em execução.

Fonte: PAK autorizado do Steam, inventário `../fantasynth-pak-inventory.json`. SHA1 de todos os payloads seleccionados validado contra o índice. Os relatórios detalhados conservam offsets relativos ao pacote e o offset absoluto do payload no PAK. Nenhum recurso original foi colocado no produto; só foram guardados metadados e propriedades JSON. Não se executou o original, Ghidra ou IDA.

## Relatórios reutilizáveis

| Ficheiro | Conteúdo |
| --- | --- |
| `timeline.json` | 42 pistas com GUID resolvido, object_path, secções, defaults, todas as chaves, tangentes, pesos e modos; 9 master tracks |
| `map-layouts.json` | 518 actores/layouts/componentes seleccionados e 952 componentes com transformações/anexos |
| `scatter-summary.json` | 70 volumes, fontes, pesos/culling, escalas/rotações, alternativas de layouts e transformações locais |
| `composer-sequencers.json` | 82 sequenciadores: directores, presets DataTable A/B, índices, easing, tempo e passos |
| `composition-table.json` | 142 presets: posição, rotação, escala, modos, valores base/máximos, sementes e easing |
| `action-sequencers.json` | 14 arrays de BeatAction ligados a directores e actores |
| `mesh-bounds.json` | 66 meshes, bounds locais, slots de material, hash e bytes nativos não interpretados |
| `enriched-map-sequence.json` | Evidência detalhada do mapa, sequência, DataTable e áudio |
| `controllers-enums.json` | Configuração estática dos controladores, defaults de layouts, FText e enum nativo |
| `geometry-controller-assets.json` | Bounds e templates de Blueprints Lower, layouts serializados e meshes do campo |
| `mesh-assets.json` | Evidência detalhada de 67 recursos geométricos seleccionados |
| `validation.json` | Verificações da ferramenta nova |

`map-sequence.json` conserva a primeira leitura com o inspector existente. A ferramenta adicional é `../../tools/inspect_fantasynth_sequence.py`; reutiliza Reader, properties e inspect do inspector existente sem o modificar. Acrescenta parsing limitado de structs etiquetadas, arrays, referências GUID, FText de história Base, enum nativo do perfil observado e corpo DataTable. Os corpos DataTable e enum têm consumo exacto até ao limite do export. Estruturas não suportadas ficam explícitas.

## Timeline: observações reais

`ChezNousSequence.uasset`: SHA1 `e34b3d02621b8a31185f4cb5b43bc6d6515daf9a`; playback UpperBound=424 no offset 31654. Áudio MASTER: Duration no offset 791. A sequência usa MovieScene com tempos flutuantes deste perfil, em segundos; não foram confundidas as curvas normalizadas de HelloSky com segundos da música.

DirectorSection.Time tem chaves `(tempo,valor)` em `(0,0),(16,1),(48,2),(80,3),(112,4),(144,5),(176,6),(208,7),(224,8),(256,9),(288,10),(320,11),(352,12),(384,13),(416,14)`; os valores exactos de float, modos e tangentes estão em `timeline.json`. Chaves das outras 28 pistas de directores também foram preservadas. Estes valores podem orientar divisões da reconstrução; não foi demonstrado por bytecode o mecanismo que transforma o valor em chamada de acção.

Eventos NameCurve: `Set_Mode_2D` em 0 s e `Set_Mode_3D` em 208 s. Esta é uma associação temporal serializada; os efeitos das funções não foram traçados. Duas outras pistas de eventos contêm marcadores sobretudo `None`, com um marcador literal `10` aos ~288 s; não se atribuiu significado de mudança de cena a estes nomes.

CameraRig, ligado pelo GUID `c61376fd4a6ae4408b43611d3df14659` ao objecto `PersistentLevel.CameraRig`, tem Translation.Y:

| Tempo (s) | Y local |
| --- | --- |
| 112 | -6400.00048828125 |
| 224 | 16000 |
| 288 | 28800 |
| 351.9999694824219 | 48000 |
| 416 | 60800 |
| 448 | 67200 |

Translation.X/Z default=0, Rotation.Z default≈90°, Scale XYZ=1. A secção é infinita e possui StartTime=-98, EndTime=448; a última chave está além do playback de 424. Extrapolação antes da primeira chave, hierarquia da câmara/HMD e movimento efectivo exigem o consumidor. **A curva de ParticlesRig é distinta**: 108→-6400, 220→16000, 284→28800, 348→48000, 412→60800; não deve ser usada como curva da câmara.

Cor MappingColorController: t142 RGB=(.25,.45,1); t144=(1,.45,.25); t176 mantém este valor; t178=(.25,.45,1), alpha=1. InsideColorController: t176 branco; t178=(1,.437,.25); t208 e224 preto, alpha=1. As pistas HelloSky.Skybox Color e HeightFogComponent0.FogInscatteringColor também estão completas.

Névoa: FogDensity `(0,4),(80,.01),(224,.01),(256,.01)`; FogHeightFalloff `(0,2),(112,8),(176,8),(208,2),(224,2),(256,2)`; StartDistance `(0,1024),(80,128)`. São valores de propriedades UE4, não intensidades do renderizador da reconstrução.

## Distribuição: observações reais

Cada Scatter referencia um Spawner, que referencia classes Blueprint Sources; SourceParams contém pesos e distâncias de culling. SourceScales/SourceRotations foram integralmente descodificados. Vários layouts coexistem no mesmo actor: a sua presença não prova que todos sejam seleccionados ou executados. A ausência de um override Enabled não prova desactivação.

| Actor / configuração | Propriedades serializadas |
| --- | --- |
| 010ScatterTunnelDotGrid | Extent=(1600,25600,1600); local position=(10,38410,200) anexada a ScatterTunnelContainer; escala fonte=.3; Layout_Tube ArcPoints48, ArcDistance128, RadiusMin/Max1024 |
| 011ScatterTunnelDots | Extent=(4000,25600,4000); Layout_Tube ArcPoints48, ArcDistance128, Radius4000, ArcJitter2, AngleJitter2 |
| 012ScatterTunnelNeon | Layout_Tube ArcPoints48, ArcDistance1024, Radius4000, AngleOffset.5 |
| 013ScatterTunnelLower | Sources BP_Lower_1..6, pesos1, culling16000; Layout_Tube ArcPoints48, ArcDistance128, Radius3200 |
| 00ScatterTunnelVideoWall | Extent=(1600,48000,1600); local position=(0,60800,200); SourceScale80..160; Grid CellSize=(2000,2000,2000); Tube ArcPoints5, ArcDistance1024, Radius1024..1536 |
| 05ScatterTunnelRock | Tube ArcPoints128, ArcDistance96, Radius4000; alternativas Cluster e Grid CellSize320 |
| 06ScatterTunnelRockLight | Tube ArcPoints96, ArcDistance128, Radius4000; alternativas Cluster e Grid CellSize256 |
| 09ScatterTunnelRings | Tube ArcPoints16, ArcDistance8192, Radius480 |
| 0ScatterPyramidGroundFront | Extent=(8000,6000,6400); local position=(0,4800,0) anexada a ForestScatterContainer; Grid CellSize3000; SourceScale1.5..2 |
| 27ScatterVoidLower | Sources BP_Lower_1..6, culling8000, Extent=(6000,6000,6400), local position=(0,18800,-.166), SourceScale faixas .25.. .5, .5..1, 1..2, 3..4 |

Defaults de Layout_Tube do Blueprint: ArcDistance1000, RadiusMin/Max100, AngleEnd360. A divisão 360/48=7.5° é uma inferência aritmética útil para uma reconstrução independente; a distribuição efectiva requer a função Layout. As propriedades Element Orientation e Element Distance guardam resultados/componentes auxiliares, mas não foram tratadas como inventário completo de instâncias.

## Geometria: dados e limites

`SM_Lower_1..6` possuem os mesmos bounds locais: Origin≈(0,0,12.5), BoxExtent≈(43.30127,43.30125,12.5), SphereRadius45.069389. Isto permite dimensionar formas largas e baixas na reconstrução; não prova cones segmentados, número de lados ou contorno exacto. Os templates BP_Lower contêm StaticMeshComponent e HierarchicalInstancedStaticMeshComponent, fontes de mesh e Collider_Sphere Radius45; a geometria de cada submesh pode diferir.

`SM_Tree`: Origin≈(0,0,800), BoxExtent≈(25,25,800), raio800.3905. `camo32_tube/alpha32` e `green32`: Origin=(-25,-7.457743,0), BoxExtent=(15.915508,7.957743,50). São bounds e referências reais de famílias do campo; não justificam trocar todas estas estruturas por árvores rectas. A curvatura e o número de faixas exigem descodificar vértices/índices ou inspeccionar o original visualmente. `forest_content_green_fixed` foi incluído no relatório de geometria, mas a topologia continua opaca.

## Cadeia de composição

O corpo nativo de HelloCompositionTable foi consumido integralmente: 142 linhas com nove canais Position/Rotation/Scale e seis parâmetros por canal. ComposerActions dos 82 ComposerSequencer apontam RowName e DataTable reais, pelo que é possível ligar cada configuração a um preset sem inferir nomes. Exemplo: `2SequencerDiamonds` tem DirectorSection, StartIndex5 e presets Diamond_Void_0/1. O significado visual de cada preset e o momento de aplicação dependem das funções de Blueprint.

BeatActionType: enum0/NewEnumerator0 tem FText `Start`; enum1/NewEnumerator1 tem FText `Stop`. BeatDirectorMode: enum0=`Alpha`, enum1=`Index`. O inspector validou os três entries nativos de cada enum incluindo MAX, e os dois textos de exibição. Esta identificação de enum é estática; não demonstra que uma acção armazenada seja disparada.

## Perguntas e lacunas precisas

| Pergunta | Estado |
| --- | --- |
| Duração, chaves temporais, bindings, curvas de cores/névoa/câmara | Respondida para toda a LevelSequence serializada |
| Fontes Scatter, volumes, layouts, parâmetros, pesos, escalas | Respondida como configuração estática; instâncias finais não recuperadas |
| Presets DataTable e suas referências nos sequenciadores | Respondida, consumo exacto e zero erros adicionais |
| Contorno dos elementos curvos, cones segmentados e lajes | Parcial: bounds, fontes e slots; buffers de vértices/índices não descodificados |
| Cena visual em cada um dos 424 segundos | Não demonstrada: bytecode dos consumidores, shaders/render targets e execução original não observados |
| Instâncias carregadas por Layout_Serialized_Forest/Intro/Tunnel | Não recuperadas: templates/instâncias observados não expõem arrays de posições nas propriedades etiquetadas seleccionadas; consumidor/layout base não traçado |

A reconstrução pode aproveitar a duração, as chaves da câmara, as transições de cor/névoa e as divisões temporais observadas. Uma promessa de reprodução integral da coreografia visual requer resolver estas lacunas e validar visualmente toda a experiência original.
