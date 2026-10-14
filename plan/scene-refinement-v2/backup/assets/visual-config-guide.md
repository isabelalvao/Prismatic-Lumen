# Afinação visual sem recompilar

**Versão atual 0.11.0:** ambiente quente com menos vermelho/nevoeiro e reflexos preservados; cones construídos em oito nervuras e nove círculos independentes; triângulos sólidos e plataformas removidos; cenário conservado 60 m atrás com dissolução nos 30 m seguintes. Os anéis alinham-se e crescem entre 290–306 s; os outros artefactos dispersam-se e o fundo rosa acende entre 294–306 s. A secção «Afinação do cenário — 0.11.0», no fim, substitui os tempos e comportamentos das descrições históricas anteriores.

Edite `visual-config.json` junto do executável, guarde em UTF-8 e volte à experiência. O programa verifica alterações a cada 0,25 s. **F5** força a releitura. JSON inválido mantém a última configuração válida; o erro fica em `logs/vr.log` ou `logs/desktop.log` nos lançadores do pacote.

Na pasta do projeto, `Fantasynt.cmd` usa o ficheiro `C:/ORCA/Fantasynt/visual-config.json`. Um executável iniciado diretamente usa o ficheiro junto dele. `--config CAMINHO.json` permite escolher outro. O caminho efetivo aparece no registo de arranque.

O movimento e a presença luminosa começam imediatamente: `camera.delay_seconds=0`, `camera.acceleration_seconds=0`, `startup.auto_play=true`. A presença inicial é reduzida e cresce gradualmente. As formas arredondadas continuam apagadas quando a música não tem um pulso detetado. Os sólidos triangulares estão removidos por predefinição; os cones luminosos começam aos 136 s. Opening dura **360 s**. **C** mostra/oculta o contador `MM:SS.d`, no monitor e nos dois olhos VR, ligado ao tempo da música. A pausa congela-o; Q repõe-o a zero.

## Ajustes mais frequentes

| Pretende alterar | Campo |
| --- | --- |
| Velocidade do percurso | `camera.speed_m_s` |
| Velocidade de todos os movimentos decorativos | `animations.speed_multiplier` |
| Queda dos pontos | `points.fall_speed_m_s` e `sky.fall_speed_cells_s` |
| Movimento dos tracejados | `traces.speed_m_s`, `traces.direction` |
| Entrada e acendimento das pirâmides | `timeline.pyramids_start_seconds`, `timeline.pyramids_reveal_seconds` |
| Rotação das pirâmides | `pyramids.rotation_degrees_s` |
| Força/sensibilidade dos pulsos | `pulse.emission_gain`, `pulse.relative_threshold` |
| Sincronização visual com o som | `pulse.time_offset_seconds` |
| Cor, luz e densidade de uma família | `families.<nome>.colour/emission/density` |
| Brilho, névoa e pós-processamento | `parameters`, `fog`, `post` |
| Corredor e distribuição | `layout` |

As distâncias usam **metros**, velocidades físicas **m/s**, rotação **graus/s** e tempos **segundos**. Os pontos atmosféricos usam células por segundo porque estão numa grelha angular. `traces.direction` aceita `alternating`, `up` ou `down`; uma velocidade negativa inverte o sentido. Cores usam `#RRGGBB`. `true/false` ativam/desativam famílias e efeitos.

`parameters.motion_speed=0.3` usa a velocidade indicada em `camera.speed_m_s`; zero imobiliza o percurso e os outros valores multiplicam-no proporcionalmente. `animations.speed_multiplier` controla movimentos, mantendo os pulsos ligados ao relógio da música. Espaço pausa/retoma a experiência.

Ao mudar velocidades em execução, a posição da câmara e as fases dos movimentos são preservadas. Corredor, setores e escala reconstroem a distribuição; alterar a disposição pode mudar as posições dos objetos de imediato. Alterações no detetor de graves exigem uma breve preparação, com áudio e animação suspensos em conjunto. Reduzir a duração abaixo do tempo atual termina nesse novo limite sem fazer a câmara recuar.

## Opções aplicadas no próximo arranque

`startup.auto_play`, `startup.scene`, `startup.vr_scale`, `startup.window_width/height` e `startup.soundtrack` requerem reiniciar o programa. O registo mantém `restart_required:true` enquanto essas opções diferirem do arranque ativo. Argumentos explícitos de linha de comandos têm prioridade.

`camera.start_z_m` é a posição inicial: **Q** reinicia o percurso com esse valor, sem fechar o programa. O WAV é PCM16 estéreo a 44100 Hz e deve cobrir a duração selecionada; caminhos relativos são resolvidos a partir da pasta do executável.

`visual-config.default.json` conserva os valores iniciais. Guarde uma cópia da afinação antes de a substituir por estes valores. Compilar o projeto conserva o JSON existente junto do executável; o ficheiro de defaults é atualizado separadamente.

Campos omitidos usam os valores iniciais. Nomes desconhecidos, duplicados, tipos errados e valores fora dos limites são rejeitados. JSON não aceita comentários, vírgulas decimais ou vírgula depois do último campo. O ficheiro deve ter até 64 KiB.

## Alcance

O catálogo afina a experiência atual **Opening**, com seis minutos. Os campos `extension` afetam apenas Opening; a sequência de comparação conserva a composição de 424 s. Os parâmetros globais, câmara, pontos atmosféricos, chão e pós-processamento também afetam as cenas anteriores disponíveis para comparação. As malhas e a lógica dos shaders continuam em código; trocar a forma de uma malha ou criar um efeito novo exige desenvolvimento. As instâncias preparadas têm um limite de 65 000 para manter a capacidade de renderização.

## Catálogo completo

| Campo | Inicial | Intervalo/opções | Função |
| --- | --- | --- | --- |
| `startup.auto_play` | true | true/false | Inicia áudio e animação ao abrir; Espaço continua a pausar. |
| `startup.vr_scale` | 1 | 0.4–1 | Escala da resolução OpenXR; requer reiniciar a aplicação. |
| `reload.enabled` | true | true/false | Relê automaticamente o ficheiro ao guardar. |
| `reload.poll_interval_seconds` | 0.25 | 0.05–5 | Intervalo de verificação do ficheiro. |
| `reload.parameter_transition_seconds` | 0.3 | 0.1–15 | Transição de cores e parâmetros globais após guardar. |
| `camera.delay_seconds` | 0 | 0–424 | Espera antes do avanço; zero elimina o atraso. |
| `camera.acceleration_seconds` | 0 | 0–30 | Duração da aceleração; zero aplica logo a velocidade. |
| `camera.speed_m_s` | 1.5 | 0–10 | Velocidade com motion_speed=0.3. |
| `camera.start_z_m` | -64.0000048828125 | -10000–10000 | Posição longitudinal inicial, antes da escala/deslocamento do cenário. |
| `camera.eye_height_m` | 2.6 | 0.1–10 | Altura da vista desktop; a origem VR tem parâmetros próprios. |
| `camera.eye_back_m` | 4.5 | 0–20 | Recuo da vista relativamente ao percurso. |
| `camera.desktop_vertical_fov_degrees` | 72 | 30–130 | Campo de visão vertical do monitor; em VR usa o runtime. |
| `camera.near_plane_m` | 0.1 | 0.01–5 | Plano próximo da projeção. |
| `camera.far_plane_m` | 280 | 10–10000 | Plano distante da projeção. |
| `animations.speed_multiplier` | 1 | 0–10 | Multiplica movimentos decorativos; não altera o ritmo da música. |
| `timeline.opening_duration_seconds` | 360 | 1–424 | Fim da experiência Opening. |
| `timeline.pyramids_start_seconds` | 80 | 0–424 | Início do acendimento das pirâmides. |
| `timeline.pyramids_reveal_seconds` | 8 | 0.05–120 | Tempo para acender todas as linhas. |
| `timeline.forest_start_seconds` | 80 | 0–424 | Entrada das restantes famílias Forest. |
| `timeline.ground_start_seconds` | 112 | 0–424 | Entrada dos grupos de chão. |
| `timeline.intro_end_seconds` | 112 | 0.05–424 | Fim dos grupos Intro. |
| `timeline.dome_end_seconds` | 80 | 0.05–424 | Fim dos grupos Dome. |
| `timeline.stage_fade_seconds` | 4 | 0.01–60 | Escurecimento na saída de Intro/Dome. |
| `timeline.handoff_seconds` | 8 | 0.05–60 | Transição para as cenas anteriores no modo Sequence. |
| `appearance.initial_presence` | 0.15 | 0–1 | Presença das famílias desde o primeiro instante. |
| `appearance.growth_seconds` | 60 | 0.05–424 | Crescimento gradual da presença. |
| `appearance.points_initial_presence` | 0.05 | 0–1 | Presença inicial dos pontos físicos. |
| `appearance.points_final_presence` | 0.7 | 0–1 | Presença final dos pontos físicos. |
| `appearance.points_growth_seconds` | 40 | 0.05–424 | Tempo do crescimento dos pontos físicos. |
| `appearance.sky_initial_brightness` | 0.25 | 0–1 | Brilho inicial dos pontos atmosféricos. |
| `appearance.sky_growth_seconds` | 45 | 0.05–424 | Tempo do crescimento do brilho atmosférico. |
| `layout.corridor_width_m` | 4 | 0.5–30 | Largura livre do percurso. |
| `layout.corridor_margin_m` | 0.2 | 0.01–5 | Margem além da largura livre. |
| `layout.corridor_floor_m` | 0.1 | 0–2 | Limite inferior do volume desimpedido. |
| `layout.corridor_ceiling_m` | 5.5 | 1–20 | Limite superior do volume desimpedido. |
| `layout.intro_sectors` | 5 | 1–8 | Quantidade de setores introdutórios. |
| `layout.intro_sector_spacing_m` | 32 | 1–200 | Distância entre setores introdutórios. |
| `layout.scale` | 1 | 0.1–3 | Escala das formas e da distribuição. |
| `layout.offset_x_m` | 0 | -1000–1000 | Deslocamento do cenário no eixo x. |
| `layout.offset_y_m` | 0 | -1000–1000 | Deslocamento do cenário no eixo y. |
| `layout.offset_z_m` | 0 | -1000–1000 | Deslocamento do cenário no eixo z. |
| `layout.cull_distance_multiplier` | 1 | 0.1–10 | Multiplica a distância de visibilidade das malhas. |
| `points.fall_speed_m_s` | 0.1 | 0–5 | Velocidade de queda dos pontos físicos. |
| `points.cycle_height_m` | 60 | 1–500 | Altura do ciclo vertical. |
| `points.scale` | 0.15 | 0.001–3 | Escala das partículas relativamente à malha. |
| `points.edge_fade_m` | 2 | 0.01–100 | Zona de apagamento junto aos extremos do ciclo. |
| `points.emission` | 0.45 | 0–10 | Intensidade luminosa dos pontos físicos. |
| `sky.fall_speed_cells_s` | 0.45 | -10–10 | Velocidade dos pontos atmosféricos; positiva desce. |
| `sky.longitude_cells` | 512 | 32–2048 | Células horizontais por volta completa. |
| `sky.latitude_cells` | 135 | 16–1024 | Células por radiano vertical. |
| `sky.loop_rows` | 64 | 4–512 | Comprimento periódico da grelha vertical. |
| `sky.density` | 0.006 | 0–0.2 | Proporção de células com um ponto. |
| `sky.point_inner_radius` | 0.035 | 0–0.45 | Raio interior de cada ponto na célula. |
| `sky.point_outer_radius` | 0.22 | 0.001–0.49 | Raio exterior de cada ponto na célula. |
| `sky.emission` | 0.14 | 0–5 | Intensidade dos pontos atmosféricos. |
| `traces.speed_m_s` | 0.1 | -5–5 | Velocidade vertical dos tracejados. |
| `traces.period_m` | 2.4 | 0.1–30 | Comprimento de repetição do tracejado. |
| `traces.direction` | alternating | alternating/up/down | alternating: alterna; up: sobe; down: desce. |
| `traces.dash_start` | 0.63 | 0–0.99 | Início da faixa luminosa dentro do período. |
| `traces.dash_end` | 0.96 | 0.01–1 | Fim da faixa luminosa dentro do período. |
| `traces.minimum_edge_softness` | 0.004 | 0.00001–0.2 | Suavidade mínima da aresta. |
| `traces.emission` | 0.2 | 0–10 | Luz de base dos tracejados. |
| `traces.beat_gain` | 0.12 | 0–10 | Reforço de luz com os graves. |
| `pyramids.rotation_degrees_s` | 0.25 | -30–30 | Velocidade de rotação sobre a base. |
| `pyramids.alternate_rotation` | true | true/false | Alterna o sentido entre instâncias. |
| `pyramids.line_count` | 6 | 1–64 | Quantidade de repetições do padrão luminoso. |
| `pyramids.line_width_min` | 0.022 | 0.0001–0.3 | Largura interior das linhas. |
| `pyramids.line_width_max` | 0.036 | 0.0001–0.4 | Largura exterior das linhas. |
| `pyramids.chevron_slope` | 2 | 0–10 | Inclinação do padrão em V. |
| `pyramids.reveal_softness` | 0.12 | 0.001–1 | Suavidade do acendimento de cada linha. |
| `pyramids.reveal_order_divisor` | 12 | 1–128 | Distribuição da ordem de acendimento. |
| `pyramids.emission` | 0.3 | 0–10 | Luz das linhas acesas. |
| `pyramids.beat_gain` | 0.08 | 0–10 | Reforço das linhas com os graves. |
| `pulse.emission_gain` | 0.75 | 0–10 | Intensidade das formas arredondadas nas batidas. |
| `pulse.idle_emission` | 0 | 0–5 | Luz entre batidas; zero deixa as formas apagadas. |
| `pulse.visibility_threshold` | 0.003 | 0–1 | Limite abaixo do qual a forma deixa de desenhar. |
| `pulse.time_offset_seconds` | 0 | -2–2 | Positivo adianta o pulso visual; negativo atrasa. |
| `pulse.low_frequency_hz` | 35 | 10–500 | Frequência inferior da banda grave. |
| `pulse.high_frequency_hz` | 120 | 20–2000 | Frequência superior da banda grave. |
| `pulse.attack_seconds` | 0.025 | 0.001–0.5 | Ataque da luz. |
| `pulse.decay_seconds` | 0.09 | 0.005–2 | Tempo de decaimento exponencial. |
| `pulse.tail_seconds` | 0.36 | 0.02–5 | Duração máxima de um pulso. |
| `pulse.minimum_interval_seconds` | 0.28 | 0.05–2 | Intervalo mínimo entre transientes. |
| `pulse.relative_threshold` | 0.045 | 0.001–1 | Sensibilidade relativa à energia máxima. |
| `pulse.minimum_energy` | 0.003 | 0.000001–1 | Energia mínima detetada. |
| `pulse.background_ratio` | 1.25 | 1–10 | Energia relativa ao fundo recente. |
| `pulse.strength_reference` | 0.35 | 0.01–2 | Normalização da intensidade do pulso. |
| `pulse.onset_compensation_seconds` | 0.015 | 0–0.1 | Compensação da janela de deteção. |
| `hexagons.emission` | 0.12 | 0–10 | Luz de base dos hexágonos. |
| `hexagons.beat_gain` | 0.65 | 0–10 | Reforço dos hexágonos nas batidas. |
| `hexagons.top_normal_threshold` | 0.45 | -1–1 | Limiar para iluminar a face superior. |
| `poles.emission` | 0.22 | 0–10 | Luz de base das pontas. |
| `poles.beat_gain` | 0.55 | 0–10 | Reforço das pontas nas batidas. |
| `poles.tree_tip_height_m` | 15.4 | 0–100 | Altura local a partir da qual ilumina Tree. |
| `poles.pole_tip_height_m` | 1.34 | 0–20 | Altura local a partir da qual ilumina Pole. |
| `materials.emission_multiplier` | 4 | 0–20 | Multiplicador geral de emissão. |
| `materials.palette_mix` | 0.2 | 0–1 | Contribuição da paleta global nas luzes. |
| `materials.palette_near_mix` | 0.25 | 0–1 | Mistura entre as duas cores perto do observador. |
| `materials.palette_far_mix` | 0.75 | 0–1 | Mistura entre as duas cores à distância. |
| `materials.palette_depth_start_m` | 15 | 0–1000 | Início da mistura pela distância. |
| `materials.palette_depth_end_m` | 120 | 0.1–10000 | Fim da mistura pela distância. |
| `materials.minimum_normal_shading` | 0.3 | 0–1 | Luz mínima nas faces escuras. |
| `fog.base_density_per_m` | 0.0025 | 0–1 | Densidade de base da névoa das formas. |
| `fog.parameter_density_per_m` | 0.004 | 0–1 | Reforço pelo parâmetro global fog_density. |
| `fog.atmosphere_gain` | 0.012 | 0–1 | Contribuição das curvas de cor da névoa. |
| `floor.reflections_enabled` | true | true/false | Ativa os reflexos. |
| `floor.reflection_base` | 0.045 | 0–2 | Reflexo de base. |
| `floor.reflection_pools` | 0.23 | 0–2 | Reflexo das zonas mais espelhadas. |
| `floor.reflection_fresnel` | 0.15 | 0–2 | Reflexo dependente do ângulo. |
| `floor.fresnel_power` | 3 | 0.1–12 | Expoente da variação angular. |
| `floor.grid_spacing_m` | 6.5 | 0.1–100 | Espaçamento da grelha. |
| `floor.grid_intensity` | 0.009 | 0–1 | Intensidade da grelha; zero apaga. |
| `floor.disturbance` | 0.0012 | 0–0.05 | Perturbação dos reflexos. |
| `floor.roughness_x` | 0.0015 | 0–0.05 | Suavização horizontal do reflexo. |
| `floor.roughness_y` | 0.0008 | 0–0.05 | Suavização vertical do reflexo. |
| `floor.fog_base_per_m` | 0.0015 | 0–1 | Densidade de névoa no chão. |
| `floor.fog_parameter_per_m` | 0.008 | 0–1 | Reforço da névoa no chão pelo parâmetro global. |
| `floor.atmosphere_gain` | 0.025 | 0–1 | Contribuição da cor da névoa no chão. |
| `post.bloom_multiplier` | 1.5 | 0–10 | Multiplica o parâmetro bloom. |
| `post.exposure` | 1.15 | 0.01–10 | Exposição geral. |
| `post.vignette` | 0.25 | 0–1 | Escurecimento periférico. |
| `post.vignette_scale` | 1.2 | 0.1–3 | Área da vinheta. |
| `post.tone_mapping` | true | true/false | Ativa a compressão de brilho HDR. |
| `post.desktop_gamma` | 2.2 | 1–3 | Gamma do monitor; VR conserva a saída sRGB do runtime. |
| `families.dark.enabled` | true | true/false | Ativa esta família. |
| `families.dark.density` | 1 | 0–1 | Multiplica a presença desta família. |
| `families.dark.emission` | 1 | 0–10 | Multiplica a luz desta família. |
| `families.hex.enabled` | true | true/false | Ativa esta família. |
| `families.hex.density` | 1 | 0–1 | Multiplica a presença desta família. |
| `families.hex.emission` | 1 | 0–10 | Multiplica a luz desta família. |
| `families.trace.enabled` | true | true/false | Ativa esta família. |
| `families.trace.density` | 1 | 0–1 | Multiplica a presença desta família. |
| `families.trace.emission` | 1 | 0–10 | Multiplica a luz desta família. |
| `families.pulse.enabled` | true | true/false | Ativa esta família. |
| `families.pulse.density` | 1 | 0–1 | Multiplica a presença desta família. |
| `families.pulse.emission` | 1 | 0–10 | Multiplica a luz desta família. |
| `families.pyramids.enabled` | true | true/false | Ativa esta família. |
| `families.pyramids.density` | 1 | 0–1 | Multiplica a presença desta família. |
| `families.pyramids.emission` | 1 | 0–10 | Multiplica a luz desta família. |
| `families.points.enabled` | true | true/false | Ativa esta família. |
| `families.points.density` | 1 | 0–1 | Multiplica a presença desta família. |
| `families.points.emission` | 1 | 0–10 | Multiplica a luz desta família. |
| `families.poles.enabled` | true | true/false | Ativa esta família. |
| `families.poles.density` | 1 | 0–1 | Multiplica a presença desta família. |
| `families.poles.emission` | 1 | 0–10 | Multiplica a luz desta família. |
| `camera.vr_origin_height_m` | 1.6 | 0–10 | Altura da origem OpenXR; soma a posição real do HMD. |
| `camera.vr_origin_back_m` | 4.5 | 0–20 | Recuo da origem OpenXR. |
| `startup.window_width` | 1280 | 64–8192 | Largura da janela/espelho; requer reiniciar. |
| `startup.window_height` | 720 | 64–8192 | Altura da janela/espelho; requer reiniciar. |
| `startup.scene` | opening | opening/sequence/night/landscape/tunnel/field | opening, sequence, night, landscape, tunnel ou field; requer reiniciar. |
| `materials.body_colour` | `#0D121A` | #RRGGBB | Cor em sRGB, convertida para luz linear. |
| `fog.base_colour` | `#030609` | #RRGGBB | Cor em sRGB, convertida para luz linear. |
| `floor.fog_colour` | `#020305` | #RRGGBB | Cor em sRGB, convertida para luz linear. |
| `families.dark.colour` | `#FFFFFF` | #RRGGBB | Cor em sRGB, convertida para luz linear. |
| `families.hex.colour` | `#FFFFFF` | #RRGGBB | Cor em sRGB, convertida para luz linear. |
| `families.trace.colour` | `#FFFFFF` | #RRGGBB | Cor em sRGB, convertida para luz linear. |
| `families.pulse.colour` | `#FFFFFF` | #RRGGBB | Cor em sRGB, convertida para luz linear. |
| `families.pyramids.colour` | `#FFFFFF` | #RRGGBB | Cor em sRGB, convertida para luz linear. |
| `families.points.colour` | `#FFFFFF` | #RRGGBB | Cor em sRGB, convertida para luz linear. |
| `families.poles.colour` | `#FFFFFF` | #RRGGBB | Cor em sRGB, convertida para luz linear. |
| `startup.soundtrack` | `audio/ChezNous_MASTER.wav` | caminho local | Ficheiro de reprodução e análise de graves. |

Os sete campos em `parameters` são `palette` (duas cores), `light_intensity`, `motion_speed`, `pulse_strength`, `fog_density`, `geometry_density` e `bloom`. Os seis valores numéricos aceitam 0–1 e mantêm compatibilidade com o controlo JSON v1.


## Contador, pilares e extensão — 0.8.0

Para indicar uma edição, use por exemplo: «02:08.4, olhando para a direita: reduzir brilho das pirâmides». C alterna o contador, que na versão atual arranca desligado. `timer.visible` é aceite apenas por compatibilidade; guardar o JSON conserva a escolha feita com C. `timer.scale/x/y/opacity` afinam o painel.

Os pilares oscilam ±0,7 m em ciclos de referência de 36 s, com fases e frequências individuais. `poles.motion_amplitude_m=0` imobiliza-os; `animations.speed_multiplier` também os afeta. Alterar a amplitude pode reposicionar o pilar imediatamente. O corredor considera todo o alcance da oscilação.

As formas com material sem padrão emissivo passam a acender linhas entre 104–116 s; não se alterou a regra de escuridão entre batidas das formas arredondadas. `materials.unlit_pattern_emission=0` desativa estas linhas.

Primeira proposta musical: novas formas e laranja/dourado 136–148 s; camadas curvas e azul/violeta 172–184 s; redução de luz 208–220 s; regresso da música com anéis/túnel 224–232 s, sustentados até 240 s. Pode alterar tempos, mistura de formas e cores em `extension`. `extension.enabled=false` conserva a composição nativa durante a duração escolhida. A proposta usa também as geometrias das cenas anteriores e não declara reprodução exata do original.

| Campo novo | Inicial | Intervalo | Função |
| --- | --- | --- | --- |
| `timer.visible` | false | true/false | Compatibilidade; contador arranca desligado e C alterna visibilidade. |
| `timer.scale` | 1.5 | 0.4–3 | Tamanho do contador em cada olho e no monitor. |
| `timer.x` | 0.5 | 0.15–0.85 | Posição horizontal normalizada. |
| `timer.y` | 0.65 | 0.1–0.9 | Posição vertical medida a partir do topo. |
| `timer.opacity` | 0.95 | 0–1 | Opacidade do contador. |
| `poles.motion_amplitude_m` | 0.7 | 0–5 | Deslocamento vertical máximo dos pilares em cada sentido. |
| `poles.motion_period_seconds` | 36 | 8–240 | Período de referência da oscilação lenta dos pilares. |
| `poles.period_variation` | 0.22 | 0–0.8 | Variação individual da frequência de oscilação. |
| `materials.unlit_pattern_emission` | 0.16 | 0–3 | Luz das linhas em formas sem material emissivo recuperado. |
| `materials.unlit_pattern_lines` | 5 | 1–32 | Quantidade de linhas destas formas. |
| `materials.unlit_pattern_width` | 0.035 | 0.005–0.15 | Espessura relativa das linhas destas formas. |
| `timeline.unlit_reveal_start_seconds` | 104 | 0–424 | Início do acendimento das formas sem luz; não afeta formas de pulso. |
| `timeline.unlit_reveal_seconds` | 12 | 0.1–120 | Duração do acendimento destas linhas. |
| `extension.enabled` | true | true/false | Ativa proposta de formas e cor na segunda metade de Opening. |
| `extension.forms_start_seconds` | 136 | 0–424 | Início da entrada de novas formas. |
| `extension.forms_transition_seconds` | 12 | 0.1–120 | Duração da entrada de formas segmentadas e diamantes. |
| `extension.forms_share` | 0.75 | 0–1 | Participação das novas formas, conservando restantes formas nativas. |
| `extension.modules_start_seconds` | 172 | 0–424 | Entrada das estruturas curvas e suspensas. |
| `extension.modules_transition_seconds` | 12 | 0.1–120 | Duração da entrada da segunda família. |
| `extension.modules_share` | 0.35 | 0–1 | Participação da segunda família nas novas formas. |
| `extension.warm_start_seconds` | 136 | 0–424 | Entrada dos tons quentes. |
| `extension.warm_transition_seconds` | 12 | 0.1–120 | Duração da passagem para os tons quentes. |
| `extension.cool_start_seconds` | 172 | 0–424 | Entrada de azul e violeta. |
| `extension.cool_transition_seconds` | 12 | 0.1–120 | Duração da passagem para a segunda paleta. |
| `extension.palette_strength` | 0.85 | 0–1 | Intensidade da cor nas formas nativas durante a extensão. |
| `extension.quiet_start_seconds` | 208 | 0–424 | Início da redução luminosa na quebra musical. |
| `extension.quiet_transition_seconds` | 12 | 0.1–120 | Duração da redução luminosa. |
| `extension.quiet_minimum_light` | 0.25 | 0–1 | Luz conservada durante a quebra musical. |
| `extension.return_start_seconds` | 224 | 0–424 | Retoma luminosa no regresso da música. |
| `extension.return_transition_seconds` | 8 | 0.1–120 | Duração da retoma luminosa. |
| `extension.tunnel_start_seconds` | 224 | 0–424 | Entrada de anéis e túnel. |
| `extension.tunnel_transition_seconds` | 8 | 0.1–120 | Duração da passagem para anéis e túnel. |
| `extension.warm_first_colour` | `#FF733F` | `#RRGGBB` | Paleta da extensão. |
| `extension.warm_second_colour` | `#FFBD73` | `#RRGGBB` | Paleta da extensão. |
| `extension.cool_first_colour` | `#438FFF` | `#RRGGBB` | Paleta da extensão. |
| `extension.cool_second_colour` | `#BF6FFF` | `#RRGGBB` | Paleta da extensão. |

## Ajustes de conforto — 0.8.1

O contador está em `timer.y=0.65` (anteriormente 0.88), com `timer.scale=1.5` e `timer.opacity=0.95`. A posição é medida a partir do topo: um valor menor coloca-o mais acima. C continua a mostrar/ocultar.

Os mini-pilares SM_Pole usam agora uma malha derivada sem a sapata, mantendo o original em disco. A haste alonga/encurta desde o piso y=0; a cabeça desloca-se como bloco, sem mudar de tamanho. O deslocamento vertical do layout não levanta estes mini-pilares. `poles.motion_amplitude_m`, `motion_period_seconds` e `period_variation` continuam a controlar o movimento. O mínimo solicitado nunca inverte a haste nem leva a cabeça abaixo do piso. SM_Pole_Map já não tinha sapata e os pilares grandes conservam o movimento anterior.

`extension.geometry_density_multiplier=0.35` conserva cerca de 35% da geometria na fase colorida, mais uma pequena faixa de atenuação para desaparecer gradualmente. Aplica-se de forma dispersa às formas nativas e às novas, entra em 136–148 s e regressa a 1 durante a entrada do túnel 224–232 s. Zero elimina a geometria dessa fase; o céu, chão e contador continuam presentes. O parâmetro multiplica a densidade dos controlos existentes, mantendo a primeira metade e as cenas de comparação.

| Campo | Inicial | Intervalo | Função |
| --- | --- | --- | --- |
| `extension.geometry_density_multiplier` | 0.35 | 0–1 | Densidade relativa da fase colorida. |
| `poles.minimum_height_m` | 0.35 | 0.05–3 | Altura mínima solicitada dos mini-pilares; a espessura da cabeça impõe um limite natural. |

## Seis minutos e coreografia — 0.9.0

Opening dura 360 s. Foram preservados o áudio MASTER, o arranque imediato, os mini-pilares ancorados, o contador e as cenas de comparação. A nova configuração usa cerca de 25% da geometria colorida, menos nevoeiro e microdetalhe LED.

Os sete segmentos de cada cone formam-se da base para o topo, no seu próprio lugar. Cada segmento só inicia a rotação após ficar revelado; anéis pares e ímpares rodam em sentidos opostos. Mudar a velocidade em execução conserva a fase dos anéis ativos; anéis futuros começam a rodar no seu próprio instante de entrada. `animations.speed_multiplier` também afeta a rotação.

Saída individual das peças anteriores: **208–224 s**. Construção do túnel, da proximidade para a profundidade: **224–272 s**, durante os **48 s escolhidos pelo utilizador**. A luz axial de fundo permanece apagada até à construção completa e acende **272–284 s**. O túnel e a música continuam até **360 s**. A rotação, padrões e pulsos continuam associados ao relógio da experiência; o movimento do HMD permanece livre.

A cobertura aleatória de píxeis da transição anterior foi retirada e substituída por formação/saída das peças. As novas grelhas LED são menos finas e filtradas. A gravação WhatsApp também tem compressão H.264, mas a intervenção atua nas causas do renderer; não é uma alteração do vídeo fornecido.

`red_phase.fog_multiplier` escala uma vez os coeficientes de nevoeiro; `red_phase.atmosphere_multiplier` reduz a luz difusa do ambiente vermelho. Estes fatores desaparecem durante a formação do túnel. `patterns.led_density_scale` regula os padrões no modo dirigido: 1 conserva a grelha anterior; 0,33 é a nova predefinição.

Regras: o tempo de revelação de um segmento deve caber no intervalo entre segmentos; a janela das outras peças deve cobrir a revelação; a saída anterior deve terminar antes do início do túnel; o tempo de revelação de uma peça do túnel deve caber no seu tempo de construção. Um ficheiro que viole estas regras é rejeitado e conserva-se a configuração anterior.

| Campo novo | Inicial | Intervalo | Função |
| --- | --- | --- | --- |
| `cones.segment_interval_seconds` | 2 | 0.1–10 | Intervalo entre segmentos sucessivos, da base para o topo. |
| `cones.segment_reveal_seconds` | 0.8 | 0.05–5 | Tempo de formação de cada segmento no seu lugar. |
| `cones.rotation_degrees_s` | 2 | 0–15 | Rotação de segmentos alternados em sentidos opostos, após a revelação. |
| `cones.instance_delay_seconds` | 2 | 0–20 | Desfasamento máximo da entrada de diferentes cones. |
| `extension.previous_exit_start_seconds` | 208 | 0–424 | Início da saída individual das peças anteriores. |
| `extension.previous_exit_seconds` | 16 | 0.1–120 | Duração da saída; deve terminar antes da construção do túnel. |
| `extension.tunnel_piece_reveal_seconds` | 0.8 | 0.05–10 | Tempo de formação de cada peça do túnel. |
| `extension.tunnel_glow_reveal_seconds` | 12 | 0.1–120 | Acendimento da luz de fundo, apenas depois do túnel completo. |
| `red_phase.fog_multiplier` | 0.35 | 0–1 | Multiplica o nevoeiro da fase colorida. |
| `red_phase.atmosphere_multiplier` | 0.15 | 0–1 | Multiplica a névoa luminosa de fundo na fase vermelha. |
| `patterns.led_density_scale` | 0.33 | 0.1–2 | Escala das grelhas LED no modo dirigido; menor reduz microdetalhe instável. |
| `extension.other_piece_reveal_seconds` | 8 | 0.1–60 | Janela de entrada individual das outras formas coloridas. |

Valores atualizados: `extension.geometry_density_multiplier=0.25`, `extension.tunnel_transition_seconds=48`, `timeline.opening_duration_seconds=360`. As secções anteriores registam também as escolhas das versões anteriores.

## Brocas e anéis — 0.9.1

Esta secção substitui a entrada dos cones da versão 0.9.0. A broca sobe inteira através do piso: a ponta aparece primeiro e o material que fica abaixo do chão é recortado também no reflexo. Durante a subida, todos os módulos rodam juntos. Só quando a base chega à superfície se abrem as folgas; depois começa a rotação independente com sentidos alternados. O desfasamento `cones.instance_delay_seconds` continua a variar a entrada de cada cone. A mudança das velocidades de rotação em execução preserva os ângulos; editar os tempos ou a folga altera diretamente a coreografia.

| Campo | Inicial | Função |
| --- | --- | --- |
| `cones.emerge_seconds` | 14 | Duração da subida inteira através do piso. |
| `cones.separate_seconds` | 3 | Duração da abertura das folgas após a emergência. |
| `cones.drill_rotation_degrees_s` | 8 | Rotação comum durante a subida. |
| `cones.module_gap_m` | 0.6 | Folga adicional entre módulos depois de separados. |
| `cones.rotation_degrees_s` | 2 | Velocidade dos módulos em sentidos alternados. |
| `extension.tunnel_departure_start_seconds` | 270 | Início do afastamento dos artefactos do túnel. |
| `extension.tunnel_departure_seconds` | 30 | Duração até restarem os anéis. |
| `extension.tunnel_departure_distance_m` | 100 | Distância radial do afastamento, também com avanço em profundidade. |

Os anéis desta experiência ficam centrados no eixo do percurso, formando uma sequência de 64 anéis por onde a câmara avança. Os outros artefactos afastam-se lentamente com desfasamentos individuais entre 270–300 s; desaparecem no fim do afastamento. A montagem 224–272 s e a luz 272–284 s permanecem. Não há uma nova cena depois dos anéis nesta versão.

O contador **arranca sempre desligado** e só a tecla **C** altera a visibilidade; recarregar o JSON mantém a escolha feita pelo teclado. `timer.visible` conserva-se apenas para aceitar ficheiros anteriores e é ignorado no arranque e na recarga. `timer.x`, `timer.y`, `timer.scale` e `timer.opacity` continuam ativos. Em VR estas posições referem-se a uma placa comum a dois metros da cabeça; cada olho recebe a projeção correspondente, incluindo a diferença de posição e de campo de visão. No monitor continuam a ser coordenadas normalizadas do ecrã.

`cones.segment_interval_seconds` é uma opção de compatibilidade sem efeito na nova broca. `cones.segment_reveal_seconds` ainda regula a suavidade da entrada das outras formas, em conjunto com `extension.other_piece_reveal_seconds`; conservam-se as validações desses campos para aceitar os mesmos ficheiros.

## Cones espectrais e cristais — 0.10.0

As escolhas desta secção substituem a broca e as folhas anteriores. Os cones têm ponta superior e base larga, formada por nove conjuntos de anéis e nervuras. Aos 136 s começa a construção da base até à ponta, como o utilizador confirmou. Cada peça aparece com a dimensão final, por fade de 0,8 s, com intervalo de 2 s entre peças e desfasamento por cone até 2 s. Só depois de completar o cone começa a rotação comum de 2 graus/s. Não há separação nem contrarrotação de módulos.

Os cristais substituem as folhas da fase noturna/colorida. Há seis malhas distintas, com combinações de altura, inclinação, orientação e distribuição de nove prismas hexagonais pontiagudos. Entram peça a peça a partir dos 172 s, em dimensão final. A fase mantém a sua densidade reduzida e o corredor considera a espessura dos novos aglomerados.

| Campo | Inicial | Função |
| --- | --- | --- |
| `cones.segment_interval_seconds` | 2 | Intervalo entre as nove peças, da base à ponta. |
| `cones.segment_reveal_seconds` | 0.8 | Fade de cada peça do cone, em dimensão final. |
| `cones.instance_delay_seconds` | 2 | Desfasamento máximo entre cones. |
| `cones.rotation_degrees_s` | 2 | Rotação comum depois de terminar a construção. |
| `cones.emission` | 0.8 | Brilho dos anéis e nervuras. |
| `cones.cyan_colour` | #00D9FF | Cor ciano das linhas. |
| `cones.magenta_colour` | #FF24D7 | Cor magenta das linhas. |
| `crystals.piece_interval_seconds` | 0.65 | Intervalo entre prismas. |
| `crystals.piece_reveal_seconds` | 1 | Fade de cada prisma em dimensão final. |
| `crystals.instance_delay_seconds` | 3 | Desfasamento máximo entre aglomerados. |
| `crystals.variants` | 6 | Número de variantes usadas, inteiro de 1 a 6. |
| `crystals.emission` | 0.9 | Brilho das arestas. |
| `crystals.body_brightness` | 0.045 | Luz das facetas interiores. |
| `crystals.edge_width` | 0.035 | Espessura visual das arestas nas coordenadas da faceta. |
| `crystals.cyan_colour` | #34F6FF | Primeira cor dos cristais. |
| `crystals.violet_colour` | #A23FFF | Segunda cor dos cristais. |

`cones.emerge_seconds`, `cones.separate_seconds`, `cones.drill_rotation_degrees_s` e `cones.module_gap_m` são aceites para compatibilidade e **já não alteram a experiência**. Ao contrário da versão anterior, `cones.segment_interval_seconds` voltou a controlar a construção. As velocidades de rotação continuam a preservar o ângulo na recarga; editar tempos e cores altera diretamente a coreografia.

Não há animação geométrica de escala na densidade, entrada ou saída, incluindo na construção do túnel. As peças conservam tamanho e proporções; a opacidade é ordenada por profundidade em cada olho, depois das malhas opacas e do piso. Os mini-pilares movem-se por translação e têm uma extensão fixa de haste abaixo do chão, recortada no passe principal e no reflexo. A altura mínima limita a translação, sem deformar o poste.

Os materiais dos cristais representam facetas e arestas luminosas; não simulam refração física de vidro. A descrição acima documenta a entrega 0.10.0; os novos tempos e parâmetros estão na secção seguinte. O contador permanece desligado até C e a duração mantém-se em 360 s.

## Afinação do cenário — 0.11.0

Esta secção é a referência para a configuração atual. O ambiente quente conserva os reflexos; `red_phase.saturation` reduz o vermelho sem aplicar uma paleta fria. Os cones têm oito nervuras completas e nove círculos independentes. As nervuras desenham-se em altura e só depois entram os círculos, da base à ponta. `cones.beat_sync=true` alinha as entradas com os ataques graves detetados no áudio selecionado. Um intervalo sem ataques usa o tempo programado, sem bloquear a montagem.

| Campo | Valor inicial | Efeito |
| --- | --- | --- |
| `layout.retain_behind_m` | 60 | Distância depois da passagem antes de começar a dissolução. |
| `layout.behind_fade_m` | 30 | Distância ao longo da qual a peça desaparece gradualmente. |
| `layout.remove_dark_triangles` | true | Remove sólidos triangulares nativos e procedurais. |
| `layout.remove_platforms` | true | Remove plataformas procedurais e bases Hex/Lower do Forest. |
| `red_phase.saturation` | 0.32 | Saturação da atmosfera quente; 1 recupera o vermelho anterior. |
| `red_phase.fog_multiplier` | 0.28 | Intensidade relativa do nevoeiro da fase colorida. |
| `red_phase.atmosphere_multiplier` | 0.10 | Intensidade relativa da névoa luminosa quente. |
| `cones.beat_sync` | true | Sincroniza a entrada das 17 peças com ataques graves. |
| `extension.ring_alignment_start_seconds` | 290 | Início do alinhamento e crescimento dos anéis, perto de 4:50. |
| `extension.ring_alignment_seconds` | 16 | Janela da transformação dos 64 anéis. |
| `extension.ring_inner_radius_m` | 5 | Raio útil do buraco de cada anel, mínimo ampliado conforme o corredor. |
| `extension.ring_spacing_m` | 8 | Distância entre centros ao longo do percurso. |
| `extension.ring_lead_distance_m` | 40 | Antecedência mínima; inclui automaticamente margem para formar o primeiro anel. |
| `extension.tunnel_departure_start_seconds` | 290 | Início da dispersão individual dos restantes artefactos. |
| `extension.tunnel_departure_seconds` | 16 | Duração da dispersão; no fim restam os anéis. |
| `extension.tunnel_glow_colour` | #FF58CF | Cor do brilho axial ao fundo. |
| `extension.tunnel_glow_emission` | 1.4 | Intensidade do brilho rosa. |
| `extension.tunnel_glow_reveal_seconds` | 12 | Duração do acendimento, iniciado após 25% da janela de alinhamento. |

Cada anel demora metade da janela a formar-se, com entradas desfasadas da proximidade para a profundidade: o primeiro completa-se aos 298 s, o último aos 306 s. O afastamento inicial reserva espaço para a velocidade máxima de `motion_speed` durante essa formação. Na configuração inicial, a antecedência efetiva é 50 m. O raio útil inicial é 5 m (diâmetro 10 m); se o corredor exigir mais espaço, aumenta automaticamente. A espessura longitudinal mantém-se inferior ao espaçamento.

A distância de permanência é medida em relação à câmara e à extensão da peça, tanto na vista principal como nos reflexos. A reciclagem do campo procedural ocorre depois de a peça estar dissolvida. As transições musicais de saída de uma fase continuam a funcionar independentemente destas distâncias. As alterações de velocidade preservam a posição do túnel já formado; durante a montagem, uma aceleração extrema antecipa suavemente a conclusão das peças próximas antes da margem de 10 m. Guardar o JSON ou carregar F5 aplica os campos durante a execução; editar diretamente os tempos, raio, espaçamento ou antecedência redefine a respetiva coreografia.

Os pequenos pontos permitidos no corredor conservam emissão branca, incluindo na fase quente. O movimento livre do headset e o corredor configurado são respeitados; a validação automática usa um runtime OpenXR simulado, sem substituir a observação no Quest 3 físico.
