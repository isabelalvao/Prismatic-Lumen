# Validação XR e análise da sessão real — 15 de outubro de 2026

## Resultado

Nesta máquina, `tools/build.cmd -Test` concluiu com **19/19 CTest e 73/73 testes Python aprovados**. A sessão completa do Quest foi localizada, copiada sem alteração e analisada: integridade válida, mas seguimento das mãos intermitente. Ainda falta um ensaio com momentos de repouso, movimento e oclusão identificados para avaliar estabilidade e interpretar as perdas.

## Falhas anteriores e isolamento dos testes

As quatro asserções que falharam na primeira validação pertenciam a três métodos: perda de sessão, perda de instância e recentragem. Nesta análise passaram isoladamente, nas combinações aplicação nova/runtime antigo e aplicação antiga/runtime novo, e depois na bateria completa, antes de qualquer edição do motor ou dos testes. **A causa original não foi reproduzida nem demonstrada.** Não declarar correção de um defeito do motor.

O diagnóstico confirmou uma dependência externa dos testes: selecionar `XR_RUNTIME_JSON` não impedia o carregamento da camada implícita `XR_APILAYER_VIRTUALDESKTOP_oculus_compatibility`, instalada globalmente pelo Virtual Desktop. Os três lançadores de simulação em `tests/test_observation.py`, `tests/test_vr.py` e `tests/test_opening.py` passaram a definir `DISABLE_XR_APILAYER_VIRTUALDESKTOP_OCULUS_COMPATIBILITY=1` apenas no ambiente do processo filho. O nome vem do manifesto instalado; o código do loader fixado pelo projeto confirma o tratamento de `disable_environment`.

- [Diagnóstico anterior](loader-before.log): uma camada carregada.
- [Diagnóstico posterior](loader-after.log): zero camadas carregadas; origem muda no frame XR 4, como esperado.
- [Bateria anterior à edição](integration-before.log): 73/73.
- [Compilação e bateria após a edição](build-and-tests.log): 19/19 e 73/73.

Esta mudança isola a simulação; não prova que a camada tenha causado as falhas iniciais. Não foram alterados o motor, o runtime ativo do Windows ou os lançadores do Quest. O SHA-256 da aplicação continua `323128d6a0dc614685589e6d0c97e287e1de8ca71b3d4b2f5b9a7c5b905af3de`. Alterações revistas contra cópias delimitadas em `backup/`; verificadas também a configuração da raiz e a identidade do MASTER. Os testes gráficos podem atualizar as capturas existentes em `plan/`; não houve restauro global, commit ou push.

## Origem e integridade do registo real

O utilizador indicou a pasta:

`C:\Users\joaov\Downloads\Prismatic-Lumen-Q3-HandTracking-P0-2026-10-14\Prismatic-Lumen-Q3-HandTracking-P0-2026-10-14`

O ficheiro `logs/hand-tracking-01.jsonl` dessa pasta tem **61 714 982 bytes**. A cópia de trabalho está em `C:\DEV\Prismatic\logs\hand-tracking-01.jsonl`; origem e cópia têm SHA-256 `12cc88226df36106f28679c189eecc5bbb68d49a3a6c5e8c8d026f5b026bdfa8`. O original foi preservado. `logs/*.jsonl` está ignorado pelo Git; conservar a cópia e/ou o original em futuras migrações.

Executado `tools/inspect_session.py` com a identidade esperada do executável P0. [Análise completa](quest-real-analysis.json), [resumo CLI](quest-inspector.log) e [detalhe suplementar](quest-supplement.json).

- Sessão `e5c613d2ad8177b90e29d92cc905c2fa`; VirtualDesktopXR, 3072×3264 por olho, aplicação 0.12.0, base `bc86e22`.
- Hash do executável gravado igual ao P0 de referência: `d1c09d8a695a1af5d0833c6a6653625cfa8c74de9e02e36317795089407b66b0`. Configuração e MASTER coincidem com os ficheiros atuais da raiz/recursos.
- `valid_record=true`, sem erros; 3 858 observações e um `application_end`, accepted=3859, complete=true, normal_exit=true, dropped=0, write_failed=false no encerramento. O marcador declara flush final reportado por `recording_result`; o ficheiro não constitui uma medição independente de durabilidade física do disco.
- Intervalo observado de receção: **211,414 s**; publicação efetiva **18,244 Hz**, alvo 20 Hz. Intervalo mediano 55,53 ms, p95 60,94 ms, máximo 102,26 ms. Nenhuma lacuna de receção superior a 250 ms.
- Saída normal com 25 251 frames e tempo da cena **206,69 s**. Esta gravação não cobre os 424,182 s de Opening/MASTER; `complete=true` refere-se ao ficheiro.

## Cobertura e perdas observadas

As percentagens de amostras são contagens; as temporais estimam o estado entre receções consecutivas até 250 ms. Nenhuma prova continuidade entre consultas ou precisão física. Tempos nesta secção e no suplemento são relativos à primeira receção monotónica, não ao relógio musical. As velocidades usam o tempo XR das poses.

| Fonte | Amostras com posição válida e seguida | Cobertura temporal estimada | Intervalos sem posição utilizável | Recuperações observadas |
|---|---:|---:|---:|---:|
| Cabeça | 100% (3858/3858) | 100% | 0 | 0 |
| Mão esquerda | 81,57% (3147/3858) | 81,69% / 172,713 s | 8 | 8 |
| Mão direita | 60,32% (2327/3858) | 60,55% / 128,017 s | 38 | 38 |
| Ambas as mãos | 54,74% (2112/3858) | 55,12% / 116,529 s | — | — |

Nas mãos, as 26 articulações têm as mesmas percentagens de posição válida/seguida; os intervalos indisponíveis são `inactive`, sem erros de localização registados. Isto distingue ausência de seguimento de mão imóvel. Todas as perdas observadas terminam em recuperação dentro do registo, mas não foi registada uma intenção física que explique cada uma.

Principais intervalos sem posição utilizável:

| Fonte | Início–fim (s) | Duração |
|---|---:|---:|
| Esquerda | 3,126–26,827 | 23,701 s |
| Direita | 3,126–26,552 | 23,426 s |
| Direita | 63,949–80,222 | 16,273 s |
| Esquerda | 203,990–210,274 | 6,284 s |
| Direita | 148,151–154,226 | 6,075 s |

Há dez observações `not_rendering` entre 209,003 e 209,434 s e uma mudança de origem observada aos **209,676 s**. O registo é coerente com interrupção de derivadas através de ausência de renderização, perda e mudança de origem; não demonstra por si só uma recentragem manual.

## Coerência numérica e limites físicos

O inspetor reconstituiu os descritores sem divergências; não detetou valores numéricos inválidos. Há, contudo, picos de velocidade que exigem contexto físico:

- Esquerda: mediana 0,0119 m/s, p95 0,1846 m/s, máximo **3,820 m/s** aos 159,125 s.
- Direita: mediana 0,0458 m/s, p95 0,5056 m/s, máximo **13,175 m/s** aos 92,526 s; outro pico de 12,571 m/s nas proximidades. O maior corresponde a cerca de 0,744 m entre duas poses separadas por 56,44 ms XR, ambas marcadas válidas/seguidas, na mesma origem.
- A validade declarada pelo runtime não elimina saltos. Estes valores podem refletir movimento, relocalização ou outro comportamento do seguimento; sem referência física não é possível atribuir a causa ou usá-los diretamente para afinar motivos.

Conversão XR→QPC disponível em 100% das observações. Duração de consulta mediana 0,003 ms e p95 0,0049 ms; não mede o percurso físico de tracking/renderização. Diferença entre instante previsto e receção: mediana +34,200 ms, p95 +40,018 ms; três valores negativos no arranque, incluindo −368,768 ms na primeira observação. Não os interpretar como latência percetiva ou erro de sincronismo global demonstrado. Diferença áudio consultado–cena: mediana 10 ms, máximo 30 ms, sem medição física da saída sonora.

## Próxima ação

O utilizador crê ter ocultado/baixado as mãos e recentrado, mas não consegue confirmar momentos ou ações. Nenhuma perda ou mudança de origem foi atribuída a uma ação física concreta.

A pedido do utilizador, **adiar a afinação dos limiares até haver mãos 3D visíveis para diagnóstico**. A representação visual ajudará a comparar observação e movimento físico, mas não corrigirá o tracking. Quando for feita a afinação, recolher uma gravação controlada com mãos visíveis em repouso, movimentos lentos/rápidos, oclusão identificada e recentragem marcada; usar nome novo e conservar esta sessão. A aquisição e o registo P0 estão demonstrados, mas os limiares e a estabilidade para adaptação não estão validados. Não é preciso bloquear o desenvolvimento futuro de diagnóstico visual à espera dessa afinação. Nesta tarefa não foram implementados mãos desenhadas, adaptação audiovisual ou Jev.
