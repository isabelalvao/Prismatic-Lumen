# Compilação local de REA main

Estado final: CLI e fornecedor Ghidra Windows prontos segundo os verificadores públicos. O complemento nativo foi compilado experimentalmente com GNU MinGW alojado no Windows; não foi usado o percurso oficial Linux. A prontidão do fornecedor não estabelece sucesso da análise do Fantasynth.

## Proveniência

- Repositório: https://github.com/morluto/rea
- Commit fixado: df2c08b546d4f40103e11b8b31dbfc3d41ee687b, de 2026-10-12T13:49:15Z.
- ZIP oficial: https://codeload.github.com/morluto/rea/zip/df2c08b546d4f40103e11b8b31dbfc3d41ee687b
- SHA-256: 3cbfdc576fa2f9ba6675401400d54d9917ba2e8579778d93135ba7225a156690.
- Arquivo preservado em rea-main-source.zip neste diretório; fontes em C:/ORCA/Fantasynt/tools/vendor/rea-main-source/rea-df2c08b546d4f40103e11b8b31dbfc3d41ee687b.
- Node existente 24.21.0 e npm 11.19.0. Upstream fixa Node 24.18.0/npm 11.16.0 para desenvolvimento; os existentes satisfazem os engines declarados, mas não reproduzem exatamente essas versões.
- Não foram inicializados repositórios Git, alterados perfis/configurações globais nem modificadas fontes/scripts upstream. source-integrity-after-build.json confirma os hashes dos quatro ficheiros nativos, do builder, do verificador e do carregador.

## Compilação JavaScript

AGENTS.md, CONTRIBUTING.md, native/windows/README.md, docs/installation.md e scripts oficiais foram consultados.

- npm ci: código 0; 375 pacotes; auditoria sem vulnerabilidades. HUSKY=0 apenas no processo evita configurar hooks.
- npm run build: código 0; metadata:check, compile e artifacts:skills concluídos.
- CLI pública node scripts/rea.mjs --help: código 0.
- npm run test:local -- src/ghidra/GhidraDisabledAnalyzers.test.ts: código 0; 10 testes passaram.
- Launcher: C:/ORCA/Fantasynt/tools/vendor/rea-main-source/rea-df2c08b546d4f40103e11b8b31dbfc3d41ee687b/scripts/rea.mjs.

## Compilação experimental do complemento Windows

O comando upstream npm run build:windows-native continua a recusar Windows: The Windows x64 artifact build lane currently requires Linux and MinGW-w64. Não foi editado nem executado com uma plataforma falsificada. WSL não tem distribuições instaladas; MSVC não foi utilizado.

Uma receita separada, build-windows-hosted-gnu.mjs, reproduz as fontes, símbolos Node-API, flags GNU e manifesto do builder upstream utilizando WinLibs portátil. Ferramentas e headers ficam em tools/vendor/rea-main-source/toolchain/.

- WinLibs oficial: https://github.com/brechtsanders/winlibs_mingw/releases/tag/16.2.0posix-14.0.0-ucrt-r2
- ZIP x86_64 posix/seh GCC 16.2.0, MinGW-w64 UCRT 14.0.0: SHA-256 d5dbafc4a170e762ca6143151ec918fb9e2c72736fb14cd704abebc6bdd5276a, verificado contra o digest do asset GitHub.
- GNU Binutils 2.47.20260726. WinLibs fornece dlltool.exe sem o prefixo do alvo; mantém-se a geração da biblioteca de importação diferida com -d e -y.
- Headers Node 22.19.0: SHA-256 183bdc17092336ad21e01a425d238e85db4ee077ae3caa0547ff1fbda07d9bd8, verificado contra SHASUMS256.txt oficial de nodejs.org.
- Receita: código 0; fontes addon.cc, filesystem.cc e process.cc; flags -std=c++17, -O2, -DNAPI_VERSION=8, -D_WIN32_WINNT=0x0A00, -shared, -static, -static-libgcc, -static-libstdc++, -Wl,--no-insert-timestamp; bibliotecas advapi32, bcrypt e ntdll. Os mesmos símbolos napi_* são extraídos das fontes.
- Complemento: native/windows/build/rea-windows-x64.node, 908251 bytes, SHA-256 d9ccef536ab92189002d471b9e845c81799f64fef3dc3badcefae48de0c82fc4.
- Manifesto gerado: packageVersion 6.4.0, win32, x64, ABI 1, Node-API 8, digest dos bytes efetivamente compilados.

A receita foi revista pelo agente rea_review contra o builder upstream; node --check passou, sem bloqueador encontrado. A revisão confirmou fontes, flags, símbolos, importação diferida e campos do manifesto. Registou-se apenas uma melhoria opcional no diagnóstico da ausência do asset WinLibs; o asset selecionado existe e foi verificado. Os executáveis e headers extraídos têm hashes registados nos ficheiros extracted-toolchain-sha256.json e extracted-node-headers-sha256.json; os arquivos de origem foram verificados antes da extração.

## Verificação pública no Windows

O agente principal executou os verificadores públicos no artefacto produzido e preservou os resultados neste diretório:

- node scripts/verify-windows-native-artifact.mjs: passou; confirmou versão, arquitetura PE32+ x64, ABI, Node-API e SHA-256. native-artifact-verification.log.
- node scripts/verify-windows-native.mjs: passou; native-controls-verification.log tem ok:true. Inclui carregamento num executável Node renomeado, DACL, identidade NTFS, rejeição de junctions/streams/handles falsificados, snapshots imutáveis e assíncronos, cancelamento, retenção após falhas de limpeza, processos descendentes, kill-on-owner-close, transporte stdin e limites de memória de Job Object.
- O fixture compilado opcional de testes adicionais de token/breakaway/reparse não foi fornecido nesta execução; esses testes opcionais não estão cobertos pelo resultado.
- CLI pública doctor --provider ghidra: todos os checks do âmbito Ghidra passaram, incluindo native_authority. doctor-native-ready.json. Ghidra 12.1.4, lançador headless, descompilador x64 e JDK 21.0.12.1 foram admitidos.

Não foram iniciadas importações/análises Ghidra nem executado o programa original por este subagente. A análise do original é gerida pelo agente principal.

## Parâmetros de analisadores

REA_GHIDRA_DISABLED_ANALYZERS está implementado nesta fonte. Recebe nomes exatos separados por vírgulas; uma pré-análise verifica a desativação e analyzer_preset regista o parâmetro. Nomes desconhecidos abortam a importação.

O Ghidra 12.1.4 identifica Scalar Operand References em Ghidra/Features/FunctionID/ghidra_scripts/FunctionIDHeadlessPrescript.java. A documentação REA identifica WindowsResourceReference para recursos PE. O agente principal selecionou Scalar Operand References,WindowsResourceReference para a nova importação. A desativação reduz a cobertura dos factos produzidos por estes analisadores; os testes de configuração passaram, mas o sucesso da importação do alvo deve ser confirmado na investigação principal.

## Comparação com npm 6.4.0

O complemento npm do commit a56f7bb668ab23fec8d33f7af353ac7ed2fe1895 não foi reutilizado: addon.cc, authority.hpp, process.cc, build-windows-native.mjs e WindowsNativeLoader.ts diferem funcionalmente de main. native-source-comparison.json contém hashes, e native-source-differences.patch contém as diferenças. O artefacto atual foi produzido das fontes main, preservando os controlos atuais de memória, processo e limpeza.

experimental-build-provenance.json e windows-hosted-gnu-recipe-result.json guardam a proveniência final, ferramentas, parâmetros e resultados verificáveis. Os logs doctor.json/windows-native-build.log documentam o bloqueio anterior; doctor-native-ready.json e native-controls-verification.log documentam o estado final pronto.
