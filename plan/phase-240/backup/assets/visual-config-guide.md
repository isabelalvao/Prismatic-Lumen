# Afinação visual sem recompilar

Edite `visual-config.json` junto do executável, guarde em UTF-8 e volte à experiência. O programa verifica alterações a cada 0,25 s. **F5** força a releitura. JSON inválido mantém a última configuração válida; o erro fica em `logs/vr.log` ou `logs/desktop.log` nos lançadores do pacote.

Na pasta do projeto, `Fantasynt.cmd` usa o ficheiro `C:/ORCA/Fantasynt/visual-config.json`. Um executável iniciado diretamente usa o ficheiro junto dele. `--config CAMINHO.json` permite escolher outro. O caminho efetivo aparece no registo de arranque.

O movimento e a presença luminosa começam imediatamente: `camera.delay_seconds=0`, `camera.acceleration_seconds=0`, `startup.auto_play=true`. A presença inicial é reduzida e cresce gradualmente. As formas arredondadas continuam apagadas quando a música não tem um pulso detetado. A entrada das pirâmides mantém-se nos 80 s e também é editável.

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

O catálogo afina a experiência atual **Opening**. Os parâmetros globais, câmara, pontos atmosféricos, chão e pós-processamento também afetam as cenas anteriores disponíveis para comparação. As malhas e a lógica dos shaders continuam em código; trocar a forma de uma malha ou criar um efeito novo exige desenvolvimento. As instâncias preparadas têm um limite de 65 000 para manter a capacidade de renderização.

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
| `timeline.opening_duration_seconds` | 120 | 1–424 | Fim da experiência Opening. |
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

