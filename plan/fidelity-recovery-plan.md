# Recuperação orientada à fidelidade — próximos marcos

Pedido: contornar os obstáculos que deixaram geometria, materiais, distribuição, coreografia e áudio por recuperar. Preservar aplicação C++/bgfx, VR e JSON; instalação original só para leitura; sem Git.

## Estado comprovado

- Inventário PAK: 1696 entradas, zero cifradas e zero comprimidas ao nível do PAK. Isso não exclui compressão interna de texturas/áudio.
- REA/Ghidra/JDK preparados; evidência anterior reutilizável. A sessão terminou, pelo que não existe sessão quente para reutilizar. Evitar repetir diagnósticos e análises globais; agrupar as próximas funções focadas numa única sessão.
- Cabeçalhos inspecionados de malhas têm ue4_version=0: não inferir versão exata a partir do ano de lançamento. Resolver pelo executável/estrutura e documentar qualquer override necessário.
- Compatibilidade de UE Viewer/CUE4Parse com este jogo ainda não testada. Não confundir capacidades declaradas com extração conseguida.

## Marco 1 — prova de extração pequena

- [x] Perfil UE4.13 confirmado por ramo/revisão3172292; instalação4.13.2 correspondente.
- [x] SM_Lower_1 glTF/BIN exportado e verificado:121v,648 índices; UV/normais/tangentes/bounds.
- [x] DiagonalLine32 PNG256×256 RGB descodificado e verificado; material completo pendente.
- [x] MASTER Ogg extraído e totalmente descodificado com DLLUE4:44100Hz/2ch/424.182131519s.
- [ ] Guardar comandos, logs e hashes. Se houver falha, identificar etapa/formato; tentar leitor alternativo CUE4Parse ou analisar só o serializador responsável com REA. Não repetir a mesma falha sem hipótese nova.

Fontes oficiais consultadas: https://www.gildor.org/en/projects/umodel (exportação de malhas/texturas/áudio, limitações de materiais); https://github.com/FabianFG/CUE4Parse (classes nativas e conversão). Sem novas ferramentas adquiridas neste turno.

## Marco 2 — recursos integrados

- [x] Banco selecionado de67StaticMesh exportado e validado; materiais/texturas adicionais ainda pendentes.
- [ ] Integrar geometria e texturas recuperadas e verificar eixos/escala/submeshes/materiais.
- [x] Áudio MASTER integrado na0.4.2; relógio de consumoSDL governa sequência, pausa e seek.
- [ ] Reconstruir funções dos materiais/emissão/transparência/efeitos no renderizador próprio. Não prometer que shaders preparados para UE4 entram diretamente em bgfx.

## Marco 3 — distribuição e coreografia

- [ ] Traçar Layout_Serialized_Forest/Intro/Tunnel e consumidores Scatter antes de reconstruir posições.
- [ ] Recuperar geradores e seleção de fontes; comparar amostras de posições/rotações/escalas/sementes.
- [ ] Completar cadeia BeatDirector → subscritores → ComposerSequencer → presets, com testes de mudanças de índice e relógio.
- [ ] Eliminar transições inventadas apenas quando o comportamento original correspondente estiver demonstrado.

## Marco 4 — comparação de fidelidade

- [ ] Comparar execução integral de 424 segundos com a referência em movimento, no mesmo instante e com pose de câmara comparável.
- [ ] Identificar diferenças por área: formas, distribuição, material, luz, cor, movimento, eventos e áudio.
- [ ] Corrigir divergências e conservar provas. Testes funcionais de VR/JSON são regressões necessárias, não métricas de fidelidade.

## Regra de progresso

O próximo marco é uma malha recuperada, uma textura descodificada e áudio utilizável, com prova de formato/conteúdo. Uma nova versão ou testes do protótipo não substituem esse resultado. Não existe prazo total validado nem prova de fidelidade integral nesta fase.

## Primeiro marco atingido

Extração pequena comprovada com UE4.13/UModel; ver ue413-recovery/findings.md. Produto0.4.1 não alterado. Falta integrar recursos e recuperar distribuição/coreografia/materiais restantes.

## Produto0.4.2

Áudio integrado e pacote verificado: audio-integration/findings.md. Próximo: integrar a geometria recuperada e substituir os materiais/distribuição inventados.

## Referência visual activa

Vídeo fornecido pelo utilizador: Topz0NkGIus; observações/correções em video-reference/findings.md. Menu inicial ignorado; não copiar orientação do HMD gravado para o mundo. Integração deve usar banco visual-resource-bank e correspondência por fase, em vez das formas/padrões inventados da0.4.2.
