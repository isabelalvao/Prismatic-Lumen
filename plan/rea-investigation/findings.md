# Registo de investigação REA

## Identidade e acesso

- REA 6.4.0 executado efetivamente através do seu public launcher, em servidor MCP stdio mantido por um cliente local temporário. A configuração global do Codex não foi modificada.
- open-binary.json confirma Fantasynth-Win64-Shipping.exe nativo PE x86_64 e SHA-256 5380708aaf59992814912122afc3f71d6b18bd41062cde1a5e04fcec4e9f191b. A abertura é seleção/admissão; não equivale ainda a descompilação.
- Ghidra 12.1.4 e Eclipse Temurin JDK 21.0.12.1+1 portáteis, SHA-256 oficial verificado, em tools/vendor. Versões e fontes em toolchain-manifest.json. O ambiente é limitado aos processos de análise; instalação do Fantasynth preservada.

## Questões e evidência adicional local

Os pontos abaixo foram lidos nos JSON existentes; não são resultados novos do REA.

- CameraRig → MovieScene3DTransform → canal Translation[1]: (112,-6400), (224,16000), (288,28800), (~352,48000), (416,60800), (448,67200). Rotação canal [2] default ~90°. Canal Y original corresponde ao eixo de avanço Z da reconstrução.
- Cadeia serializada Camera → Pawn.Scene → Pawn.DefaultSceneRoot → CameraRig.DefaultSceneRoot (map-layouts.json).
- PyroSequencer referencia CameraRig, MinDistance 2500 e DirectorSection; ações índices 4/5/6. PyroTunnelSequencer_150: mesma câmara, distância 6400, índices 10/11/12.
- DirectorSection associa estes índices a 112/144/176 e ~288/320/352 s. A correspondência temporal e semântica do despacho continuam a confirmar.
- SEQUENCEManager → ChezNousProfile → ChezNousSequence + ChezNousSound, duração 424 s. Não prolongar a experiência até448 só por existir uma chave da câmara nesse instante.

## Estado

Primeira análise nativa atingiu900s sem resultado; não se tratou o erro como sucesso nem se apresentou descompilação inexistente. Segunda sessão com4G, prazo1200s e pesquisa focada em curso, conservando artefactos da tentativa anterior.

## Novo achado estático — Blueprint de ocultação

Não é REA Evidence nem observação de execução. Revalidados diretamente no PAK os SHA1 de ActorHiddenSequencer.uasset (090e7ab4eed3ed25d8bd23468da744f5af227150) e BeatActionType.uasset (27347f29e46d07c039537eafccda933afe2465fc). O script ExecuteUbergraph_ActorHiddenSequencer tem750 bytes e SHA256736fe9c9840911c315d9cf9d647fc2e5a8a608f32760262c8038edb06a0e646b.

Chamadas SetActorHiddenInGame nos offsets181/688 do script (PAK123814736/123815243) recebem EX_SwitchValue. As atribuições EX_LetBool em98/105 e640/647 definem local22=true e local15=false; selector21 recebe Action_2 da estrutura de ação. Case0 seleciona local15; case1 seleciona local22. O enum confirma0=Start e1=Stop. Portanto nesta classe **Start mostra e Stop oculta**. Os contextos das chamadas são itens do array e o ator recebido; não foi reconstruído o CFG inteiro nem o comportamento do caso por omissão.

Estrutura dos tokens corroborada com UAssetAPI EX_SwitchValue e documentação Epic EExprToken. Não generalizar esta interpretação a Pyro ou outros sequenciadores sem descodificar os respetivos scripts. Regiões e candidatos de chamadas em blueprint-script-regions.json e blueprint-call-candidates.json; estas extrações foram produzidas pelo parser local e têm essa origem identificada.

## Novo achado estático — relógio de BeatDirector

SHA1 do recurso651f6e33a9d5467359527611e76f2aa1eaab37ef verificado no PAK. ExecuteUbergraph tem952 bytes serializados/1524 em memória, SHA256d43d0a9dec6b0fef7db46670687d8f8f15f1ae769775a74d5fc435384a5a91f4; base do script11860 no recurso/123723401 no PAK.

Algoritmo: T=max(Time+TimeOffset,0); Index=floor(T); Alpha=T-float(floor(T)); Changed=(Alpha!=LastAlpha)||(Index!=LastIndex). ReceiveTick chama o grafo depois de guardar DeltaSeconds. O grafo verifica IsPlaying e Enabled, despacha Update(DeltaSeconds) ANTES de recalcular Index/Alpha e depois avalia o modo.

Modo Alpha0: Beat se Alpha>0 e (LastAlpha<=0 ou Alpha<LastAlpha). Modo Index1: Beat se Index!=LastIndex. No máximo um Beat por tick, sem ciclo de recuperação de índices saltados. Depois de Beat, copia LastAlpha/LastIndex; outros modos terminam antes dessa cópia. O diretor não integra Time+=DeltaSeconds e não tem deteção explícita de direção.

Start define IsPlaying=true, calcula Index/Alpha e despacha Start; Stop define IsPlaying=false e despacha Stop; Reset repõe LastAlpha/LastIndex/Changed/IsPlaying mas não Time/Index/Alpha. As ligações reais dos subscritores e frequência de ticks continuam desconhecidas. Decodificação estrutural é análise local dos recursos, não REA Evidence nem execução observada.

## Diagnóstico da análise nativa

A stack do próprio JVM, obtida com jcmd e guardada em jvm-threads-4g.txt, mostra a análise automática em ScalarOperandAnalyzer. A opção de desativar analyzers existe na documentação/source main do REA, mas não no npm6.4.0 instalado. O heap4G foi observado na linha de comandos do JVM; não se assume que a opção ausente foi aplicada. Preparar um build oficial main com a capacidade necessária, preservando versão publicada e fonte/commit do build.

## Evidência nativa obtida — resultado atual

REA main df2c08b executado pelo launcher oficial, fonte preservada. Perfil7168c70d6fdd7c9a8a4f1f27eff4d611637d12bbcb5fec2a765beabd1458a7c2 desativa oito analyzers globais; por isso silêncio de referências não é ausência de comportamento. Os nomes candidate são sementes do analista. Dossiers completos em inline-evidence-index.json.

- Enum Linear0/Constant1/Cubic2: ev_5ceb47aed83db8153b4fc920b6f3172d724b328396c6c152b6ba1c2e0ace1fe7.
- Serializer RichCurveKey27bytes: ev_813ddb47fc69ac0628697738fd5dd08d0c0c8679ce925dbef70ef68fc8522961; a semântica do arquivo/version/auxiliar é inferida do padrão e estrutura.
- GetFloatValue wrapper→UCurveFloat→eval140a90bf0: ev_0180ea8dc43beaea1cf92dc4530ca11c1bbc886be4c66da3f5472bf009a92010, ev_43bd3e8d4e8ad416e0bfa7167b18c374d86c72f5d945b78fa96b4d19e0a431ab, ev_3ddd51e30b3f5bb36a717972b370c396437667660693d90dd5e9a991db40e15c. Modo esquerdo controla o segmento; tangentes entram no Bézier; este corpo não consulta pesos.
- Remap140aa26a0: ev_fd2f7b195f795aa5224d0ac7a6421c864b120397a9f858d08c7089ef9f8873cf. Modos3linear/4constante fora dos limites; ciclos/offset/oscillate não são necessários aos canais selecionados.
- Registo LevelSequence correto: ev_be69448288e1da7351fef7fd28a14e501f0c3d36715d2eddfd87c359e28ff215. Wrapper ev_aaac33b186a2d0e9022144d37942af9eba8b8b07fef8a570a591640f0caf1b18; funçãoev_ff3410721b80b2d5f19774bfc44deed586e9238ce6d964411a7bc4def8325e30 ainda não prova todo o mecanismo de reprodução.

Construtor leaf140356d60 foi recusado poranalyze_function (native-richcurve-constructor.json); leitura local bounded em native-interpretation/richcurve-constructor-local.json observa word0404 nos offsets58/59. Herança destes defaults pelos canais sem override é inferência estática qualificada.54chaves Camera/Fog foram novamente verificadas contra o PAK pelo agente, em channels-camera-fog.json. Não se executou o original.
