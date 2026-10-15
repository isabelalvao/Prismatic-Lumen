# Migrar Prismatic Lumen para outro PC com Codex

**Atualização de 2026-10-15:** a sincronização do projeto com o GitHub inclui agora as fontes P0, o inspetor, mãos visíveis, P1 programado, estalo, lápis e respetivas afinações/evidências. Para uma máquina nova, clonar a versão atual de `master`, executar `git lfs pull` e seguir a compilação abaixo. A configuração afinada da raiz está versionada. Copiar separadamente os registos reais de `logs/` e os pacotes de `dist/`, que continuam ignorados. As referências abaixo a trabalho não comprometido e ao commit `bc86e22` descrevem a passagem histórica de 14 de outubro; esse commit isolado continua a não conter as funcionalidades novas. O estado atual e as validações físicas pendentes estão em [AGENTS.md](AGENTS.md).

Estado de 14 de outubro de 2026. Origem: `C:\ORCA\Fantasynt`. O destino pode usar outro caminho; executar os comandos na raiz que contém este ficheiro e `CMakeLists.txt`. Um caminho simples como `C:\ORCA\Fantasynt` facilita os testes OpenXR.

## Copiar o projeto atual

**Copiar a pasta de trabalho, incluindo ficheiros ocultos e novos. Clonar apenas o GitHub não basta:** P0, o inspetor e parte da documentação são alterações locais não comprometidas sobre `bc86e22`. O ZIP portátil serve para testar, mas não substitui o projeto para desenvolver.

| Conteúdo | Tratamento |
|---|---|
| `src/`, `tests/`, `tools/`, `cmake/`, `shaders/`, `examples/` | Copiar integralmente, incluindo ficheiros novos não rastreados. |
| `assets/`, `visual-config.json`, `CMakeLists.txt` e restantes ficheiros da raiz | Copiar; preservar áudio, malhas e afinação da raiz. |
| `plan/`, `docs/`, `AGENTS.md`, `MIGRACAO.md` | Copiar; são o contexto, evidências, cópias anteriores e próximos passos. |
| `.git/`, `.gitattributes`, `.gitignore` | Copiar para conservar histórico, estado e objetos LFS; excluir o lock transitório indicado abaixo. |
| `dist/` | Copiar para conservar os pacotes/executável de referência. Está ignorado pelo Git. |
| `logs/` e gravações no PC de teste | Conservar e copiar separadamente. O JSONL real ainda não está nesta pasta de desenvolvimento. |
| `build/`, `temp/`, `__pycache__/` | Dispensáveis; recompilar com uma pasta `build/` nova. |

Foi encontrado `.git/index.lock` na origem; não o copiar. É transitório, não constitui histórico ou código e não foi removido nesta tarefa. Terminar operações Git antes da cópia. Copiar para uma pasta nova e conservar a origem até confirmar a compilação no destino.

Git LFS conserva os recursos binários: levar os ficheiros reais, não apenas ponteiros LFS. Se usares um clone como base, executar `git lfs pull` e depois repor todos os ficheiros alterados/novos da pasta de trabalho atual, sem perder a afinação. GitHub não recupera alterações locais ainda não publicadas.

O chat e as skills globais da instalação Codex não viajam com o projeto. `AGENTS.md` e os documentos locais permitem retomar. Não é necessário copiar a conta, credenciais ou instalação inteira do Codex.

## Preparar e compilar

Componentes necessários: Windows x64, controlador gráfico Direct3D 11, Visual Studio Build Tools com ferramentas C++ x64/Windows SDK/CMake, Git/Git LFS e Python 3.10+ para testes/inspeção. A primeira configuração precisa de Internet para descarregar os commits fixados em `cmake/Dependencies.cmake` e `cmake/GraphicsDependencies.cmake`.

Na origem: Build Tools 2026, MSVC 14.51 e CMake 4.3.1. `tools/build.ps1` deteta instalações Build Tools 2022/2026 e procura o CMake incluído. Não reutilizar caminhos absolutos de ferramentas da origem. Estes testes não precisam de Unity, Unreal, Node ou bibliotecas Python adicionais.

Abrir a pasta copiada no Codex. Se copiaste `build/`, conservá-la com outro nome antes de compilar: a cache tem caminhos/compilador da máquina antiga. Deixar o script criar `build/` novo. No PowerShell, dentro da raiz:

```powershell
git status --short
git lfs install
python --version
.\tools\build.cmd
```

O executável fica em `build\Release\fantasynt.exe`, com os recursos ao lado. Para compilar e testar tudo:

```powershell
.\tools\build.cmd -Test
```

Verificações específicas, depois da compilação:

```powershell
python -m unittest discover -s tests -p test_session_inspector.py -v
python -m unittest discover -s tests -p test_observation.py -v
```

Os testes de observação usam runtime simulado; não provam qualidade no Quest. Testes gráficos podem atualizar capturas em `plan/`: verificar o estado Git e conservar alterações deliberadas. Não fazer restauro global para limpar resultados.

## Testar no Quest 3 e analisar o registo

O utilizador já reportou arranque real com VirtualDesktopXR e aquisição inicial das duas mãos no PC de teste. Configurar Virtual Desktop/VDXR nesse PC, seguimento das mãos no Quest e encaminhamento de dados de tracking para o PC. O runtime é configuração da máquina, não vem na pasta do projeto. O executável é Windows/PCVR, não um APK.

Na raiz do projeto:

```powershell
.\build\Release\fantasynt.exe --xr-probe
.\build\Release\fantasynt.exe --config .\visual-config.json --vr --paused --record-session logs\quest-novo-pc-01.jsonl
```

Usar um nome novo por gravação: o programa recusa substituir o ficheiro. Espaço inicia/pausa; Esc termina e finaliza o registo. As mãos não são desenhadas em P0. Testar movimento, repouso, oclusão/recuperação e recentragem. Opening/MASTER completo dura 7:04,18.

Para analisar a gravação existente, copiar o JSONL completo do PC de teste para `logs/`:

```powershell
python .\tools\inspect_session.py .\logs\hand-tracking-01.jsonl --output .\logs\hand-tracking-01.analysis.json
```

No dist r2, o mesmo comando funciona na pasta de `fantasynt.exe`. `Verify-HandTracking.cmd` dá um resumo básico sem Python. O inspetor completo mede cobertura observada, lacunas, perdas/recuperação, tempos e reconstituição das velocidades; não mede precisão física ou latência percetiva. `complete=true` refere-se ao registo, não à continuidade do seguimento.

## Iniciar a conversa no novo Codex

O Codex deve ler `AGENTS.md` ao trabalhar nesta raiz. Mensagem sugerida:

> Continua este projeto a partir de AGENTS.md, MIGRACAO.md e plan/migration-2026-10-14/estado.md. Preserva as alterações locais. P0 já está implementado. Verifica a instalação/compilação no novo PC e retoma a análise do JSONL real completo antes de avançar para adaptação audiovisual.

Estado/evidências: [estado.md](plan/migration-2026-10-14/estado.md). Tarefa ativa: [next-step-prompt.md](plan/semantic-repertoires-v1/next-step-prompt.md).
