# Próximo passo — observação sincronizada P0

Data: 2026-10-14 · O pedido de revisão da primeira sessão foi executado documentalmente. O prompt anterior fica preservado em [backups/2026-10-14-first-session/next-step-prompt.md](backups/2026-10-14-first-session/next-step-prompt.md), como arquivo histórico.

O plano ativo é [implementation-plan.md](implementation-plan.md), acompanhado pela [sessão](first-session.md), [matriz](behavior-matrix.md), [protocolo](first-exposure-protocol.md), [repertório](repertoire.md) e [evidência](notes.md). A próxima tarefa de programação é **P0**, sem adaptação nem Jev a alterar o ambiente.

Texto preparado para uma futura tarefa de implementação:

```text
Trabalha em Prismatic Lumen (identificadores Fantasynt), C:\ORCA\Fantasynt.
Implementa apenas P0 de plan/semantic-repertoires-v1/implementation-plan.md:
observação sincronizada de cabeça/mãos, validade/referência temporal e espacial,
estado audiovisual efetivo e manifesto/registo assíncrono de sessão.

Lê primeiro os documentos ativos da revisão 2 e verifica o estado Git.
Preserva a composição Opening, MASTER, percurso e parâmetros de referência.
Não acrescentes ainda políticas adaptativas, comandos por gesto ou decisão remota.

Ficheiros/módulos: src/vr/openxr_session.*, src/app/main.cpp,
src/interaction/ (observações), src/control/ (registo), interfaces de estado em
src/render/ e src/audio/, CMakeLists.txt e testes/runtime simulado relevantes.

Cumpre os seis critérios objetivos de P0, incluindo fixtures para repouso válido,
perda de mãos, cabeça inválida, recentragem e avanço virtual; relógios correlacionáveis;
estado depois da composição; filas não bloqueantes e diagnóstico explícito.
Só declara aquisição operacional após prova no Quest 3/runtime real.
Entrega código revisto, testes pertinentes, manifesto e amostra de registo;
marca separadamente o que foi provado no simulado e no equipamento real.
```

Este texto documenta o próximo trabalho; não inicia a programação nesta revisão.
