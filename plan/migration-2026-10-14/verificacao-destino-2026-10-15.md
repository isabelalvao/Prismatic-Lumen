# Verificação no destino — 15 de outubro de 2026

Raiz: `C:\DEV\Prismatic`. Leitura de migração, estado, P0, plano semântico, diário e interfaces de aquisição/registo. Sem alterações ao motor, configuração, recursos ou runtime; sem commit/push.

## Estado conservado

- HEAD `bc86e22`; alterações locais e fontes novas P0 presentes, incluindo tracking, observações, gravador e inspetor. O estado Git normal corresponde ao trabalho não comprometido descrito na passagem.
- `build/` e `dist/` foram copiados. SHA-256 de `build/Release/fantasynt.exe`: `d1c09d8a695a1af5d0833c6a6653625cfa8c74de9e02e36317795089407b66b0`, igual à referência P0.
- Os 12 recursos binários nas extensões LFS de `assets/` foram inspecionados: nenhum é ponteiro LFS; total 77 942 039 bytes. Não foi feita auditoria de todos os binários históricos em `plan/`.
- Configurações da raiz e de `assets/` coincidem atualmente: SHA-256 `01d5bbc348c3f25337affec6463d515abd2d17adcbf894e25934e67fa87faf17`. Nenhuma foi substituída.
- `.git/index.lock` existe, com 51 182 bytes; conservado. Antes de futuras operações de escrita Git, confirmar ausência de operações ativas e tratar este bloqueio copiado de forma delimitada.

## Verificações nesta máquina

- Inspetor: 15/15 testes Python aprovados.
- Observação integrada: 13/13 testes aprovados com o executável copiado, Direct3D e runtime OpenXR simulado; inclui preservação de registos, perdas, recentragem e igualdade da imagem com/sem gravação.
- Estes resultados não constituem recompilação nem teste físico no Quest. Não foi repetida toda a bateria CTest.
- Python do PATH é um alias WindowsApps que não arrancou. Os testes usaram `C:\Users\joaov\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe`.
- Git LFS falhou dentro do isolamento do Codex; `git status --short` funcionou fora dele. O estado obtido com o filtro LFS desativado marcou binários artificialmente como modificados e não deve ser usado como diagnóstico de corrupção.
- A primeira tentativa dos testes do inspetor falhou por permissões nos temporários do isolamento; a repetição fora dele passou integralmente.

## Preparação pendente

- Não foram encontrados `vswhere.exe` no caminho padrão, ferramentas `cl`, `cmake` ou `ctest` no PATH, nem diretórios Visual Studio nos locais padrão consultados. Instalar/verificar Build Tools C++ x64, Windows SDK e CMake antes de recompilar.
- A cache copiada usa `CMAKE_HOME_DIRECTORY=C:/ORCA/Fantasynt` e gerador Visual Studio 18 2026. Não a reutilizar. Conservar `build/` com outro nome antes de criar a compilação nova, conforme MIGRACAO.md; esta verificação não moveu a pasta.
- O Windows aponta para `C:\Program Files\Virtual Desktop Streamer\OpenXR\virtualdesktop-openxr.json`, e o ficheiro existe. Isto confirma configuração local, não ligação ao headset ou aquisição real. Não foi alterado o registo Windows.
- Não foi encontrado `hand-tracking-01.jsonl` entre os ficheiros pesquisados no projeto. O exemplo em `plan/hand-observation-p0/` é simulado.

## Continuação

Preparar ferramentas e recompilar/testar numa pasta nova. Trazer o JSONL real completo para `logs/` e executar `tools/inspect_session.py`, preservando a origem. Validar cobertura, perdas/recuperação e coerência temporal antes de avançar para o executor de motivos. P0 já existe; adaptação audiovisual, mãos desenhadas e Jev continuam fora desta verificação.
