# Descodificação local de Blueprint

Estes artefactos provêm de um parser local limitado e dos bytes dos recursos
serializados no PAK original. **Não são REA Evidence, análise nativa ou observação
de execução.** O verificador apenas lê o PAK e os documentos de entrada; escreve
os resultados neste directório.

## Reprodução

Na raiz `C:/ORCA/Fantasynt`, executar:

```powershell
python -B plan/rea-investigation/decoded-blueprints/verify_decode.py
```

É necessário Python 3.10 ou superior, o parser existente
`tools/inspect_fantasynth_assets.py`, o inventário
`plan/fantasynth-pak-inventory.json`, as regiões
`plan/rea-investigation/blueprint-script-regions.json` e o PAK no caminho indicado
pelo inventário. Não existem dependências Python externas.

O resultado normal é `status=passed`, dois recursos e oito funções. Qualquer
hash divergente, token desconhecido, leitura fora dos limites, tamanho
serializado/em memória divergente ou alvo fora de um limite de instrução provoca
uma excepção e código de saída diferente de zero. Todos os recursos são
verificados antes da escrita dos resultados.

O perfil exige um único `EX_EndOfScript` como último token ao nível superior,
terminadores de argumentos/structs apenas nos seus consumidores, referências
com classes compatíveis com cada papel e alvos de selecção exactamente no fim
do resultado de cada caso ou do ramo por omissão. Referências null são permitidas
apenas no campo opcional `rvalue_property` de `EX_Context`; FNames numerados são
rejeitados, pois todas as chamadas seleccionadas usam `number=0`.
O relatório inclui 21 verificações de rejeição de entradas malformadas, incluindo
bytes de operando com valor0x53 que não constituem um token EndOfScript.

## Artefactos

- `verify_decode.py`: descodificador e verificador reproduzível.
- `BeatDirector.instructions.json`: cinco funções, instruções completas e
  expressões aninhadas, referências resolvidas, offsets e verificações.
- `ActorHiddenSequencer.instructions.json`: três funções com o mesmo formato.
- `verification-report.json`: hashes dos recursos, scripts, entradas e
  descodificador; tamanhos; desvios; spans de contexto; pontos de entrada.

Os offsets `storage_offset` são relativos ao script serializado; somar
`asset_script_offset` ou `pak_script_offset` para localizar os bytes no recurso
ou no PAK. Os offsets `memory_offset` pertencem ao perfil do script carregado,
com ponteiros de oito bytes e FNames de doze bytes. Não são endereços nativos.
A lista `instructions` inclui tokens aninhados e terminadores; os nós exteriores
contêm também os seus filhos. `top_level_storage_offsets` distingue as
instruções exteriores das expressões usadas como operandos.

## Interpretações anteriormente verificadas

Para valores finitos usuais, `BeatDirector` calcula:

```text
T = max(Time + Time Offset, 0)
Index = floor(T)
Alpha = T - float(floor(T))
Changed = (Alpha != LastAlpha) || (Index != LastIndex)
```

`ReceiveTick` guarda `DeltaSeconds` e entra no grafo em memória `1502`.
`IsPlaying` e `Enabled` condicionam a actualização. O despacho
`UpdateEventDispatcher(DeltaSeconds)` ocorre antes do cálculo de `Index/Alpha`.
O modo `Alpha` (enum 0) despacha Beat quando
`Alpha > 0 && (LastAlpha <= 0 || Alpha < LastAlpha)`; o modo `Index` (enum 1)
despacha quando `Index != LastIndex`. Há no máximo um despacho por percurso de
tick. Não existe integração `Time += DeltaSeconds` ou recuperação de índices
saltados neste script. Os modos 0/1 copiam `LastAlpha/LastIndex` depois dos
eventuais subscritores Beat; outros valores de modo saem antes dessas cópias.

`Start` define `IsPlaying=true`, calcula `Index/Alpha` e despacha Start.
`Stop` define `IsPlaying=false` e despacha Stop. Ambas as funções despacham sem
verificar o estado anterior. `Reset` define `LastAlpha=0`, `LastIndex=0`,
`Changed=false` e `IsPlaying=false`; não redefine `Time/Index/Alpha` nem despacha.

As duas chamadas `SetActorHiddenInGame` em `ActorHiddenSequencer`, nos offsets
serializados 181 e 688, usam uma selecção do campo de acção: byte 0 selecciona
o local booleano previamente definido como `false`; byte 1 selecciona o local
definido como `true`. O enum `BeatActionType` identifica 0 como Start e 1 como
Stop. Assim, nesta classe, Start mostra e Stop oculta. O ramo por omissão usa
`K2Node_Select_Default`; não foi interpretado para valores inválidos.

Os textos dos enums foram cruzados com
`plan/full-reference/controllers-enums.json` e os recursos originais na análise
anterior. O verificador deste directório verifica os dois recursos de Blueprint
e as oito funções indicadas, não volta a descodificar os recursos dos enums.

## Limites e referências

A correspondência dos hashes, tamanhos e alvos é uma verificação estrutural.
Não prova subscritores activos, ordem externa de eventos, frequência de ticks,
instantes reais, visibilidade efectiva ou efeitos dos callbacks. O parser cobre
os tokens encontrados nestes scripts; não se destina a recursos UE arbitrários.

O perfil foi cruzado com o código primário de
[UAssetAPI](https://github.com/atenfyr/UAssetAPI/tree/master/UAssetAPI/Kismet/Bytecode/Expressions)
e com a lista de tokens
[EExprToken da Epic](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/CoreUObject/EExprToken).
Nenhuma biblioteca desses projectos foi incorporada ou instalada; o script usa
a biblioteca padrão de Python e o parser local existente.
