# Reconstrução visual da experiência Fantasynth One

Pedido explícito: avançar para fidelidade completa, sem devolver apenas mais uma cena isolada. A versão 0.3.0 foi testada pelo utilizador, que confirmou que estamos no caminho certo. O objetivo continua a ser uma aplicação C++ própria, em VR Quest 3/Virtual Desktop, controlável por parâmetros JSON de um LLM.

## Critério de conclusão

Comparar todas as fases visuais identificadas na referência de 2017: campo noturno com formas curvas luminosas, paisagem colorida com cones/volumes segmentados e elementos suspensos, túnel, atmosferas/partículas/reflexos e a sequência das transições. Renderizadores/meshes/shaders próprios; conservar VR e protocolo. A cobertura da referência e as diferenças devem ser auditadas explicitamente. Não declarar cópia integral ou exatidão musical a partir de screenshots ou metadados apenas.

## Fases

- [ ] Recuperar dados estáticos adicionais de mapa, distribuição, geometria, curvas e sequência; procurar referência completa oficial, mantendo o alvo Fantasynth One de 2017.
- [ ] Substituir o campo genérico por estruturas curvas/fan-shaped e terreno/reflexos apropriados.
- [ ] Reconstruir a paisagem colorida com volumes segmentados, placas/aros, postes/luzes e céu/névoa coloridos.
- [ ] Rever o túnel, profundidade, variedade e efeitos segundo dados adicionais.
- [ ] Integrar sequência contínua, transições e controlo; separar tempos recuperados dos tempos de reconstrução quando houver lacunas.
- [ ] Capturar e comparar cada fase, verificar regressões desktop/VR/JSON e estabilidade.
- [ ] Revisão independente, pacote completo e auditoria de fidelidade; continuar nas lacunas, sem marcar fidelidade completa sem evidência.

## Questões abertas

1. Há dados de coreografia serializados que revelem tempos e sequência em segundos?
2. Podemos observar a referência completa de sete minutos, além do trailer de 40 s?
3. Quais os limites/dimensões/referências efetivos das formas curvas e dos volumes segmentados?
4. Como preservar o controlo JSON durante uma sequência automática sem sobrescrever comandos recebidos?

## Estado

Versão 0.4.0 implementada: campo noturno de leques curvos, paisagem de cones segmentados, elementos suspensos, reflexos e sequência de 424 s com transições. 6 CTest e 23 testes Python aprovados; pacote portátil extraído e verificado em desktop/estéreo simulado/JSON. Referência integral, terreno completo e coreografia exata continuam pendentes; as fases acima são critérios de fidelidade, não apenas de presença funcional. Detalhes em 2026-10-12-sequence-development.md e full-fidelity-audit.md. Não inicializar Git. ZIPs 0.2.0 e 0.3.0 conservados.

## Desenvolvimento com REA —0.4.1

Curvas de névoa e percurso da câmara corrigidos com evidência nativa;22 dossiers/leituras completos indexados. Sequência, seek, pausa e JSON verificados. Detalhes em rea-investigation/curve-development.md. Esta etapa fecha uma aproximação temporal, não a fidelidade integral; geometria, coreografia, áudio e validação física continuam pendentes.

## Desenvolvimento0.4.2 — reprodução original

MASTER recuperado integrado, sequência guiada pelo consumo de amostras, pausa/seek/reinício, JSON mantido.7CTest/30Python, saída WindowsWASAPI e pacote extraído desktop/VRsimulado aprovados. Áudio resolvido quanto à reprodução; análise de batidas/coreografia e recursos visuais completos continuam pendentes. Relatório em audio-integration/findings.md.
