# Desenvolvimento 0.4.0 — composição contínua

Pedido: retomar o desenvolvimento da reconstrução completa. O desenho C++/VR e o plano full-fidelity existentes definem o âmbito autorizado.

## Decisões

- Manter o túnel e o campo antigo selecionáveis; acrescentar Night e Landscape com malhas próprias.
- Sequência visual de 424 s, com entradas de Forest aos 80 s e Tunnel aos 224 s, inferidas de DirectorSection e ActorHiddenSequencer. A sobreposição termina aos 112 s; larguras de transição são escolhas próprias.
- Pré-calcular distribuição reproduzível, agrupar por família e renderizar somente cenas com peso positivo.
- Separar composição/atmosfera dos parâmetros do SceneController; a sequência não submete comandos nem modifica revisões, destinos ou interpolações JSON.
- Pausa congela reprodução/avanço em VR; a rede e as transições de parâmetros continuam. Seleção manual sai da sequência; Q reinicia-a.
- Expor seleção de cena e tempo inicial/fixo para verificar fases e fronteiras sem esperar sete minutos.
- Não inicializar Git; conservar pacotes anteriores. Guardar cópia dos ficheiros alterados em plan/backups/sequence-0.3.0.

## Progresso

- [x] Ler implementação e evidência, observar cinco capturas oficiais e fazer cópias.
- [x] Testes de geometria, sequência, aplicação e regressões JSON/VR (6 CTest e 23 Python).
- [x] Malhas curvas, cones segmentados, terreno, postes, placas e suspensão.
- [x] Integração de composição, atmosfera e sequência.
- [x] Compilação, cinco capturas e comparação visual; medições locais de cinco segundos/cenário.
- [x] Revisão independente, pacote 0.4.0 e auditoria; objetivo de fidelidade integral continua aberto.

## Limites

Não existe observação integral do original neste trabalho. Áudio não está implementado; pulsação continua sintética. Malhas, texturas, coreografia de instâncias e shaders próprios não são equivalentes aos recursos cooked. Os testes do runtime simulado não substituem avaliação no Quest 3.

## Verificação e ocorrências

- sequence-tests-red.log: novos testes falham por ausência das novas funções; sequence-app-red.log: aplicação rejeita novas cenas/argumentos.
- sequence-core-green.log: núcleo aprovado; sequence-build-test.log: Release, 6 CTest e 23 testes Python aprovados.
- CMake não está no PATH; tools/build.cmd encontra-o nas Build Tools existentes.
- Primeira submissão da edição rejeitada por duplicação de um destino no patch; corrigida antes de aplicar.
- Revisão independente: corrigidas oclusão do chão no fim da transição, seleção ativa no relatório e normais dos leques.
- Comparação pixel a pixel rigorosa falhava devido a variações subpixel entre composições distintas. Métrica de continuidade substituiu-a apenas na fronteira; falha confirmada com chão opaco e aprovação com transparência restaurada.
- sequence-final-test.log: 6 CTest e 23 Python aprovados; sequence-boundary-green.log aprova recursos finais restaurados.
- sequence-package-check.json: pacote 0.4.0 extraído, hashes/desktop/estéreo/JSON/transição aprovados; runtime simulado excluído.

## Estado final desta etapa

Entrega 0.4.0 pronta para avaliação no Quest. Consultar full-fidelity-audit.md; fidelidade integral, áudio e integração do LLM continuam pendentes. Sem alterações ao contrato JSON, sem inicialização de Git. Cópias e versões anteriores preservadas; pastas temporárias de testes/empacotamento eliminadas automaticamente.
