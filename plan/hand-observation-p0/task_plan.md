# P0 — aquisição e registo

2026-10-14. Implementar observações físicas de cabeça/mãos e registo opt-in, sem adaptação audiovisual.

- [x] Ler interfaces, testes, dependências e preservar ficheiros existentes.
- [x] Acrescentar testes de validade, origem, relógios e gravação.
- [x] Implementar aquisição OpenXR e observações normalizadas.
- [x] Implementar registo assíncrono com manifesto e estado efetivo.
- [x] Compilar, executar testes e rever o código.
- [x] Atualizar instruções/resultados, distinguindo simulado e Quest real.
- [ ] Provar aquisição/cobertura e comparar desempenho no Quest real — runtime configurado inexistente.

Decisões: `--record-session FILE.jsonl`; poses em metros no espaço LOCAL, sem câmara virtual; articulações completas; head space VIEW dedicado; origem segmentada por eventos XR; SHA-256 de recursos; nenhuma representação visual das mãos.

Referência anterior: backup delimitado em `backup/`; documentos de desenho já modificados pelo trabalho anterior devem ser preservados.

Estado: implementação local pronta; 19 testes CTest, 13 de observação integrada e 10 VR passaram. Revisão de código sem problemas críticos/altos. A aprovação completa de P0 requer hardware real; não foi implementada adaptação.

Ocorrências: a primeira invocação CTest foi prematura enquanto os novos executáveis compilavam; repetida depois da compilação, com sucesso. Regressão de renderização gerou 15 PNG de controlos; repostos a partir de HEAD/LFS depois de confirmar que eram outputs desta execução. `git restore` foi impedido por `.git/index.lock` preexistente nessa altura; o lock não foi removido e o índice não foi alterado. O restauro por leitura de HEAD/LFS não dependeu do índice. Duas advertências de casts no mock e um ficheiro Python não fechado foram corrigidos e reverificados.

Próxima prova: seguir [README.md](README.md), configurar o percurso OpenXR/PCVR real, gravar movimentos/repouso/perdas/recentragem e inspecionar o JSONL. Os testes simulados e o exemplo não substituem esse ensaio.

## Pedido posterior: pasta para outro PC

- [x] Preparar pasta e ZIP com nome próprio, preservando os pacotes anteriores.
- [x] Incluir build atual, perfil aprovado, shaders, malhas/texturas, MASTER, loader, DLLs CRT e licenças.
- [x] Acrescentar lançadores e verificador Windows PowerShell5.1, sem Python obrigatório no destino.
- [x] Verificar hashes, extração externa, execução dos scripts, gravações únicas e diagnósticos de ausência/falha.
- [x] Rever alterações de empacotamento e entregar caminhos concretos.

Artefacto: `dist/Prismatic-Lumen-Q3-HandTracking-P0-2026-10-14/` e ZIP correspondente. Cerca de 79 MiB comprimidos. Ver `package-check.json`. Corrigidas na revisão uma variável de identidade sobrescrita no empacotador e a transformação de stderr nativo em erro terminante no PowerShell5.1. O runtime simulado externo usa manifesto num caminho ASCII por limitação de abertura do loader; a aplicação/scripts/gravações foram testados em pasta com espaços e acentos. Nenhuma nova alteração ao motor nesta tarefa de distribuição; nenhuma prova física alegada.
