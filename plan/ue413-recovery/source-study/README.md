# Recuperação com as fontes locais UE 4.13.2

Estudo exclusivamente de leitura da instalação `C:/Program Files/Epic Games/UE_4.13`. `Engine/Build/Build.version` declara 4.13.2, changelist 3172292 e branch `++UE4+Release-4.13`. A coincidência com a versão observada no executável pelo agente principal favorece o uso destes serializadores, mas não prova ausência de alterações privadas no motor do jogo.

[sources.json](C:/ORCA/Fantasynt/plan/ue413-recovery/source-study/sources.json) conserva caminhos absolutos, SHA-256, tamanhos, data de modificação e intervalos de linhas de 15 ficheiros instalados. Todos os resumos e datas foram novamente verificados iguais. Não houve execução do jogo, REA, UModel, editor ou UnrealPak por este agente; não houve alterações no motor, no produto ou nas ferramentas.

## Carregamento cooked no editor desta versão

[LinkerLoad.cpp](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Runtime/CoreUObject/Private/UObject/LinkerLoad.cpp:1130>) identifica pacotes com `PKG_FilterEditorOnly`. Nas linhas 1138–1140 lê `[Core.System] AllowCookedDataInEditorBuilds` e **usa true quando a opção não está configurada**; só recusa explicitamente o carregamento quando a configuração efectiva é false. Na linha 1285 marca o pacote `bIsCookedForEditor`.

A pesquisa literal de `AllowUnversionedContentInEditor` em `Engine/Source` e `Engine/Config` não encontrou ocorrências. Essa opção não é sustentada pelas fontes instaladas 4.13; não é necessária para explicar o seu tratamento de versões zero. [PackageFileSummary.cpp](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Runtime/CoreUObject/Private/UObject/PackageFileSummary.cpp:111>) trata UE4Version=0 e LicenseeVersion=0 como pacote sem versão: marca `bUnversioned` e adopta as versões actuais do motor e o conjunto de custom versions registadas (linhas 111–118). Isto explica por que deve ser usado o motor correspondente ao pacote. Não significa que um motor arbitrariamente recente possa interpretar o mesmo formato.

**Carregar e abrir um editor de recurso são operações diferentes.** [AssetEditorManager.cpp](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Editor/UnrealEd/Private/Toolkits/AssetEditorManager.cpp:280>) devolve false ao tentar abrir um editor dedicado para um pacote `bIsCookedForEditor` (linhas 280–282). Por isso, a tentativa de abrir uma malha cooked por duplo clique pode ser bloqueada mesmo quando o objecto foi carregado correctamente. Uma rota com o motor deve carregar o objecto e chamar directamente o exportador/API num projecto isolado; não exige alterar o motor para contornar este bloqueio de interface. A disponibilidade real dessa rota nos recursos escolhidos ainda precisa de teste.

## SoundWave: recuperar os dados comprimidos

[SoundWave.cpp](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Runtime/Engine/Private/SoundWave.cpp:177>) serializa primeiro a parte UObject, depois `bCooked`, possíveis metadados legados de compressão e o formato apropriado. Quando cooked e não dependente da rota de streaming, serializa `CompressedFormatData` (linhas 209–231). `RawData` só é serializado no ramo **não cooked** (235–238). Logo, um WAV de origem pode não existir no asset distribuído, apesar de os dados de reprodução estarem presentes.

[BulkData.cpp](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Runtime/CoreUObject/Private/Serialization/BulkData.cpp:1452>) mostra o formato do contentor: quantidade de formatos, FName de cada formato e o seu `FByteBulkData` (1456–1464). Ao cozinhar esta rota, força os payloads inline (1489–1492). Isto fornece uma rota para um leitor estático delimitado por export e por bulk header, sem assumir que qualquer ocorrência `OggS` isolada corresponde a um áudio completo.

O exportador nativo [USoundExporterOGG](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Editor/UnrealEd/Private/EditorExporters.cpp:133>) pede `GetCompressedData("OGG")` e copia directamente o bulk para a saída (133–142). [GetCompressedData](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Runtime/Engine/Private/SoundWave.cpp:323>) reutiliza o formato existente; se faltar, pode tentar a DDC em builds que a suportam (329–345). Assim, a recuperação deve seleccionar e validar os bytes cooked já presentes, em vez de depender de recompressão a partir de RawData ausente. O exportador OGG não valida conteúdo nem exige tamanho maior que zero antes de devolver true: sucesso da chamada, existência do ficheiro e tamanho não bastam para confirmar áudio válido.

O [exportador WAV](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Editor/UnrealEd/Private/EditorExporters.cpp:97>) apenas copia `Sound->RawData` (97–103); não descodifica automaticamente o OGG cooked. Para obter PCM/WAV, a rota prática é recuperar primeiro o formato comprimido identificado e depois descodificá-lo numa saída própria. Validar cabeçalhos, descodificação integral, canais, frequência e duração contra os metadados já recuperados.

Para áudio com streaming, [AudioDerivedData.cpp](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Runtime/Engine/Private/AudioDerivedData.cpp:666>) serializa `NumChunks`, `AudioFormat` e cada chunk (666–682); `SerializeCookedPlatformData` carrega `RunningPlatformData` (990–1032). Um exportador que só examine `CompressedFormatData` pode não cobrir essa rota. `SupportsAudioStreaming` no Windows devolve `!IsServerOnly()` em [WindowsPlatformProperties.h](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Runtime/Core/Public/Windows/WindowsPlatformProperties.h:65>). Determinar `IsStreaming` do recurso antes de escolher o leitor.

## StaticMesh: os buffers cooked conservam a geometria de renderização

[UStaticMesh::Serialize](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Runtime/Engine/Private/StaticMesh.cpp:2049>) lê strip flags e `bCooked` (2057–2060). Os source models/raw mesh editor-only só são serializados quando não foram removidos (2139–2147). No ramo cooked, cria `FStaticMeshRenderData` e lê os dados serializados inline (2157–2162). O cooking pode, portanto, remover RawMesh sem eliminar posições, índices e atributos usados para desenhar a malha.

Em [PostLoad](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Runtime/Engine/Private/StaticMesh.cpp:2219>), as correcções editor-only e `CacheDerivedData()` estão dentro da condição que exclui `PKG_FilterEditorOnly` (2219–2282). A rota cooked evita reconstruir os dados a partir de SourceModels que já podem ter sido removidos.

A sequência dos dados nativos é explícita nas fontes:

| Etapa | Referência local | Conteúdo relevante |
|---|---|---|
| FStaticMeshRenderData | StaticMesh.cpp:675–746 | LODResources; distance fields opcionais; Bounds; flags; factores de streaming; ScreenSize |
| FStaticMeshLODResources | StaticMesh.cpp:284–343 | Strip flags; Sections; MaxDeviation; positions, atributos, cores e vários buffers de índices |
| Secção | StaticMesh.cpp:272–280; StaticMeshResources.h:292–320 | MaterialIndex, FirstIndex, NumTriangles, Min/MaxVertexIndex, colisão e sombra |
| Posições | PositionVertexBuffer.cpp:121–138; StaticMeshResources.h:350–359 | Stride, NumVertices e dados de FVector |
| Atributos | StaticMesh.cpp:199–225 | NumTexCoords, Stride, NumVertices, full-precision UV e high-precision tangent flags; vertex data |
| Índices | RawIndexBuffer.cpp:241–261 | Flag 16/32 bits e IndexStorage.BulkSerialize, conforme a versão |

Esta ordem é uma orientação para um leitor específico de 4.13, não uma declaração de offsets fixos universais. As strip flags, formatos dos atributos, contadores e comprimentos de arrays/bulk têm de controlar a leitura. Os buffers de índices adicionais são distintos da lista principal de triângulos. Não reduzir o recurso a bounds nem tratar stride inferido como validado sem consumo exacto.

## Exportar a malha com o motor, se necessário

[UStaticMeshExporterFBX::ExportBinary](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Editor/UnrealEd/Private/EditorExporters.cpp:1779>) encaminha o objecto para FFbxExporter. [FbxMainExport.cpp](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Editor/UnrealEd/Private/Fbx/FbxMainExport.cpp:2471>) usa `GetLODForExport` (2483), lê `PositionVertexBuffer` (2527), a lista principal de índices e as secções (2543–2550), normais via tangente Z (2570) e UVs (2630). [GetLODForExport](<C:/Program Files/Epic Games/UE_4.13/Engine/Source/Runtime/Engine/Private/StaticMesh.cpp:2934>) retorna directamente `RenderData->LODResources[LODIndex]` (2934–2939). Esta rota exporta geometria de renderização e não requer RawMesh de importação.

O exportador FBX transforma `(X,Y,Z)` em `(X,-Y,Z)` na linha 2528 e pode soldar vértices antes de escrever. Consequentemente, validar a convenção de eixos, bounds, triângulos e atributos da saída; uma contagem de vértices diferente não prova, por si só, perda de geometria. O exportador pode devolver sem geometria quando faltam vertices/secções (2486–2495). A validade do ficheiro final tem de ser verificada independentemente.

## Rotas práticas por prioridade

1. Usar os resultados dos três testes UModel conduzidos pelo agente principal, mantendo versão declarada, comandos, logs, hashes e validação de áudio/malha. Este estudo não executou nem validou esses testes.
2. Se necessário, carregar cópias dos pacotes num projecto isolado do **UE 4.13.2**, com os mesmos caminhos virtuais de pacote e dependências disponíveis; chamar o exportador OGG ou FBX directamente, sem exigir a abertura de um asset editor cooked. Confirmar `AllowCookedDataInEditorBuilds` efectivo caso o carregamento seja recusado. Não assumir que flags modernas existem em 4.13.
3. Se o carregamento pelo motor continuar bloqueado por dependências/formato, usar as rotas de serialização acima para um leitor offline limitado de cooked render data ou bulk áudio. Preservar hashes, limites, formatos e bytes não interpretados; confirmar consumo e valores válidos antes de integrar qualquer recuperação no produto.

## UnrealPak -List e código de saída 1

O log guardado pelo agente principal termina com a contagem de **1696 ficheiros**, total **257257886 bytes**, e lista entradas do pacote até ao fim. Isto demonstra que uma listagem foi produzida; não estabelece o significado do código de processo 1.

A instalação contém `Engine/Binaries/Win64/UnrealPak.exe`, mas a pesquisa em `Engine/Source` não encontrou o ficheiro do programa UnrealPak ou o seu main; `Source/Programs` contém apenas UnrealHeaderTool. **A causa exacta do exit code 1 permanece desconhecida nesta nota.** Não se atribuiu o resultado a uma conversão bool→exit code nem se reinterpretou 1 como sucesso sem o código ou uma investigação independente do executável. A listagem pode ser usada como evidência de inventário em conjunto com os hashes/validação já existentes, mantendo separado o estado do processo.
