# Instalação no destino — 15 de outubro de 2026

Pedido do utilizador: instalar as ferramentas em falta em `C:\DEV\Prismatic`.

## Instalado

- Visual Studio Build Tools 2026, pacote 18.10.4, instalação 18.10.12306.391.
- Workload C++ VCTools com componentes recomendados e CMake: MSVC 14.51.36231 (compilador 19.51.36260), Windows SDK 10.0.26100 e CMake 4.3.1-msvc1.
- Python 3.13.15 para o utilizador em `C:\Users\joaov\AppData\Local\Programs\Python\Python313`; acrescentado ao PATH do utilizador pelo instalador.
- Git para Windows 2.55.0.windows.5, incluindo Git LFS 3.7.1, em `C:\Program Files\Git`.

Instaladores obtidos através de winget; hashes verificados pelo gestor. Não foi solicitado reinício automático. Os testes do inspetor passaram 15/15 com o novo Python.

## Compilação

A pasta copiada foi movida de forma delimitada para `temp/build-origem-2026-10-15`, conservando executável, recursos e cache antigos. A configuração nova em `build/` reconheceu compilador/SDK e descarregou as dependências fixadas pelo projeto.

Comando executado: `tools/build.cmd -Test`, com o Python novo no PATH do processo. Registo em `temp/build-destino-2026-10-15.log`.

- Compilação Release concluída, incluindo aplicação, recursos, runtime simulado e testes.
- CTest: 19/19 aprovados.
- Integração Python: 73 testes executados; 3 métodos falharam, com 4 falhas de asserção porque um deles testa dois modos. Restantes 70 métodos aprovados.
- Falhas reproduzidas isoladamente: `test_aborted_runtime_produces_incomplete_record` (saída 0 inesperada), `test_recenter_takes_effect_at_requested_xr_time` (sem nova origem), `test_runtime_loss_is_explicit_error` (saída 0 nos modos loss-session/loss-instance). O motivo ainda não foi diagnosticado; não declarar a bateria integral aprovada ou atribuir a causa ao compilador.
- Os testes usam runtime simulado; não houve validação no headset. O binário copiado anterior tinha passado os 13 testes de observação nesta máquina.
- SHA-256 do novo executável: `323128d6a0dc614685589e6d0c97e287e1de8ca71b3d4b2f5b9a7c5b905af3de`.
- Os testes gráficos atualizaram capturas rastreadas em `plan/controls/` e `plan/m1-integration.png`; conservadas, sem restauro global. Rever o estado Git antes do próximo trabalho.

Não foram alterados código, configuração visual, áudio, runtime OpenXR ativo ou bloqueio Git copiado. Novos terminais recebem o PATH atualizado; o script de compilação deteta as Build Tools e o CMake pelo vswhere. A preparação das ferramentas está concluída; a investigação das falhas de integração é trabalho seguinte.

Seguimento posterior em 2026-10-15: a bateria passou 73/73 antes de alterações; causa das falhas iniciais não reproduzida. Isolada a camada global do Virtual Desktop nos processos de teste; nova execução de `tools/build.cmd -Test` passou 19/19 CTest e 73/73 Python. O JSONL real completo também foi localizado e analisado. [Evidência e limites](xr-validation-2026-10-15/README.md).
