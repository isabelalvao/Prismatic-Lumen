# P1 — executor local de motivos audiovisuais

Implementado a pedido do utilizador em 2026-10-15, em `C:\DEV\Prismatic`. Versão do guião: `p1-scheduled-long-dip-1`. P0 e mãos 3D preservados; o utilizador reportou «funciona bem» para as mãos. Este P1 usa apenas tempo da sessão: sem decisões corporais, afinação de limiares, condições T/O/R ou Jev/LLM. Sem commit/push.

**Atualização do relógio, 2026-10-15:** a pedido do utilizador, números/separadores verdes enquanto `e(t)>0`; cor habitual quando a contribuição é neutra ou P1 está desligado. C continua a mostrar/esconder o relógio; pausa conserva a cor correspondente ao ponto congelado. Indicador partilhado por desktop, ambos os olhos VR e espelho, sujeito ao fade final existente. Compilação e verificação/capturas desta atualização em [timer-green.json](timer-green.json); `verification.json` conserva os resultados/hash da primeira entrega P1. Cópias delimitadas em `timer-green-backup/`.

## Ensaiar em VR

Na raiz do projeto, com Quest/PCVR e runtime OpenXR preparados:

```powershell
.\build\Release\fantasynt.exe --config .\visual-config.json --scene opening --vr --paused --motifs-scheduled --record-session .\logs\p1-vr-01.jsonl
```

Usar um nome de registo novo; não sobrescreve ficheiros. Espaço inicia/pausa/retoma; Q reinicia áudio/percurso/agenda no zero, conservando o estado de pausa; C alterna o contador; Esc encerra. O primeiro motivo começa aos 50 s, com máximo de atenuação aos 52 s. Para uma audição/observação curta da primeira entrada, acrescentar `--start-time 48` e usar outro nome de registo. Este excerto não substitui o ensaio integral de 7:04,18.

Sem `--motifs-scheduled`, conserva o comportamento anterior e ganho 1. Com a opção ativa, o registo é obrigatório e automático se o caminho for omitido: `logs/p1-<id>.jsonl`, relativo à pasta de execução, anunciado em `ready`. A configuração artística fica congelada durante P1: F5, presets, mudanças de cena e comandos v1 de alteração são recusados/registados. O ficheiro de configuração não é escrito. O ensaio exige Opening completo e emissão base 0,6; aceita a configuração atual da raiz.

## Curva e composição

`e(t)` usa smoothstep cúbico, derivada nula nas junções, entrada/saída de 2 s e patamar no intervalo restante. A curva é determinística, com pico 1 e integral `duração − 2` segundos. Nesta entrega usa-se a família **diminuição longa**, retomada/retemporizada nas oito oportunidades. As variantes de duas diminuições e o par desigual ficam para a biblioteca de seleção posterior; não há escolha de formas nem alegação de contingência nesta versão. O mesmo motivo e a mesma duração dão a mesma integral/pico em todas as execuções.

| ID | Origem prevista | Início–fim (s) | Ancestral previsto |
|---|---|---|---|
| M1 | iniciativa autónoma | 50–62 | — |
| M1.1 | retorno autónomo | 66–74 | M1 |
| M1.2 | retorno autónomo | 152–162 | M1.1 |
| M2 | iniciativa autónoma | 188–200 | M1.2 |
| M2.1 | retorno autónomo | 202–208 | M2 |
| M3 | iniciativa autónoma | 316–328 | M2.1 |
| M3.1 | retorno autónomo | 334–344 | M3 |
| M1.3 | retorno histórico autónomo | 370–382 | M1.2 |

Um comportamento principal por instante. Composição **depois** de `directed_visual_parameters`: `light_effective = light_directed − 0.06 e`; ganho estéreo `1 − 0.12 e`. Limite defensivo [0,1] na emissão; com a base congelada resulta [0,54;0,60], ganho [0,88;1]. Outros parâmetros são copiados integralmente. Composição/exposição/fade continuam no renderer, soberanos. Nada compensa o dip 208–220 s ou o fade 408,182–424,182 s. Contribuição nula nas travessias e a partir de 382 s, portanto antes do prazo de 392 s.

O ganho é calculado **por frame PCM estéreo**, na cópia em memória do WAV antes de enfileirar no SDL. Usa os índices originais de 44 100 Hz e a mesma curva/agenda do visual. Não altera o WAV, número/ordem de frames, taxa, relógio, pitch ou análise dos graves. Fora dos motivos o PCM fica exatamente igual, incluindo todo o final. Esta opção é específica do guião P1 imutável; um futuro executor de planos dinâmicos precisará de alimentação por blocos e respetiva validação. Não usa `SDL_SetAudioStreamGain`, chamadas remotas ou atualizações de ganho por frame gráfico; evita assim uma fonte de degraus/mutex no ciclo gráfico. Há quantização PCM16 até meio LSB.

## Relógios, cancelamento e memória

O visual/executor segue a posição consumida SDL. Em `--silent`, usa o relógio local limitado existente; `--fixed-time` permite prévias visuais e desliga áudio. Pausa manual suspende áudio/curva, cancela pendentes e abre nova epoch; retoma reagenda o futuro na nova epoch, conservando a curva ativa no ponto congelado. Q cancela a execução anterior, limpa a memória dessa execução e repõe áudio/percurso/agenda no zero. Não acumula atenuações nem reaplica oportunidades passadas.

Em P1/VR, frame indisponível ou cabeça sem posição/orientação válida **e seguida** suspendem áudio/executor. Mudança de origem suspende um frame, invalida pendentes e retoma com a amostra nova. As mãos continuam a ser adquiridas/desenhadas, sem comandarem o guião. A perda apenas das mãos não altera a agenda. O alinhamento é ao relógio consumido SDL, não uma medida de chegada de som/luz ao participante; buffers do dispositivo e streaming continuam externos.

Até dois motivos correntes, retenção de 90 s após o episódio, e arquivo limitado aos oito planos realmente iniciados por execução. Retornos históricos referenciam o arquivo aplicado. Um arranque em excerto não inventa ancestrais aplicados: regista `ancestor=null`, `planned_ancestor` separado e `origin=autonomous_excerpt` quando necessário. Prévia iniciada em pausa não promove memória. Encerramento normal ou exceção cancela planos; numa exceção o registo permanece incompleto.

## Registo e verificação

O JSONL existente mantém schema 1 e acrescenta manifesto `condition=p1_scheduled`, agenda/versão/curva/dose/método PCM, eventos `executor_event`/`motif_event` e tentativas de comandos recusados. Eventos identificam epoch, ID, origem/ancestral aplicado e previsto, tempos previstos/efetivos, curva, integral, envelope, contribuição e ganho. Observações a 20 Hz separam `base`, `directed`, `semantic_modulation` e `effective`, ganho comandado, clamp e pausa técnica. As curvas analíticas e o hash do executável permitem reconstituição; não são medidas físicas de volume/luminância. Continua o escritor assíncrono limitado de P0, com perdas/integridade explícitas.

O inspetor existente pode verificar integridade/aquisição, mas **não valida ainda os envelopes P1**:

```powershell
python .\tools\inspect_session.py .\logs\p1-vr-01.jsonl --output .\logs\p1-vr-01.analysis.json
```

Compilação/testes/evidência final em [verification.json](verification.json). Capturas temporárias ficam em `temp/`, sem substituir imagens históricas. Cópias delimitadas do estado anterior, incluindo alterações locais preexistentes, em [backup/](backup/); [estado Git inicial](git-status-before.txt).

- Compilação Release pelo fluxo local concluída; **20/20 CTest** sem `renderer_resize`, mais **teste gráfico separado aprovado** com saída `temp/p1-renderer/resize.png` e controlos em `temp/p1-renderer/controls/`. São os 21 testes C++ registados no CMake. [CTest](ctest.log), [gráfico](renderer.log).
- **82/82 testes Python aprovados**, incluindo sete testes novos de integração P1. [Relatório](python-tests.log). Comparação D3D11 com/sem P1 em 54/156/192/338 s mostra efeito; em 50/100/216/296/392/416/424,182 s dá imagens iguais à base, incluindo o fade. Pausa/retoma/Q/F5, comando v1 recusado, falha XR, registo automático e cena incompatível também verificados.
- Teste C++ percorre toda a agenda, confirma três iniciativas, continuidade `1e-4` nas junções, dose/integral, envelopes, memória aplicada, expiração, reinício e replay idêntico. PCM com extremos PCM16 confirma atenuação sem clipping, quantização até meio LSB, passo de ganho limitado, equivalência exata entre blocos de 512 frames e processamento integral e PCM final inalterado. O player com SDL dummy mantém duração, posição e pausa.
- Cinco ensaios adicionais com **áudio SDL dummy + runtime XR simulado**: normal, cabeça parcial inválida, frames omitidos, recentragem e perda de mãos; integridade JSONL válida em todos. Cabeça inválida conservou relógio/áudio exatamente em 54 s; frames omitidos/recentragem suspenderam e recuperaram; perda apenas das mãos não suspendeu.
- Capturas persistentes de comparação em `temp/p1-evidence-3402faef/`; registos dos cinco ensaios em `temp/p1-audio-xr-e816f03f/`. Imagens 54 s com/sem e anéis aos 338 s revistas visualmente: efeito global discreto; não é avaliação percetiva no headset.
- Revisão local contra as cópias delimitadas, verificando composição após direção artística, uniformidade estéreo, posição/frames PCM, congelamento da configuração, ordem de pausa XR/áudio e cancelamento em exceção. Sem problema bloqueante identificado; verificação de whitespace aprovada. Configuração da raiz/defaults e MASTER conservam hashes anteriores. Sem alteração a shaders, geometria, percurso ou aquisição/representação de mãos nesta tarefa.

Os logs anteriores de um teste de pausa falhado foram conservados em `temp/p1-integration.log`: a asserção esperava reagendamento após Q, mas Q cria uma execução nova. O ensaio foi corrigido para verificar retoma **antes** de Q e passou isoladamente e na bateria final. Não foi uma falha de pausa do motor.

### Verificação no equipamento real ainda necessária

Ouvir/ver o guião completo: detetabilidade e suavidade das entradas/saídas, ausência de clicks, dose nas fases de 50/152/188/334 s, alternância/paleta/pulsos conservados nos anéis, pausa/retoma/Q, headset sem pose/recuperação/recentragem e fim do MASTER/fade. Não declarar precisão, latência física, qualidade audível ou efeito percetivo a partir dos testes sintéticos. O relato «funciona bem» refere-se às mãos anteriores, não a P1. Não calibrar limiares corporais com este guião.
