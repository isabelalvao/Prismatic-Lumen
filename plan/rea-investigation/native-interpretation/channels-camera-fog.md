# CameraRig e FogInscatteringColor: canais exactos

`channels-camera-fog.json` conserva os cinco canais completos, todos os campos das 54 chaves sem arredondamento, os bytes originais de cada chave, os seis padrões binários32, offsets relativos ao asset e absolutos no PAK, limites de secção, defaults e seis exports da hierarquia. As 54 chaves foram comparadas directamente com o payload original em modo `rb`; SHA1 `e34b3d02621b8a31185f4cb5b43bc6d6515daf9a` confirmado. Não houve REA/Ghidra nem execução nesta subinvestigação; o PAK e o executável conservam tamanho/data de modificação, e o executável conserva SHA-256 antes/depois.

## CameraRig Translation.Y

Binding `c61376fd4a6ae4408b43611d3df14659`, objecto `PersistentLevel.CameraRig`, pista export8, secção export5, offset33756, array Translation índice1. Default serializado **0.0** no tag34139. Secção: StartTime=-98.0 (tag34778), EndTime=448.0 (tag34807), bIsInfinite=true (tag34836). Playback da MovieScene termina em424.0; a chave448.0 fica além desse limite. As unidades são segundos no perfil serializado de tempos flutuantes; não houve confirmação em execução.

Todas as chaves têm modos `[0,0,0]`, pesos de chegada/saída0.0. O byte de interpolação0 é Linear pelo enum nativo `ev_5ceb47aed83db8153b4fc920b6f3172d724b328396c6c152b6ba1c2e0ace1fe7`. Eval nativo (`ev_3ddd51e30b3f5bb36a717972b370c396437667660693d90dd5e9a991db40e15c`, `0x140a90bf0`) usa o modo da chave esquerda: com modo0, as tangentes abaixo não entram na interpolação observada.

| Tempo | Valor Y | Tangente chegada | Tangente saída | Offset da chave |
|---|---|---|---|---|
| 112.0 | -6400.00048828125 | 0.0 | 0.0 | 33977 |
| 224.0 | 16000.0 | 199.9999542236328 | 199.9999542236328 | 34004 |
| 288.0 | 28800.0 | 0.0 | 0.0 | 34031 |
| 351.9999694824219 | 48000.0 | 100.0 | 100.0 | 34058 |
| 416.0 | 60800.0 | 199.99993896484375 | 199.99993896484375 | 34085 |
| 448.0 | 67200.0 | 0.0 | 0.0 | 34112 |

Os restantes canais da secção serializam Translation.X/Z=0.0; Rotation[0/1]=0.0, Rotation[2]=90.00000762939453; Scale[0/1/2]=1.0. São valores de canais do transform, não observação da posição global da câmara.

## FogInscatteringColor

Objecto `PersistentLevel.ExponentialHeightFog_1.HeightFogComponent0`, binding `887b8a49ad24be4087d6fa26481b62b8`, pista export28, secção export24, offset40388. EndTime=369.0 (tag42257), bIsInfinite=true (tag42286); StartTime não está serializado. Cada canal RGB tem16 chaves nos mesmos tempos e modos. R/G têm DefaultValue0.0 (tags40955/41559). **B não serializa DefaultValue**; o default efectivo do componente/owner não foi estabelecido. Alpha não tem chaves e serializa DefaultValue1.0 (tag42220).

Formato verificado: 27 bytes por chave, `[InterpMode,TangentMode,TangentWeightMode]` + seis float32 little-endian, pela ordem tempo,valor,tangente_chegada,peso_chegada,tangente_saída,peso_saída. TangentMode e TangentWeightMode são0 em todas estas chaves; os nomes desses dois modos não são assumidos. Todos os pesos são0.0. As tangentes RGB completas e exactas encontram-se no JSON; há valores não zero em177.0,207.0,322.0 e350.9999694824219, mas apenas o segmento iniciado na chave207.0 usa modo2/cúbico na avaliação nativa observada.

| Tempo | R | G | B | Modo da chave esquerda |
|---|---|---|---|---|
| 0.0 | 0.0 | 0.0 | 0.0 | 1 |
| 80.0 | 0.0 | 0.0 | 0.0 | 0 |
| 112.0 | 0.024000000208616257 | 0.03799999877810478 | 0.05999999865889549 | 1 |
| 144.0 | 0.0 | 0.12200000137090683 | 0.24699999392032623 | 1 |
| 175.0 | 0.0 | 0.12200000137090683 | 0.24699999392032623 | 0 |
| 177.0 | 0.3190000057220459 | 0.0010000000474974513 | 0.003000000026077032 | 0 |
| 207.0 | 0.9909999966621399 | 0.032999999821186066 | 0.014999999664723873 | 2 |
| 208.0 | 0.0 | 0.0 | 0.0 | 1 |
| 224.0 | 0.0 | 0.0 | 0.0 | 0 |
| 255.99996948242188 | 0.0 | 0.00800000037997961 | 0.019999999552965164 | 1 |
| 288.0 | 0.0 | 0.00800000037997961 | 0.019999999552965164 | 0 |
| 289.9999694824219 | 0.030712999403476715 | 0.015995999798178673 | 0.04970699921250343 | 0 |
| 320.0 | 0.030712999403476715 | 0.015995999798178673 | 0.04970699921250343 | 0 |
| 322.0 | 0.1748565137386322 | 0.008497999981045723 | 0.0263534989207983 | 0 |
| 350.9999694824219 | 0.9909999966621399 | 0.032999999821186066 | 0.014999999664723873 | 0 |
| 351.9999694824219 | 0.0 | 0.00800000037997961 | 0.019999999552965164 | 1 |

Modos:0=Linear,1=Constant,2=Cubic. O intervalo207.0→208.0 usa a chave esquerda cúbica e a tangente de chegada da chave208.0; o intervalo350.9999694824219→351.9999694824219 é linear, apesar das tangentes armazenadas na primeira chave. A avaliação observada não lê os pesos das tangentes. A aplicabilidade temporal das secções e qualquer alteração posterior dependem do consumidor MovieScene, não apenas destas curvas.

## Extrapolação: ausência serializada e default nativo

Nenhum dos cinco canais serializa PreInfinityExtrap ou PostInfinityExtrap; o JSON conserva `serialized=false` e distingue isto do default efectivo. A associação nativa RichCurve→ponteiro de operações `0x141dbeb08` é observada no dossier `ev_84869b372c481be87ebb5976c32faf4a29375c21bd7d36574e2ef06b1ed295b6`. Uma leitura PE local dos primeiros9 slots mostra slot3 em `0x141dbeb20`→entrada folha `0x140356d60`. Em `0x140356d92`, os bytes `66c742580404` escrevem uma palavra little-endian0x0404 em `[RDX+0x58]`; portanto, ambos os bytes +0x58/+0x59 começam em **4**. Em `0x140356da0`, o construtor escreve0x7f7fffff no default em+0x70 (FLT_MAX), antes de overrides de instância/owner. Prova local em `richcurve-constructor-local.json`; nenhum limite `.pdata` foi inventado para a entrada folha.

Remap `ev_fd2f7b195f795aa5224d0ac7a6421c864b120397a9f858d08c7089ef9f8873cf` em `0x140aa26a0` deixa os modos3/4 sem remapeamento cíclico. Eval distingue3 como extrapolação pela inclinação do segmento extremo; para4, selecciona o valor da chave extrema. **Inferência de herança, agora sustentada:** os tags omitidos mantêm o modo4 do construtor. Não se excluíram inicializações especiais do owner, alterações em execução ou um consumidor diferente.

Sob essa herança, CameraRig.Y vale -6400.00048828125 antes da primeira chave112.0 e67200.0 depois de448.0. Os defaults0.0 não substituem a primeira/última chave nestes ramos com chaves presentes. Fog RGB depois de351.9999694824219 conserva R=0.0, G=0.00800000037997961, B=0.019999999552965164; antes de0.0 conserva RGB=(0.0,0.0,0.0). Alpha usa o default serializado1.0 por não possuir chaves. Isto é uma derivação estática de curva; não é uma observação de frames do jogo.

## Transformação e hierarquia no mapa

O mapa ChezNous.umap tem SHA1 `1c1b5632455958ca69c0cbd15a0565315963ed0f`. CameraRig é Actor `/Script/Engine.Actor`, export78; RootComponent aponta para export1420 `CameraRig.DefaultSceneRoot` (tag294965). Esse SceneComponent, offset1091606, só serializa CreationMethod=Instance; **não** serializa RelativeLocation, RelativeRotation, RelativeScale3D ou AttachParent. Consequentemente, transform identidade e ausência de parent são hipóteses de defaults, não valores explicitamente escritos neste mapa.

A cadeia de anexos é explícita, com referências resolvidas:

- `Fantasynth_Pawn_890.DefaultSceneRoot` export1563 → `CameraRig.DefaultSceneRoot`, tag1132445;
- `Fantasynth_Pawn_890.Scene` export1564 → root do pawn, tag1132553;
- `Fantasynth_Pawn_890.Camera` export382 → Scene, tag452766.

Esses três componentes não têm overrides locais de transform serializados. O pawn (export685) referencia os mesmos componentes e RootComponent, e AutoPossessPlayer=Player0. A inferência é que mover CameraRig pode mover esta cadeia de câmara. Templates Blueprint, aplicação de pose/HMD, transform global efectivo, reparenting e consumidor da pista não foram confirmados aqui. Não se deve acrescentar uma transformação local “exacta” que o mapa não serializa.
