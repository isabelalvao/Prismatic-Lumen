# Prismatic Lumen / Fantasynt — instruções locais para o Codex

Responder em português de Portugal, com explicações curtas e concretas. Prismatic Lumen é o nome atual; Fantasynt permanece nos identificadores técnicos. Projeto Windows x64, C++20, SDL3, bgfx/Direct3D 11 e OpenXR.

## Retomar noutra máquina

Ler primeiro [MIGRACAO.md](MIGRACAO.md) e [estado da passagem](plan/migration-2026-10-14/estado.md). Depois consultar [aquisição P0](plan/hand-observation-p0/README.md) e [próximo passo](plan/semantic-repertoires-v1/next-step-prompt.md). A continuação não depende do histórico deste chat nem de ficheiros externos em `.codex/attachments`.

Verificar `git status --short` antes de editar. O estado entregue tem alterações locais e ficheiros novos não comprometidos. Clonar apenas o GitHub em `bc86e22` perde P0 e as ferramentas novas. Preservar alterações existentes; não executar reset/clean/restauro global. Os caminhos históricos `C:\ORCA\Fantasynt` referem-se à origem; trabalhar na raiz efetivamente aberta.

## Estado e tarefa seguinte

- Efeitos locais das mãos atualizados a pedido de 2026-10-15: estalo com dispersão +25% e pontos +12%; `--vr --hand-light-pencil` acrescenta rasto curto e evanescente quando só o indicador está esticado, outros dedos/polegar recolhidos. Usar em conjunto com `--hand-snap` para os dois efeitos. Duração do traço 0,65 s, posições válidas/seguidas, sem unir perda/origem/segmentos. Funciona em pausa musical, sem alterar música/P1/percurso. Evidência/comando em `plan/hand-light-pencil-2026-10-15/README.md`; validação física do lápis pendente.

- Protótipo de estalo pedido em voz, 2026-10-15: `--vr --hand-snap`, contacto polegar–médio seguido de recolha/libertação rápida e 14 pontos luminosos junto à mão. Evidência/uso em `plan/hand-snap-2026-10-15/README.md`. É aproximação cinemática, sem microfone; validação física no Quest pendente. Funciona em pausa; não altera os motivos P1 nem o relógio/percurso. Sem a opção, comportamento anterior. Preservar a afinação atual da raiz (dose P1 alterada pelo utilizador para 0,60/0,90).

- P1 programado está implementado mediante pedido de 2026-10-15: `--motifs-scheduled`, emissão/ganho com agenda determinística, sem decisões corporais ou Jev/LLM. Execução/evidência em `plan/p1-motifs-2026-10-15/README.md`. Sem a opção, comportamento anterior/ganho 1. Validação percetiva/áudio no Quest continua pendente; as mãos foram reportadas pelo utilizador como «funciona bem».
- Afinação P1 exposta em `visual-config.json`: `motifs.light_attenuation` (0–0,60) e `motifs.master_attenuation` (0–1), defaults 0,06/0,12. Guardar e reabrir a aplicação; P1 mantém a dose congelada durante a sessão. Relógio verde assinala a curva ativa. O utilizador reportou não distinguir o efeito da dose inicial; não o tratar como efeito percetivo validado nem aumentar a dose sem escolha explícita.

- P0 de aquisição/registo de cabeça e mãos está implementado. Não voltar a implementá-lo a partir do prompt histórico.
- Os pacotes P0 históricos só observam/registam. A compilação de desenvolvimento passou a desenhar mãos a pedido do utilizador: esferas e segmentos das articulações OpenXR, apenas posições válidas e seguidas, sem retenção após perda. Evidência em `plan/vr-hands-2026-10-15/README.md`. Sem adaptação de luz/áudio ou Jev/LLM.
- Nesta máquina: mãos visíveis compiladas e verificadas com 19 CTest, um teste gráfico separado em `temp/` e 75 testes Python aprovados. Testes simulados isolam a camada global do Virtual Desktop; a causa das quatro asserções falhadas na primeira instalação não foi reproduzida.
- O JSONL real completo já foi analisado e está em `logs/hand-tracking-01.jsonl` (ignorado pelo Git). Evidência em `plan/migration-2026-10-14/xr-validation-2026-10-15/README.md`: integridade válida, cabeça 100%, esquerda 81,57%, direita 60,32% das amostras utilizáveis; perdas e picos de velocidade por validar fisicamente. Não voltar a pedir o ficheiro como se faltasse.
- O utilizador aceita adiar a afinação dos limiares até haver mãos 3D visíveis para diagnóstico. A representação visual foi pedida em 2026-10-15 e reportada como funcional; medições físicas no Quest continuam pendentes. Não calibrar com o ensaio anterior sem protocolo, nem tratar dados ausentes como repouso. A seleção relacional de motivos baseada no corpo continua por implementar e requer pedido próprio; P1 usa apenas agenda. O gesto de estalo explicitamente pedido é um efeito local independente.
- Preservar Opening, MASTER, percurso e configuração visual da raiz. Motivos adaptativos são um passo posterior à validação técnica e ao pedido do utilizador.

## Trabalho e verificação

Planos/evidência em `plan/`; temporários em `temp/`. Fazer cópias delimitadas antes de alterações significativas. Seguir a compilação/testes de MIGRACAO.md, com execução limitada dos testes gráficos. A raiz usa `visual-config.json`; `assets/visual-config.json` contém defaults. Não substituir a afinação da raiz por defaults.

Rever alterações de código e executar testes pertinentes. Separar simulação, dados reportados e medições verificadas no equipamento real. Git usa Conventional Commits; não fazer commit/push como parte da mera leitura ou migração documental.

Não há ligação Obsidian configurada. A memória durável está nos documentos locais, incluindo `plan/semantic-repertoires-v1/notes.md`; não criar uma ligação a um vault arbitrário nem exigir chaves/API para continuar.
