# Interpretação dos dossiers nativos mínimos

Alvo: `Fantasynth-Win64-Shipping.exe`, SHA-256 `5380708aaf59992814912122afc3f71d6b18bd41062cde1a5e04fcec4e9f191b`. Os quatro dossiers têm perfil `7168c70d6fdd7c9a8a4f1f27eff4d611637d12bbcb5fec2a765beabd1458a7c2`; foram desactivados Disassemble Entry Points, Reference, Scalar Operand References, WindowsResourceReference, Stack, Function ID, Decompiler Switch Analysis e Non-Returning Functions - Discovered. Consequentemente, ausência de referências ou nomes não demonstra ausência de comportamento. Os nomes `candidate_*` foram fornecidos pelo analista; `FUN_*`/`PTR_*` são nomes do Ghidra. Pseudocódigo não é código-fonte original, e as assinaturas inferidas não estabelecem os tipos C++ originais.

## Enumeração: observado e derivado

Evidence `ev_5ceb47aed83db8153b4fc920b6f3172d724b328396c6c152b6ba1c2e0ace1fe7`, procedimento `0x140402860`: a construção da enumeração associa nomes a bytes explícitos e regista `/Script/CoreUObject`.

| Nome | Valor derivado | Instrução que coloca o valor |
|---|---:|---|
| RCIM_Linear | 0 | `0x1404029e9`: MOV byte ptr [RSP+0x58], R14B; R14D foi posto a zero em `0x140402890` |
| RCIM_Constant | 1 | `0x140402a43`: MOV byte ptr [RSP+0x58], 1 |
| RCIM_Cubic | 2 | `0x140402a9d`: MOV byte ptr [RSP+0x58], 2 |
| RCIM_None | 3 | `0x140402af7`: MOV byte ptr [RSP+0x58], 3 |
| RCIM_MAX | 4 | `0x140402b51`: MOV byte ptr [RSP+0x58], 4 |

Isto confirma valores nativos da enumeração. Não demonstra a avaliação de curvas lineares/cúbicas nem as fórmulas usadas pelo jogo.

## RichCurveKey: metadados observados e disposição derivada

Evidence `ev_9bfa06c2e511b6d3db98eda40539808d004e9f9fc204a35be3f399cb6555e966`, procedimento `0x14049b6f0`: `0x14049b76e` coloca `0x1c` no argumento R8D de `FUN_140252aa0`; o objecto de operações guarda `0x1c` em `[RAX+8]` (`0x14049b825`) e `4` em `[RAX+0xc]` (`0x14049b82c`). A interpretação é tamanho declarado de 28 bytes e alinhamento declarado de 4 bytes; os corpos destes auxiliares ainda não foram interpretados aqui.

Os nomes são construídos antes de cada chamada a `FUN_140268120`. O quarto argumento contém os deslocamentos abaixo; o tamanho da propriedade é escrito no seu descritor em `+0x34`.

| Campo | Deslocamento na chave | Tamanho declarado | Prova de argumento e chamada |
|---|---:|---:|---|
| InterpMode | 0 | 1 | `0x14049be22` XOR R9D,R9D; chamada `0x14049be33`; tamanho `0x14049be38` |
| TangentMode | 1 | 1 | `0x14049bd79`; chamada `0x14049bd8d`; tamanho `0x14049bd92` |
| TangentWeightMode | 2 | 1 | `0x14049bcd1`; chamada `0x14049bce5`; tamanho `0x14049bcea` |
| Time | 4 | 4 | `0x14049bc1e`; chamada `0x14049bc32`; tamanho `0x14049bc37` |
| Value | 8 | 4 | `0x14049bb83`; chamada `0x14049bb97`; tamanho `0x14049bb9c` |
| ArriveTangent | 12 | 4 | `0x14049bade`; chamada `0x14049baf2`; tamanho `0x14049baf7` |
| ArriveTangentWeight | 16 | 4 | `0x14049ba43`; chamada `0x14049ba57`; tamanho `0x14049ba5c` |
| LeaveTangent | 20 | 4 | `0x14049b9a8`; chamada `0x14049b9bc`; tamanho `0x14049b9c1` |
| LeaveTangentWeight | 24 | 4 | `0x14049b90d`; chamada `0x14049b921`; tamanho `0x14049b926` |

O registo de InterpMode chama a entrada da enumeração `0x140402860`. O byte em deslocamento 3 fica sem propriedade reflectida; a interpretação como enchimento é corroborada pelo construtor candidato `0x140356db0`, obtido do ponteiro de operações, que escreve nos deslocamentos 0, 2, 4, 12 e 20 e também omite o byte 3.

## SetActorHiddenInGame: observado

Evidence `ev_23c380fae87c2730622c8716833582676993560a2c9aa02ccc0779cc2071d7c0`, procedimento `0x1403933a0`: o pseudocódigo contém `thunk_FUN_1402b3170(uVar1,"SetActorHiddenInGame",FUN_1404fadf0)`. Em assembly, `0x140393efd` carrega `0x1404fadf0` em R8, `0x140393f04` carrega o nome em RDX a partir de `0x141d66000`, e `0x140393f0b` chama `0x1402cd210`. O vínculo nativo nome→ponteiro está demonstrado; o comportamento do handler ainda não foi analisado neste dossier. Próxima entrada sugerida: `0x1404fadf0`.

## Reprodução: âmbito correcto

Evidence `ev_ba372464739055602d529a91d67bacd44388a442fde45fa3740b8edda43e0254`, procedimento `0x1403a6930`: observado `/Script/Engine`, `GetTimelineLength`, `OnRep_Timeline`, `SetTimelineLength` e `SetTimelineLengthMode`, com SetPlaybackPosition ligado a `0x140507740` (R8 em `0x1403a6afd`; nome em `0x1403a6b04`; chamada em `0x1403a6b0b`). A identificação do subsistema Timeline é uma inferência forte pelos nomes, mas o dossier não demonstra a implementação de LevelSequence.

A leitura local limitada das entradas existentes corrigiu outro falso candidato: `0x141536b80` referencia `/Script/Paper2D` em `0x141536b84` e GetFlipbook/SetFlipbook. Não deve ser seleccionado como registo LevelSequence.

A entrada apropriada é **`0x1417c8ac0`**: referências locais verificadas desde a sua entrada `.pdata` mostram `/Script/LevelSequence` em `0x1422dd758` (primeira instrução de referência `0x1417c8ac4`), CreateLevelSequencePlayer, GetPlaybackStart/End, PlayLooping e SetPlaybackPosition. Em `0x1417c8c6b`, R8 recebe **`0x1417cf460`**; em `0x1417c8c72`, RDX recebe o nome SetPlaybackPosition em `0x1422dc2c8`; a chamada de registo é `0x1417c8c79`. Este vínculo é derivado do mesmo padrão de registo corroborado pelos dossiers. O corpo local de `0x1417cf460` extrai um parâmetro de 4 bytes com MOVSS e chama **`0x1417d3f20`** em `0x1417cf4cb`. A assinatura e o efeito dessa última função são desconhecidos.

Próximas consultas REA sugeridas: `0x1417c8ac0` para confirmar o registo LevelSequence, `0x1417cf460` para o wrapper, e depois `0x1417d3f20` para a operação. `0x1417ca170` e `0x1417cd580` são candidatos de metadados no mesmo subsistema, mas não foram interpretados aqui.

## Ponteiros de operações e candidato Serialize

O dossier RichCurveKey observa que `0x14049b833` carrega `PTR_FUN_141dbebf8` e `0x14049b83a` o instala no objecto de operações. A leitura PE local encontrou 27 ponteiros para `.text`, seguidos de dados não executáveis no índice 27. Isto delimita a janela observada, mas não prova uma descrição completa da tabela virtual.

O índice 7, slot **`0x141dbec30`**, contém **`0x14038f080`**. Nesse endereço, os bytes descodificam `MOV RCX,R8` (`0x14038f080`) e `JMP 0x140aa3fa0` (`0x14038f083`). É um thunk observado; a identificação da função virtual como Serialize continua inferencial.

O destino **`0x140aa3fa0`** é uma entrada primária `.pdata` (índice 49860), com fragmentos encadeados `0x140aa3fd2` e `0x140aa40f8`. Os seis intervalos locais estudados têm descodificação limitada sem falhas registadas; não foram atravessados saltos indirectos nem executado o alvo.

Este candidato testa o valor de 32 bits em `[RDX+8]` contra `0x16e` (366) em `0x140aa3faa` e devolve zero no ramo inferior. No outro ramo, usa o ponteiro guardado em RDI para pedir transferências através do método virtual em `+0x40` do objecto guardado em RBX:

| Deslocamento da chave | Quantidade passada em R8D | Chamada indirecta |
|---|---:|---|
| 0 | 1 | `0x140aa3fda` |
| 1 | 1 | `0x140aa3fed` |
| 2 | 1 | `0x140aa4000` |
| 4 | 4 | `0x140aa4013` |
| 8 | 4 | `0x140aa403e` |
| 12 | 4 | `0x140aa4069` |
| 16 | 4 | `0x140aa4094` |
| 20 | 4 | `0x140aa40bf` |
| 24 | 4 | `0x140aa40ea` |

O mapeamento coincide com as propriedades da chave. **Derivado:** são pedidos 27 bytes de conteúdo (3×1 + 6×4), omitindo o enchimento em deslocamento 3, apesar do tamanho declarado em memória ser 28. Não se deve deduzir um passo de 28 bytes no arquivo a partir do tamanho em memória. A interpretação do objecto como arquivo e de `[arquivo+8]` como versão é forte, mas ainda inferencial; não se estabeleceu aqui o formato de todos os arquivos nem a selecção deste método pelos assets específicos do jogo. Após cada transferência de 4 bytes, o código pode chamar `0x140245c90` se o byte `[RBX+0x3b]` for diferente de zero; a semântica desse indicador e do auxiliar não foi confirmada.

Os demais ponteiros não fornecem uma alternativa tão forte: o índice 18 chama `0x140a877e0` e coloca o resultado através de um argumento, mas a sua identidade virtual é desconhecida; o índice 24 (`0x140f6ead0`) apenas devolve um endereço. Próxima consulta prioritária sugerida: **`analyze_function(0x140aa3fa0)`**; o wrapper `0x14038f080` serve de prova adicional de vínculo, mas não tem entrada `.pdata` própria.

## Proveniência e limites

`supporting-evidence.json` conserva os IDs e os resumos SHA-256 dos quatro dossiers, os seus pseudocódigos/assemblies, opções desactivadas e limitações; conserva ainda slots e bytes dos ponteiros, posições no ficheiro, intervalos `.pdata`, cadeias, resumos de bytes e instruções das seis descodificações locais. O executável foi lido sem execução; SHA-256 e data de modificação são iguais antes e depois. Não houve chamada REA/Ghidra nem escrita na sessão, no jogo ou nas fontes upstream por este agente.

As conclusões observadas descrevem nomes, argumentos e bytes; as derivadas combinam instruções verificadas; as inferenciais identificam possíveis papéis C++; as desconhecidas incluem fórmulas de interpolação, nomes C++ originais, comportamento temporal do jogo e a aplicação efectiva destes métodos aos seus assets.
