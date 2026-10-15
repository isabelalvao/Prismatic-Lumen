# Prismatic Lumen / Fantasynt — instruções locais para o Codex

Responder em português de Portugal, com explicações curtas e concretas. Prismatic Lumen é o nome atual; Fantasynt permanece nos identificadores técnicos. Projeto Windows x64, C++20, SDL3, bgfx/Direct3D 11 e OpenXR.

## Retomar noutra máquina

Ler primeiro [MIGRACAO.md](MIGRACAO.md) e [estado da passagem](plan/migration-2026-10-14/estado.md). Depois consultar [aquisição P0](plan/hand-observation-p0/README.md) e [próximo passo](plan/semantic-repertoires-v1/next-step-prompt.md). A continuação não depende do histórico deste chat nem de ficheiros externos em `.codex/attachments`.

Verificar `git status --short` antes de editar. O estado entregue tem alterações locais e ficheiros novos não comprometidos. Clonar apenas o GitHub em `bc86e22` perde P0 e as ferramentas novas. Preservar alterações existentes; não executar reset/clean/restauro global. Os caminhos históricos `C:\ORCA\Fantasynt` referem-se à origem; trabalhar na raiz efetivamente aberta.

## Estado e tarefa seguinte

- P0 de aquisição/registo de cabeça e mãos está implementado. Não voltar a implementá-lo a partir do prompt histórico.
- O executável P0 só observa/regista: sem mãos desenhadas, sem adaptação de luz/áudio, sem Jev/LLM.
- Testes simulados aprovados; o utilizador reportou aquisição inicial no Quest 3 com VirtualDesktopXR. A análise do JSONL real completo está pendente.
- Procurar ou pedir `hand-tracking-01.jsonl` completo e analisá-lo com `tools/inspect_session.py`. Quatro amostras iniciais ou `complete=true` não demonstram seguimento contínuo ou precisão física.
- Preservar Opening, MASTER, percurso e configuração visual da raiz. Motivos adaptativos são um passo posterior à validação técnica e ao pedido do utilizador.

## Trabalho e verificação

Planos/evidência em `plan/`; temporários em `temp/`. Fazer cópias delimitadas antes de alterações significativas. Seguir a compilação/testes de MIGRACAO.md, com execução limitada dos testes gráficos. A raiz usa `visual-config.json`; `assets/visual-config.json` contém defaults. Não substituir a afinação da raiz por defaults.

Rever alterações de código e executar testes pertinentes. Separar simulação, dados reportados e medições verificadas no equipamento real. Git usa Conventional Commits; não fazer commit/push como parte da mera leitura ou migração documental.

Não há ligação Obsidian configurada. A memória durável está nos documentos locais, incluindo `plan/semantic-repertoires-v1/notes.md`; não criar uma ligação a um vault arbitrário nem exigir chaves/API para continuar.
