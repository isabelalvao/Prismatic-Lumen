# Inspecção nativa estática limitada

O original foi aberto duas vezes em `rb`, sem execução, e conserva SHA-256 `5380708aaf59992814912122afc3f71d6b18bd41062cde1a5e04fcec4e9f191b` e data de modificação. A análise não chamou REA/Ghidra nem alterou a instalação, as fontes upstream ou o jogo.

`priority-seeds.tsv` contém quatro entradas primárias de desenrolamento verificadas e executáveis: enumeração, propriedades RichCurveKey/InterpMode, SetActorHiddenInGame e SetPlaybackPosition. `seeds.tsv` contém os 25 candidatos primários. Os 28 intervalos que contêm referências, incluindo fragmentos encadeados, estão em `evidence.json` (`referencing_runtime_ranges`).

Foram encontradas 49 ocorrências ASCII/UTF-16LE, 34 referências RIP LEA/MOV verificadas por Capstone 5.0.6 e nenhum ponteiro absoluto de 64 bits nas secções não executáveis. A pesquisa de ponteiros é limitada a esse formato e não exclui outros esquemas. `.pdata` contém 136107 entradas; o resumo SHA-256 do directório é `912fec71eec1b846a6ba19d28eed49a0d2d201ceb1603b37a17221d281dee0c7`. Todos os intervalos constam de `pdata-ranges.tsv`; os resumos das secções, entradas seleccionadas, fragmentos encadeados e descodificações constam da evidência.

O intervalo `0x140402921` referencia ERichCurveInterpMode e RCIM_Linear/Constant/Cubic, e encadeia para a entrada primária `0x140402860`. O candidato `0x14049b6f0` reúne referências RichCurveKey e InterpMode por fragmentos encadeados. Estas observações sustentam metadados de reflexão; não demonstram fórmulas de interpolação, o método original C++ nem comportamento em execução. O encadeamento de desenrolamento associa fragmentos a uma entrada primária, mas não prova um percurso de controlo desde essa entrada até cada referência.

Os limites de instrução e de tamanho aplicam-se por intervalo .pdata; saltos indirectos e saídas do intervalo não são seguidos. As descodificações são TSV com endereço, bytes, mnemónica e operandos. O início/fim de um intervalo de desenrolamento não prova uma função C++ completa; funções folha podem faltar em .pdata. Os nomes `candidate_*` são inferenciais.

A interpretação de RUNTIME_FUNCTION, alinhamento UNWIND_INFO e UNW_FLAG_CHAININFO segue a [documentação oficial Microsoft](https://learn.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-170), consultada nesta investigação. Os dados específicos e as cadeias são extraídos do executável seleccionado.

Os nomes exactos dos analisadores foram lidos dos arquivos de código-fonte da instalação Ghidra 12.1.4, com resumos e linhas em `ghidra_analyzers`: **Disassemble Entry Points**, **Reference**, **Stack**, **Function ID**. Nenhuma opção foi alterada.

Verificação: oito casos rejeitados de PE/resumo, três regressões de descodificação e SHA-256/mtime antes/depois. A revisão independente identificou três problemas do guião inicial (cauda não descodificada omitida, limite por bloco e prefixo REX no filtro); foram corrigidos e os dois primeiros têm casos de regressão explícitos. O filtro agora pesquisa opcodes sobrepostos sem consumir REX e valida a instrução contendo o opcode.

Reprodução: `python C:/ORCA/Fantasynt/plan/rea-investigation/targeted-native/inspect_native.py`. Dependência confinada a `targeted-native/deps`: `python -m pip install --target C:/ORCA/Fantasynt/plan/rea-investigation/targeted-native/deps capstone==5.0.6`. O guião recusa qualquer ficheiro cujo SHA-256 difira do original seleccionado. Os ficheiros de sementes usam o formato público REA `address<TAB>function<TAB>name`; a aplicação e a descompilação ainda requerem confirmação pelo fornecedor.

| Entrada primária candidata | Referências observadas | Índice .pdata |
|---|---|---|
| 0x1403933a0 | SetActorHiddenInGame | 20319 |
| 0x1403a6930 | SetPlaybackPosition | 20457 |
| 0x1403b2ec0 | SetActorHiddenInGame | 21960 |
| 0x14041fb80 | SetActorHiddenInGame | 23084 |
| 0x1406c6020 | SetPlaybackPosition | 33686 |
| 0x140773a10 | SetPlaybackPosition | 36174 |
| 0x141536b80 | SetPlaybackPosition | 102206 |
| 0x141538e10 | SetPlaybackPosition | 102279 |
| 0x1415453f0 | SetPlaybackPosition | 102432 |
| 0x141545730 | SetPlaybackPosition | 102435 |
| 0x1417c8ac0 | SetPlaybackPosition | 115753 |
| 0x1417ca170 | SetPlaybackPosition | 115791 |
| 0x1417cd580 | SetPlaybackPosition | 115848 |
| 0x140070820 | RichCurveKey | 1475 |
| 0x1402a67c0 | InterpMode | 14698 |
| 0x1402a6ca0 | InterpMode | 14701 |
| 0x1402a7190 | InterpMode | 14706 |
| 0x1402a7680 | InterpMode | 14711 |
| 0x1402a7b70 | InterpMode | 14716 |
| 0x1402a8060 | InterpMode | 14721 |
| 0x140366580 | ERichCurveInterpMode | 19201 |
| 0x1403afb10 | RichCurveKey | 21574 |
| 0x140402860 | ERichCurveInterpMode, RCIM_Constant, RCIM_Cubic, RCIM_Linear | 22638 |
| 0x14049b6f0 | InterpMode, RichCurveKey | 24815 |
| 0x14082bf60 | InterpMode | 38739 |
