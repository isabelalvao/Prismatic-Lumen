# Prismatic Lumen / Fantasynt — instruções locais para o Codex

Responder em português de Portugal, com explicações curtas e concretas. Prismatic Lumen é o nome atual; Fantasynt permanece nos identificadores técnicos. Projeto Windows x64, C++20, SDL3, bgfx/Direct3D 11 e OpenXR.

## Retomar noutra máquina

Ler primeiro [MIGRACAO.md](MIGRACAO.md) e [estado da passagem](plan/migration-2026-10-14/estado.md). Depois consultar [aquisição P0](plan/hand-observation-p0/README.md) e [próximo passo](plan/semantic-repertoires-v1/next-step-prompt.md). A continuação não depende do histórico deste chat nem de ficheiros externos em `.codex/attachments`.

Verificar `git status --short` antes de editar. O estado entregue tem alterações locais e ficheiros novos não comprometidos. Clonar apenas o GitHub em `bc86e22` perde P0 e as ferramentas novas. Preservar alterações existentes; não executar reset/clean/restauro global. Os caminhos históricos `C:\ORCA\Fantasynt` referem-se à origem; trabalhar na raiz efetivamente aberta.

## Estado e tarefa seguinte

- P0 de aquisição/registo de cabeça e mãos está implementado. Não voltar a implementá-lo a partir do prompt histórico.
- O executável P0 só observa/regista: sem mãos desenhadas, sem adaptação de luz/áudio, sem Jev/LLM.
- Nesta máquina: compilação e 19 CTest/73 testes Python aprovados. Testes simulados isolam a camada global do Virtual Desktop; a causa das quatro asserções falhadas na primeira instalação não foi reproduzida.
- O JSONL real completo já foi analisado e está em `logs/hand-tracking-01.jsonl` (ignorado pelo Git). Evidência em `plan/migration-2026-10-14/xr-validation-2026-10-15/README.md`: integridade válida, cabeça 100%, esquerda 81,57%, direita 60,32% das amostras utilizáveis; perdas e picos de velocidade por validar fisicamente. Não voltar a pedir o ficheiro como se faltasse.
- O utilizador aceita adiar a afinação dos limiares até haver mãos 3D visíveis para diagnóstico. Não calibrar com este ensaio sem protocolo, nem tratar dados ausentes como repouso. Representação visual e adaptação continuam por implementar; não inferir autorização para as implementar desta nota.
- Preservar Opening, MASTER, percurso e configuração visual da raiz. Motivos adaptativos são um passo posterior à validação técnica e ao pedido do utilizador.

## Trabalho e verificação

Planos/evidência em `plan/`; temporários em `temp/`. Fazer cópias delimitadas antes de alterações significativas. Seguir a compilação/testes de MIGRACAO.md, com execução limitada dos testes gráficos. A raiz usa `visual-config.json`; `assets/visual-config.json` contém defaults. Não substituir a afinação da raiz por defaults.

Rever alterações de código e executar testes pertinentes. Separar simulação, dados reportados e medições verificadas no equipamento real. Git usa Conventional Commits; não fazer commit/push como parte da mera leitura ou migração documental.

Não há ligação Obsidian configurada. A memória durável está nos documentos locais, incluindo `plan/semantic-repertoires-v1/notes.md`; não criar uma ligação a um vault arbitrário nem exigir chaves/API para continuar.
