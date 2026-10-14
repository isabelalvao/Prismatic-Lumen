# Integração OpenXR e pacote PCVR

Execução nativa nesta sessão, com revisão independente no fim. Especificação: `2026-10-12-quest3-vr-design.md`. Pasta existente sem Git; fontes anteriores serão salvaguardadas em temp durante a alteração, e os registos de validação ficam em plan.

- [x] Escrever testes da matemática de vistas e observar falha antes da implementação; criar src/vr/view_math.hpp/.cpp.
- [x] Fixar OpenXR-SDK; implementar src/vr/openxr_session.hpp/.cpp com dispositivo D3D11, sessão, imagens, tracking previsto, eventos e diagnóstico.
- [x] Escrever runtime simulado e integração VR; observar falha antes da ligação ao renderer/aplicação.
- [x] Integrar dispositivo/texturas externos no renderer e perspectivas por olho, espelho e composição de cor; acrescentar --vr/--xr-probe ao arranque. Preservar execução de monitor e contrato JSON.
- [x] Compilar e executar testes completos; verificar runtime real sem headset com diagnóstico honesto.
- [x] Criar pacote Windows x64 com dependências locais/licenças, lançadores e instruções Quest 3/VDXR; extrair noutra pasta e verificar o pacote.
- [x] Rever independentemente o conjunto de alterações, corrigir problemas relevantes e repetir verificações necessárias. Entregar ZIP e checksum, distinguindo testes locais do teste físico pendente.

Concluído: versão 0.2.0 em dist, CTest 5/5, Python 14/14, pacote extraído em pasta com espaços e acentos aprovado em desktop e OpenXR simulado, incluindo cliente JSON PowerShell. Relatório: `2026-10-12-quest3-vr-validation.md`. Backup da versão anterior conservado em `plan/backups/vr-0.1.0`; ficheiros temporários de teste removidos. Sem inicializar Git. Teste físico Quest 3/VDXR e fidelidade visual ao original continuam pendentes.
