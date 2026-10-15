# Estado para continuar no novo PC

**Nota de 2026-10-15:** este documento preserva a passagem histórica. A atualização do GitHub inclui agora o trabalho posterior: P0/inspetor, mãos visíveis, P1 programado, estalo, lápis e afinação dos anéis. O JSONL real já foi analisado; os resultados estão em [xr-validation-2026-10-15/README.md](xr-validation-2026-10-15/README.md). Para retomar, prevalecem [AGENTS.md](../../AGENTS.md) e a atualização no início de [MIGRACAO.md](../../MIGRACAO.md). Os registos reais e pacotes locais continuam fora do Git.

14 de outubro de 2026. Passagem documental, sem commit/push, mudança de runtime ou nova compilação. Origem: `C:\ORCA\Fantasynt`; adaptar caminhos à raiz efetiva no destino.

## Base e objetivo

Prismatic Lumen é um ambiente audiovisual imersivo e artefacto de investigação sobre agência atribuída e autoria ambígua. Fantasynt permanece nos identificadores. Windows x64, C++20, SDL3, bgfx/Direct3D 11 e OpenXR. Opening/MASTER duram 424,1821315192744s, com fade visual final de 16s. Preservar composição, recursos, percurso e perfil visual da raiz.

HEAD: `bc86e22` (`docs(workflow): record GitHub publication`). O código atual contém alterações locais rastreadas e ficheiros novos: [estado antes desta documentação](git-status-before-docs.txt). Clone/checkout desse commit e ZIP portátil não recompõem por si só o estado de desenvolvimento atual.

## Implementado

- Aquisição opcional `XR_EXT_hand_tracking`: cabeça VIEW relativo a LOCAL e 26 articulações por mão em metros; validade/seguimento separados, atividade/ausência/erro e mudanças de origem.
- Descritores apenas com dados válidos/seguidos; sem atravessar perda, mudança de origem ou intervalo XR >250ms. Movimento físico separado da câmara virtual.
- `--record-session FILE.jsonl`: manifesto com hashes, observações/estado audiovisual, comandos/configuração e encerramento; fila limitada e escrita assíncrona. Publicação alvo 20Hz, transições técnicas imediatas e recusa de sobrescrita.
- `tools/inspect_session.py` + `tools/session_metrics.py`: integridade, reconstituição, cobertura por cabeça/mão/articulação, perdas/recuperação observada, origem e tempos. Versão `p0-inspector-2`, schema JSONL 1, Python 3.10+ sem bibliotecas adicionais.
- Dist P0 r2 e atualização pequena das ferramentas; scripts PowerShell 5.1 e instruções próprias do dist.

Não implementado: mãos desenhadas, adaptação de luz/áudio, executor de motivos, memória semântica operacional ou Jev/LLM. Ganho MASTER=1. O desenho em `plan/semantic-repertoires-v1/` continua uma proposta para estes passos.

Fontes principais: `src/vr/tracking.*`, `src/vr/openxr_session.*`, `src/interaction/observation.*`, `src/control/session_recorder.*`, `src/app/main.cpp`, `CMakeLists.txt`, ferramentas/testes correspondentes. Cópias anteriores em `plan/hand-observation-p0/backup/` e `plan/semantic-repertoires-v1/backups/`.

## Provas e limites

Compilação Release/revisão concluídas. Na origem: 19 CTest, 13 testes de observação integrada e 10 VR aprovados; revisão do inspetor com 15 testes controlados aprovados. Dist r2 testado fora do desenvolvimento em aquisição normal, sem mãos e falha XR. Atualização sobre o dist original verificada com preservação de executável/recursos/JSONL. Esta tarefa de migração não voltou a executar a bateria do motor.

Relato do utilizador: Quest 3/VirtualDesktopXR, trackers preparados, quatro observações iniciais com ambas as mãos ativas/válidas/seguidas; accepted=3859, complete=true, dropped=0, write_failed=false; FPS médio=119,4104643 em 211,4638792s. **O JSONL real completo ainda não está no projeto.** Não foram recalculadas cobertura, perdas ou precisão física. Não confundir esses dados reportados com análise local do registo.

SHA-256 do executável P0 de referência, cujo hash coincide com o reportado pelo utilizador:

```text
d1c09d8a695a1af5d0833c6a6653625cfa8c74de9e02e36317795089407b66b0
```

`build_commit=bc86e22` identifica a base Git; P0 foi compilado com alterações locais não comprometidas. Depois da recompilação no novo PC, registar o novo hash; não exigir identidade binária com o executável antigo.

O diagnóstico da origem apontava para um manifesto Oculus inexistente; o registo Windows não foi alterado. Isto é distinto do teste VDXR real reportado no outro PC. Foi encontrado `.git/index.lock`, não removido; não o transportar. Nenhum vault Obsidian está configurado: memória nos documentos locais.

## Retomar

1. Confirmar fontes novas/alterações locais, recursos LFS reais e afinação de `visual-config.json`; preparar `build/` novo e compilar/testar.
2. Obter `hand-tracking-01.jsonl` completo do PC de teste. O relato está resumido aqui e no guia P0; não depende dos anexos externos do Codex antigo.
3. Analisar e documentar cobertura, perdas/recuperações e coerência temporal; comparar com o movimento feito fisicamente sem confundir reconstituição numérica com precisão medida.
4. Se necessário, fazer novo ensaio Quest com repouso, movimento, oclusão e recentragem, preservando registos anteriores. Considerar motivos adaptativos só depois, dentro do pedido do utilizador e do plano ativo.

Referências: [migração](../../MIGRACAO.md), [P0](../hand-observation-p0/README.md), [inspeção pendente](../hand-observation-p0/inspection-plan.md), [verificação r2](../hand-observation-p0/package-check-r2.json), [verificação da atualização](../hand-observation-p0/inspection-update-check.json), [sessão proposta](../semantic-repertoires-v1/first-session.md), [matriz](../semantic-repertoires-v1/behavior-matrix.md), [diário](../semantic-repertoires-v1/notes.md).
