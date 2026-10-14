# Investigação com REA — Fantasynth One

Pedido explícito: usar REA para orientar a reconstrução integral, mantendo o objetivo anterior de desenvolvimento.

Alvo: C:/Program Files (x86)/Steam/steamapps/common/Fantasynth/Fantasynth/Binaries/Win64/Fantasynth-Win64-Shipping.exe e recursos da mesma instalação.

## Perguntas

1. Como os diretores acionam os sequenciadores e a composição de instâncias?
2. Como a câmara percorre a experiência e como se ligam os tempos recuperados à execução?
3. Quais os mecanismos de padrões, materiais, distribuição e transições ainda ausentes da reconstrução?

## Plano

- [x] Identificar acesso CLI/servidor, motores disponíveis e identidade do alvo.
- [x] Abrir o alvo com REA; recolher evidência orientada às perguntas e guardar resultados completos.
- [ ] Correlacionar achados com recursos UE4 já recuperados, aplicar conclusões úteis à reconstrução e verificar.
- [ ] Atualizar ledger com observações/inferências/lacunas e apresentar resultados sem confundir inventário com descompilação.

## Restrições

Não modificar a instalação original. Sem inicialização de Git. Reutilizar ferramentas existentes antes de adquirir dependências; ferramentas temporárias em temp, evidência durável em plan/rea-investigation. Manter artefactos originais e identificadores de evidência REA.

## Estado

REA não exposto por MCP nesta sessão e comando rea ausente do PATH. Verificar invocação CLI oficial através de npx; não confundir esta ausência com impossibilidade de análise.

## Ocorrências

- REA 6.4.0 CLI funcional. Abertura original com Ghidra falhou por motor ausente e JRE 8 incompatível; diagnóstico completo em native-open-initial.json.
- API Adoptium respondeu HTTP 403; passar para releases oficiais do mesmo projeto no GitHub. As tentativas de aquisição dependentes desse manifesto falharam antes de criar/instalar ficheiros.
- Preparar runtime portátil Ghidra 12.1.4/JDK 21 em tools/vendor; downloads temporários em temp, SHA-256 oficial verificado. Ambiente apenas nos processos de análise, sem setup global.
- Runtime preparado e hashes verificados. Servidor REA 6.4.0 inicializado e open_binary ligou o alvo x86_64/SHA-256 5380708aaf59992814912122afc3f71d6b18bd41062cde1a5e04fcec4e9f191b. Importação/análise Ghidra em curso.
- Auxiliar MCP temporário revisto independentemente. Pedidos são publicados por .tmp/rename atómico; não reiniciar a mesma fila nem sobrescrever prova. Ajustados no ficheiro validação prévia do destino e prazo absoluto; a sessão atual já carregada usa exclusivamente pedidos internos válidos.
- Primeira análise nativa: provider_timeout aos900000 ms com heap2G; binary-overview.json conserva o erro real, não um resultado parcial. Sessão cancelada e respetivo processo analisador limpo.
- Segunda sessão independente com heap4G e prazo1200000ms, pesquisa inicial mais focada. Não sobrescreve evidência anterior. A opção de desativação de analyzers consta do ramo main mas não aparece no pacote npm6.4.0; não se assume que foi aplicada. Perfil efetivo deve ser verificado nos resultados.
- Segunda sessão também atingiu provider_timeout aos1200000ms; a ferramenta fechou o alvo e limpou o JVM. native-seeds-4g.json e server-stderr-4g.log conservam os resultados reais. Nenhuma descompilação foi apresentada como concluída.
- Source build oficial main df2c08b546d4f40103e11b8b31dbfc3d41ee687b: npm ci/build e10 testes de override aprovados; bloqueio do complemento nativo Windows. Fontes do complemento diferem da release; não copiar binário incompatível. Avaliar build experimental Windows-hosted GNU equivalente ao recipe oficial, sem alterar source/controles; só usar após verificação real.
- Complemento compilado das mesmas fontes/flags através de receita separada GNU no Windows; source upstream preservada. SHA-256 d9ccef536ab92189002d471b9e845c81799f64fef3dc3badcefae48de0c82fc4. Verificador oficial de artefacto e verificador real de controlos Windows aprovados; doctor-native-ready.json confirma disponibilidade. Não se declara equivalência ao percurso oficial de compilação Linux.
- Retomada após limite dos agentes: corrigida falsa terminação do decoder quando0x53 é byte de operando. Verificação local2recursos/8funções e21 casos negativos aprovada.
- Importação geral com source main também atingiu1200s; stack confirma custo em Disassemble Entry Points/Reference. Preservado native-seeds-main.json. Passar a quatro entradas prioritárias verificadas por .pdata/UNWIND_INFO/Capstone, com cadeias de unwind resolvidas e perfil restrito explícito, em targeted-native/priority-seeds.tsv. Desativar sweeps gerais; não afirmar cobertura integral nem nomes C++ originais a partir dos nomes candidatos.
- Perfil de quatro entradas avançou mais rapidamente, mas diagnóstico identificou Decompiler Switch Analysis a descompilar globalmente as funções do motor. Sessão cancelada para não repetir a espera; jvm-threads-targeted.txt conserva diagnóstico. Perfil minimal desativa também esse sweep e descoberta global de no-return, mantendo descompilação explícita por função. A cobertura reduzida fica no perfil/evidência; não se declara análise integral.

## Retoma — curvas e desenvolvimento0.4.1

Perfil minimal terminou em cerca620s;22 registos completos de evidência nativa guardados, sem alegar análise integral. Serialização de27bytes, enum de modos, eval e remap recuperados. Registo correto LevelSequence confirmado; primeiro candidato era Paper2D e outro TimelineComponent.

Construtor RichCurve140356d60 não existe na tabela de funções do perfil Ghidra; analyze_function devolveu invalid_request. Auxiliar encerrou sessão pelo fluxo finally(close_binary) e terminou; toda a evidência inline permanece. Prova do construtor obtida por leitura local limitada e identificada como tal, sem atribuição aoREA. Sem exportação oficial de bundle nesta sessão; inline-evidence-index.json é apenas índice local.

Implementação0.4.1: canais exatos gerados da timeline, modos nativos de névoa, percurso de câmara no seek/default, modulação incremental porJSON, relógio da sequência comum à animação. Sem afirmar equivalência de renderizador/meshes/distribuição/áudio.

- [x] Correção delimitada de curvas/câmara0.4.1 implementada, revista e verificada; pacote extraído aprovado.
- [x] Ledger atualizado com evidência real, correções de candidatos e limites.
- [ ] Fidelidade integral: geometria/distribuição/materiais/áudio e referência em movimento continuam necessários.
