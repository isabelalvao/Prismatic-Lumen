# Evidência, limites e revisões

Data: 2026-10-14 · Revisão 2 · Raiz: `C:/ORCA/Fantasynt` · Git: `bc86e22` antes desta revisão · Motor: 0.12.0.

## Como ler a evidência

**E:** código/configuração/recurso inspecionado nesta tarefa; não equivale a teste de execução ou medição no Quest. **P:** funcionalidade especificada para implementar. **H:** indício/efeito percetivo por testar. **A:** decisão experimental por fixar. A inspeção começou pelos quatro documentos prévios, README e `visual-config.json`; alargou-se apenas às interfaces indicadas e aos shaders/renderizador necessários para seguir os parâmetros até à aplicação.

Nome atual no README: Prismatic Lumen; Fantasynt permanece nos caminhos/executáveis. Finalidade confirmada: investigar agência atribuída ao ambiente e autoria ambígua num espaço não figurativo; cabeça e mãos, sem controladores, voz opcional futura e fisiologia posterior, sem conversa necessária. Jev substituível; nenhuma interpretação emocional é facto extraído dos movimentos.

## Duração e organização atuais E

| Fonte verificada | Resultado e limite |
|---|---|
| [visual-config.json](../../visual-config.json), `timeline.opening_duration_seconds` | 424.1821315192744 s; `startup.scene=opening`, `auto_play=true`. |
| Cabeçalho de [ChezNous_MASTER.wav](../../assets/audio/ChezNous_MASTER.wav), lido com biblioteca `wave` | 18 706 432 frames, 44 100 Hz, estéreo PCM16; duração 424.1821315192744 s. O valor coincide com a configuração; não foi feita audição do final nesta revisão. |
| [main.cpp](../../src/app/main.cpp), `playback_end` | Mínimo entre duração analisada do áudio e duração configurada para Opening; `sequence` usa `sequence_duration`. O relatório `ready` publica o fim selecionado. |
| [sequence.hpp](../../src/render/sequence.hpp) | `sequence_duration=424.1821315192744`; `opening_duration=120.0` é constante histórica. Não concluir que Opening atual dura 120 s. |
| [art_direction.cpp](../../src/render/art_direction.cpp), `directed_composition` | Opening recebe fim efetivo de `main.cpp`; fade visual de `final_fade_seconds=16`: 408.182–424.182 s. Mantém composição dirigida, não o handoff histórico de `sample_sequence`. |
| [player.cpp](../../src/audio/player.cpp), `position`/construtor | Fila limitada ao fim escolhido; posição baseada em amostras consumidas SDL. Não inclui latência física do dispositivo/PCVR e não significa chegada ao ouvido. Sem novo fade sonoro implementado. |
| [Fantasynt.cmd](../../Fantasynt.cmd) | Passa explicitamente `--config` para o JSON da raiz. Executável direto sem esse argumento procura JSON junto do binário. Congelar/hash do ficheiro realmente carregado, não presumir que toda execução usa a cópia da raiz. |

Organização dirigida com a configuração da raiz:

| Tempo E | Acontecimento e fonte |
|---|---|
| 0 s | Câmara/áudio começam automaticamente; velocidade base 1,5 m/s, sem atraso/aceleração; `motion_speed=0.3` conserva percurso. Movimento físico do headset acresce à vista. |
| 0–40/45/60 s | Crescimento de pontos/céu/presença respetivamente; pontos e tracejados em movimento contínuo. |
| 80–112 s | Fim Dome 80; Forest 80 e Ground 112; Intro termina 112. Pirâmides têm início/revelação 80/8, mas triângulos removidos por predefinição; nomes de tempos não asseguram objetos visíveis. |
| 104–116 s | Revelação de padrões sem material emissivo (`unlit_reveal_*`). |
| 136–148 s | Formas/cones e transição cromática quente; construção das peças pode continuar e alinha entradas aos graves. Não é uma troca instantânea de toda a cena. |
| 172–184 s | Módulos e transição fria; revelações das peças dependem dos atrasos configurados. |
| 208–224 s | Saída dos elementos anteriores; redução de exposição a partir de 208 durante 12 s até mínimo 0,25. |
| 224–272 s | Formação de túnel em 48 s; recuperação energética a partir de 224 durante 8 s. |
| 290–306 s | Partida/dispersão e alinhamento/crescimento dos anéis; glow rosa entra aproximadamente 294–306 s pela regra de prontidão. |
| Desde 306 s | Ataques graves alternam grupo par ciano/ímpar rosa; cor e ganho próprios. |
| 408.182–424.182 s | Fade de toda a imagem até preto; fim de áudio sem modulação semântica. |

Estas relações foram seguidas em `directed_composition`, `directed_visual_parameters`, `directed_group_visible`, `surface_choreography` e passagem de uniforms em `renderer.cpp`; não resultam apenas dos nomes de campos do JSON. `--scene sequence` tem organização histórica distinta, incluindo handoff 120–128 s; o piloto especifica Opening.

## Parâmetros aceites versus efeito por fase E

O protocolo [protocol.cpp](../../src/control/protocol.cpp) só aceita `apply_parameters`, schema 1, sete controlos, transição 0,1–15 s; limites 0–1 nos escalares, duas cores. Mensagem até 16 KiB. [scene_controller.cpp](../../src/control/scene_controller.cpp) interpola a partir de estado corrente e preserva destinos omitidos. Confirmação `accepted` em `main.cpp` antecede conclusão e não prova efeito visual.

| Controlo v1 | Primeira fase Opening | Formas/extensão dirigida | Anéis finais | Consequência para P |
|---|---|---|---|---|
| `light_intensity` | Multiplica emissão/corpo em `fs_opening`; pontos/reflexos/fog não respondem como uma luminância uniforme. | Multiplica luz em `fs_scene` e céu/fundo no composite. | Continua global, incluindo glow; tone mapping/fade alteram resultado. | Atuador mínimo candidato em todas as fases, mas efeito físico/percetivo precisa de prova por fase. |
| `palette` | Coloração parcial: tint mix e profundidade; pontos mantêm cor de família, não seguem paleta. | `directed_visual_parameters` mistura com warm/cool; amount chega a 1 após 148 s, substituindo a paleta recebida nessa passagem. Cones/cristais têm cores próprias `u_spectral`. | Anéis pares/ímpares têm cores próprias; glow tem configuração específica. | Não usar paleta de rede como canal relacional geral. |
| `pulse_strength` | Escala envolvente grave em `fs_opening`, com cutoff/idle e famílias diferentes. Sem onset não cria uma batida. | Escala termo de beat comum, não necessariamente todas as máscaras/cores próprias. | Branch de anéis usa `u_choreography`/`u_spectral` e faz `beat=0`; alternância principal não é graduada por este controlo. | Não prometer efeito equivalente na pulsação final. Usar emissão para motivo lento. |
| `geometry_density` | Shader de vértice exclui instâncias por rank: limiar `clamp(density*2,0,1)`, saturado a partir de 0,5. CPU ainda aplica presença/família/culling próprios. | Densidade e opacity por rank, multiplicadores artísticos/artefactos e revelação por peças. | Rank/densidade e coreografia de peças continuam; não interface de regiões. | Mesmo valor tem efeitos diferentes; congelar no piloto mínimo. |
| `fog_density` | Exponencial de distância com coeficientes tuning e atmosfera artística. | Multiplicadores de fase vermelha; não equivalente à primeira fase. | Neblina/glow e distâncias diferentes; canal global. | Congelar; alterações podem mascarar forma/luz de modo desigual. |
| `bloom` | Ganho no composite multiplicado por `post.bloom_multiplier`. | Mesmo parâmetro de pós-processamento sobre conteúdo diferente. | Mesmo caminho, conteúdo energético distinto. | Aceite globalmente; igualdade numérica não garante igualdade percetiva. Congelar. |
| `motion_speed` | Modula deslocação virtual com relação a 0,3, não a orientação física. | Afeta percurso e cálculos dependentes de posição. | Também participa no alinhamento; não regula simplesmente todas as animações/pulsos. | Congelar a 0,3; não usar como descritor corporal. |

Fontes adicionais estritamente necessárias: [renderer.cpp](../../src/render/renderer.cpp), `scene_uniforms`, `draw_opening`, `surface_opacity`, [vs_opening.sc](../../shaders/vs_opening.sc), [fs_opening.sc](../../shaders/fs_opening.sc), [fs_scene.sc](../../shaders/fs_scene.sc) e [fs_composite.sc](../../shaders/fs_composite.sc). Não se executaram comandos/capturas para medir variações nesta tarefa. A matriz é evidência estática e guia do ensaio P0/P1, não um catálogo experimentalmente validado.

## Aquisição, áudio e registo E/P

| Evidência E | Capacidade proposta P / limite |
|---|---|
| [openxr_session.cpp](../../src/vr/openxr_session.cpp) enumera extensões mas só habilita `XR_KHR_D3D11_enable`; cria espaço LOCAL. | Habilitar hand tracking suportado e consultar capacidade do sistema; disponibilidade real depende do runtime/streaming Quest. Não foi verificada nesta instalação. |
| `xrLocateViews` usa `predictedDisplayTime`; exige position/orientation VALID. Calcula cabeça média dos olhos e expõe `EyeView::head_world`, sem observação dedicada com tempos/flags. | Cabeça renderizada existe; observação analítica normalizada/seguimento de mãos não existe. Exportar dados físicos, flags VALID/TRACKED separadas e relação temporal. |
| Poll trata estados básicos de sessão e perda; não tem tratamento explícito de mudança de referência nem registo de epoch. | Recentragem, validade/suspensão e pausa coordenada precisam de implementação. A música não fica automaticamente sincronizada com ausência de frame XR. |
| [player.hpp](../../src/audio/player.hpp) tem pause/seek/position/duration sobre um WAV; nenhuma interface de ganho/camadas. | Ganho global suave é novo trabalho. Não há controlo semântico de fontes, timbre ou espacialização. Estéreo do ficheiro não equivale a fontes posicionadas no mundo. |
| [beat_envelope.hpp](../../src/audio/beat_envelope.hpp) deriva envolvente/onsets do WAV. | Contexto musical original disponível; saída depois do ganho deve ser registada separadamente. Não é captura de voz. |
| `SceneController::advance(dt)` corre durante pausa; rede pode submeter comandos; `events.flush()` no ciclo principal. | Executor semântico e writer assíncrono não existem; pausa/reinício comum e filas limitadas são requisitos novos. |
| Teclado Space/Q/Esc e recarga F5/releitura automática existem; dependem de foco. | Modo estudo, vista neutra e interrupção supervisionada independente de foco/rede precisam de validação/alteração. Não há controlo Quest mapeado. |
| [CMakeLists.txt](../../CMakeLists.txt) declara 0.12.0 e vários testes de núcleo/áudio/render/XR simulado. Dependências fixadas por commits em `cmake/`. | Ensaios futuros reutilizam fronteiras; esta revisão não recompilou nem declarou testes de execução passados. |

## Fontes primárias consultadas e alcance

- [Jev / primitivas TypeSafe](https://docs.typesafe.ai/primitives), consultado em 2026-10-14: Choice/Score/Noul, respostas independentes e campos de distribuição/confiança. Justifica validação local da combinação; não demonstra qualidade neste domínio. Não foi medida latência ou consistência Jev.
- [XR_EXT_hand_tracking, Khronos](https://registry.khronos.org/OpenXR/specs/1.1/man/html/XR_EXT_hand_tracking.html), consultado: aquisição de articulações por extensão; existência na especificação não confirma disponibilidade na instalação. [Flags de velocidade das articulações](https://registry.khronos.org/OpenXR/specs/1.1/man/html/XrHandJointVelocityEXT.html) conservadas como referência técnica da revisão anterior.
- [SDL_SetAudioStreamGain](https://wiki.libsdl.org/SDL3/SDL_SetAudioStreamGain), consultado: ganho por stream, disponível desde SDL 3.2.0, aplicado na obtenção dos dados e com mutex próprio. Verificar suporte na dependência fixada e qualidade/cadência do ganho; API não garante suavidade nem sincronismo físico.
- Referências anteriores conservadas: [estado Jev](https://docs.typesafe.ai/concepts/state), [limitações 1.13](https://docs.typesafe.ai/model-jaggedness/jev-1.13), [confiança](https://docs.typesafe.ai/confidence) e [código VDXR](https://github.com/mbucchia/VirtualDesktop-OpenXR/blob/main/virtualdesktop-openxr/instance.cpp). Não se mediu fornecedor/runtime com base nesses links nesta revisão.
- [Gao e Scholl, 2011](https://perception.yale.edu/papers/11-Gao-Scholl-JEPHPP.pdf) e [Dewey e Knoblich, 2014](https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0110118) lidos como fontes empíricas primárias; resultados/limites de transferência constam do [protocolo](first-exposure-protocol.md#fundamentação-primária-e-limites-de-transferência). Não validam tempos, itens ou amostra do piloto.

## Memória de projeto e diário de revisão

Verificadas as competências `planning-with-files`, `obsidian-project-bootstrap` e `obsidian-project-memory`. Nova deteção `project_kb.py detect` continua a dar candidato de investigação não registado. Não existe `.Codex/project-memory/registry.yaml`, nem caminho explícito de vault ou variável `OBSIDIAN_VAULT_PATH`. O script instalado continua a procurar `.claude/project-memory/registry.yaml`, divergindo da regra `.Codex/`. Não criar vault/ligação arbitrários nem alterar o script nesta tarefa. Não é uma aprovação pendente: documentação local completa, sincronização de Obsidian sem destino configurado.

Diário de 2026-10-14: verificada duração por frames do WAV/config/código; traçados atuadores até shaders; redesenhada sessão autossuficiente e três percursos; memória de motivos explicitada; comparação entre pessoas T/O/R e refutação definidas; marcos técnicos priorizados. Não houve recolha subjetiva, edição de código/configuração ou execução no headset. Estado/progresso durável em [task_plan.md](task_plan.md).

## Registo de alterações da revisão 2

| Decisão anterior | Revisão/justificação |
|---|---|
| Duração e instrumentos ainda genéricos | Duração E 424,182 s; episódios P distribuídos fora de travessias; instrumentos finais continuam A. |
| Três comportamentos sem fichas situadas | Mantidos, com memória transversal, ganho/emissão mínimos e contingência distinta de retorno autónomo. |
| Jev/expansão antes do piloto | Regras locais bastam; Jev em observação e propagação/timbre ficam fora do caminho mínimo para evitar fatores simultâneos. |
| Contrabalançar ordem de sessões | Removido para este estudo: uma condição por pessoa, recorrências internas e comparação entre participantes. |
| «Não há Git na raiz» | Informação histórica ultrapassada: Git confirmado agora, HEAD `bc86e22`. Cópia documental também preservada. |
| Prompt de próximo desenho | Executado por esta revisão; próximo passo passa a P0, programação fora do âmbito e sem implementação nesta tarefa. |

Os ficheiros anteriores estão em `backups/2026-10-14-first-session/`, identificados como arquivo. Os documentos ativos são os da raiz desta pasta; não devem ser combinados com os marcos antigos para criar um segundo plano concorrente.

## Diário posterior — implementação P0 autorizada

2026-10-15 — P1 explicitamente autorizado: implementado executor determinístico de diminuição longa/retornos aos 50/66/152/188/202/316/334/370 s, composição após direção artística, ganho PCM por amostra na cópia do MASTER em memória, pausa/retoma com epoch, reinício/cancelamentos e registo base/contribuição/efetivo. Sem decisões corporais/limiares/Jev/LLM. Configuração congelada em P1; opção desligada preserva ganho 1 e comportamento anterior. Contribuição termina aos 382 s (limite 392), fade soberano. [Implementação, comando VR e verificação](../p1-motifs-2026-10-15/README.md). Família dupla/par desigual e contingência continuam posteriores; não declarar qualidade audível/alinhamento físico antes do ensaio Quest. Cópias delimitadas preservam o estado anterior; sem commit/push.

Retorno após implementação das mãos, 2026-10-15: o utilizador reporta «funciona bem» e pergunta pelo próximo passo. Registado como confirmação funcional reportada, sem novas medições/JSONL. Próxima implementação proposta é P1 (executor local de motivos globais, começando por curvas programadas verificáveis); antes de ligar decisões ao corpo, realizar ensaio técnico com mãos visíveis para observar repouso/movimento/perdas e afinar limiares com referência física. A pergunta não autoriza por si só implementar adaptação.

Atualização de 2026-10-15 — mãos visíveis pedidas explicitamente: implementados esferas e segmentos 3D a partir da amostra prevista OpenXR por frame. Mesma origem física/escala dos olhos, sem avanço virtual; apenas posições válidas e seguidas, desaparecimento imediato das partes perdidas e recuperação sem retenção. Pausa audiovisual conserva o movimento. Compilação Release, 19 CTest, um teste gráfico separado e 75 testes Python aprovados; capturas sintéticas revistas. Evidência, hashes, cópias anteriores e protocolo no Quest em [vr-hands-2026-10-15/README.md](../vr-hands-2026-10-15/README.md). O teste de integração voltou a gerar `plan/m1-integration.png`; o seu destino futuro passou para `temp/`. Configuração da raiz, Opening, MASTER, percurso, aquisição e descritores preservados. Adaptação e afinação de limiares continuam pendentes de validação física/pedido próprio; os pacotes P0 anteriores conservam-se.

2026-10-14: implementadas aquisição em `src/vr/tracking.*`, observações/derivadas em `src/interaction/`, gravação SPSC e integração `--record-session` no arranque. Alterações anteriores/documentos preservados; backup técnico em `plan/hand-observation-p0/backup/`. Adaptação continua desligada/inexistente. Testes/revisão e limite hardware em [hand-observation-p0/README.md](../hand-observation-p0/README.md). Assim, as tabelas E acima descrevem a base inspecionada **antes** de P0; a ausência de aquisição de mãos nessa base não descreve a nova compilação de desenvolvimento.

Obsidian continua sem vault/ligação configurada; o diário e o estado durável permanecem nos documentos locais. P0 não se declara completamente aceite sem Quest real e medições de cobertura/desempenho.

Pedido posterior de 2026-10-14: criado pacote portátil P0 para teste noutro PC, com lançadores PowerShell5.1, verificação básica das mãos e ZIP próprio. Validado fora da árvore de build, com hashes e cenários normal/sem mãos/falha XR; [evidência e execução](../hand-observation-p0/README.md). Os ZIP anteriores permanecem preservados.

Diário de inspeção, 2026-10-14: o utilizador reportou teste real em VirtualDesktopXR com aquisição inicial válida das duas mãos e encerramento sem perdas de escrita. Hash do binário coincide; bc86e22 é a base Git com P0 local não comprometido. Corrigida a ausência de ferramentas no dist: inspetor com cobertura/perdas/reconstituição, atualização pequena e pacote r2, sem alterar o motor. Testes e overlay aprovados. O anexo contém apenas o relato; análise da sessão completa aguarda `hand-tracking-01.jsonl`. Sem conclusões de qualidade física ou continuidade real. Obsidian permanece sem vault configurado; estado durável nos documentos locais.

Diário de migração, 2026-10-14: criados `AGENTS.md`, `MIGRACAO.md` e `plan/migration-2026-10-14/estado.md` para retomar noutro PC com Codex. Registados código local não comprometido, dependências/compilação, recursos LFS, identidade do binário e pendência do JSONL real. Atualizado o próximo passo para não repetir P0. Levar a pasta de trabalho, não depender apenas de clone/chat. Cópias dos documentos anteriores e estado Git preservados em `plan/migration-2026-10-14/`. Só documentação; sem alterações ao motor, commit ou push.

Diário de validação no destino, 2026-10-15: ferramentas instaladas e `tools/build.cmd -Test` concluído com 19/19 CTest e 73/73 Python. As quatro asserções falhadas inicialmente passaram antes de editar código; causa não reproduzida. Detetada camada implícita do Virtual Desktop nos testes simulados; lançadores de observação/VR/Opening passaram a desativá-la apenas no processo filho, com zero camadas confirmado pelo loader. Motor, configuração e runtime real preservados. Cópias e [evidência completa](../migration-2026-10-14/xr-validation-2026-10-15/README.md).

Na mesma validação, o utilizador indicou a pasta portátil em Downloads e foi encontrado o JSONL real completo. Cópia idêntica em `logs/hand-tracking-01.jsonl`, ignorada pelo Git; SHA-256 `12cc88226df36106f28679c189eecc5bbb68d49a3a6c5e8c8d026f5b026bdfa8`. Inspetor: integridade válida, 3 858 observações/211,414 s, sem lacunas de publicação >250 ms; amostras de posição utilizável: cabeça 100%, esquerda 81,57%, direita 60,32%. Perdas/recuperações: 8 esquerda, 38 direita. Picos até 13,175 m/s na direita exigem referência física; não calibrar motivos com estes valores. Utilizador crê ter ocultado mãos/recentrado, mas não confirma; nenhuma perda é atribuída a uma ação concreta. Aceita adiar a afinação até haver mãos 3D visíveis para diagnóstico. Isso não altera o estado atual de P0 nem autoriza por si só nova implementação visual/adaptativa.
