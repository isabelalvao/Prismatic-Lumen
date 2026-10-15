# Próximo passo — novo PC e análise do JSONL real

Atualizado em 2026-10-14. **P0 já está implementado.** A versão anterior deste prompt, que propunha implementá-lo, está preservada em [backup da migração](../migration-2026-10-14/backup/next-step-prompt.md). Não repetir essa implementação.

Ler [AGENTS.md](../../AGENTS.md), [MIGRACAO.md](../../MIGRACAO.md), [estado da passagem](../migration-2026-10-14/estado.md) e [P0](../hand-observation-p0/README.md). O desenho ativo continua em [implementation-plan.md](implementation-plan.md), [sessão](first-session.md), [matriz](behavior-matrix.md), [protocolo](first-exposure-protocol.md), [repertório](repertoire.md) e [diário](notes.md).

Texto para iniciar a continuação:

```text
Continua Prismatic Lumen / Fantasynt na raiz atual do projeto.
Lê AGENTS.md, MIGRACAO.md e plan/migration-2026-10-14/estado.md.
Verifica git status e preserva alterações locais e ficheiros novos.
bc86e22 é o commit de base; não contém por si só todo o P0 entregue.

Se esta é a primeira execução no novo PC, verifica recursos LFS, compilador
e configuração visual; usa build/ novo e tools/build.cmd.
Executa as verificações pertinentes descritas em MIGRACAO.md.

P0 de observação/registo já existe. Não o voltar a implementar.
Localiza logs/hand-tracking-01.jsonl completo ou pede o ficheiro ao utilizador.
O relato anterior só confirma aquisição inicial reportada no Quest 3/VDXR;
não substitui o registo. Se disponível, usa tools/inspect_session.py e guarda
a análise, preservando o JSONL de origem.

Relata integridade, atividade/validade por mão, cobertura observada,
lacunas/perdas/recuperação, mudanças de origem e coerência dos descritores.
Distingue simulação, relato do utilizador e dados efetivamente analisados.
Não inferir intenção, precisão física ou latência percetiva destes valores.

Preserva Opening, MASTER, percurso e visual-config.json da raiz.
Não acrescentar adaptação de luz/áudio, mãos desenhadas ou Jev/LLM durante
esta validação. O executor local de motivos é um passo posterior.
Atualiza a evidência e o diário local depois de concluir a verificação.
```

Sem o JSONL completo, a análise real permanece pendente; ainda é possível verificar a instalação, compilar/testar e preparar novo ensaio técnico no equipamento real.
