# Matriz operacional e registo

Data: 2026-10-14 · Revisão 2 · **Tudo abaixo é P**, exceto limitações E explicitamente indicadas. Indícios H não são efeitos demonstrados. Fichas: [repertoire.md](repertoire.md); contexto temporal/contratos: [implementation-plan.md](implementation-plan.md).

| Comportamento | Contexto necessário | Decisão delimitada | Transformação proposta | Indício percetivo H | Registo necessário |
|---|---|---|---|---|---|
| Sustentar após acontecimento | Fase, curva/estado efetivo, motivo aplicado, cobertura corporal, próxima oportunidade. | Concluir e preservar; não adicionar acento. | Trajetória existente até fim; regresso suave à base; memória mesmo com contribuição neutra. | Continuidade que persiste quando a pessoa para. | ID ancestral, envelope atual/final, parâmetro base/efetivo, duração, motivo de saída, cobertura por fonte. |
| Sustentar nas travessias E2/E5 | Alterações artísticas e janelas protegidas; histórico de planos. | Inibir novos acentos relacionais, conservar memória. | Coreografia/música seguem; modulação neutra. | Continuidade entre fases, ou perda completa da relação. | Marcos de fase, estado da memória, capacidade por fase, ausência planeada de modulação. |
| Ressoar em padrão recente | 2/10 s de corpo válido conforme condição, estado audiovisual e 3 episódios, motivo anterior e estabilidade local. | Incorporar padrão elegível; selecionar motivo/proposta de graduação autorizada. | Curva global de emissão/ganho, relações temporais preservadas e retemporização autorizada. | Influência própria e reciprocidade sem reconhecer um comando. | Referências às amostras, fontes usadas/ignoradas, validade, descritores, ID do contexto, escolha, condição, plano/aceitação/aplicação e curva efetiva. |
| Ressoar após movimento→repouso | Transição observada com cabeça/referência válidas; período antes/depois. | Usar mudança temporal, sem etiqueta emocional. | Continuação mantém parte do motivo depois do movimento; dose não cresce com atividade. | Consequência duradoura; alternativa: mera coincidência musical. | Tempos de mudança, confiança do decisor se existir, critério de elegibilidade, motivo e proveniência. |
| Retorno de motivo sem padrão atual | Motivo anterior aplicado; corpo imóvel ou ausência de padrão estável, qualidade separada. | Sustentar/iniciar retorno autónomo; ressoar inelegível para novo padrão. | Motivo conhecido reaparece globalmente, conforme guião. | Memória/iniciativa; não prova de reciprocidade atual. | `origin=autonomous_return`, ancestralidade, motivo de inelegibilidade, número de oportunidades efetivamente contingentes. |
| Iniciar E1/E4/E6 | Agenda/seed congelada, fase/capacidades, memória e orçamento. | Abrir acontecimento autorizado, independentemente de gesto simultâneo. | M1/M2/M3 na mesma dose, com diferença temporal limitada. | Origem própria do acontecimento; alternativa: coreografia pré-programada. | Agenda e versão, seed, `origin=autonomous`, tempos previstos/efetivos, sinais antes/depois separados de interpretação. |
| Qualquer política com mãos perdidas | Validade de cabeça, esquerda/direita e última origem espacial. | Impedir nova inferência dessas mãos; usar fonte elegível restante ou terminar plano. | Sem novo acento por falha, sem escala de compensação. | Coerência sob perda parcial; possível diminuição de responsividade. | Intervalos inválidos, fontes efetivas, descartes, expiração, distinção entre repouso válido e ausência. |
| Estado técnico com cabeça inválida/recentragem/suspensão | Flags XR, origem, relógios, pedidos pendentes. | Pausa/cancelamento local e invalidação do segmento; não comportamento artístico. | Tratamento técnico coordenado; sem transformar glitch em iniciativa. | Nenhuma inferência de agência a partir da falha. | Motivo técnico, pausa/retoma, nova origem, áudio, plano interrompido, integridade da sessão. |
| Reprodução independente | Planos/motivos/seed/condição da fonte, tempo atual; corpo atual só para observação técnica. | Executar fonte; bloquear descritores atuais para a decisão. | Mesmas curvas/envelopes, trajetórias artísticas e áudio-fonte; vista XR atual. | Agência/ambiguidade comparável indicaria contribuição do conteúdo ou expectativa. | Sessão-fonte/hash, alinhamento, escolhas gravadas, ausência de leitura decisória atual, exceções técnicas locais. |
| Propagar/contrastar, expansão | Regiões ou canais que ainda não estão no contrato semântico. | Indisponível no piloto. | Não substituir por uma mudança global e declarar equivalente. | A testar num estudo posterior. | Capacidades E insuficientes e razão de adiamento. |

## Registo comum mínimo P

| Camada | Campos mínimos e utilização |
|---|---|
| Manifesto | ID pseudónimo de sessão, condição mascarada para análise, build/commit, hashes de configuração/áudio/repertório/rubrica, seed, runtime, dispositivo/volume, postura e familiarização. Sem credenciais. |
| Observação | Sequência, tempo monotónico, `XrTime` consultado/previsto, espaço/origem, posição/orientação física de cabeça e articulações selecionadas, flags válidas/seguidas e cobertura. Vista virtual registada separadamente. |
| Ambiente | Tempo consumido SDL e tempo de sessão, fase, câmara virtual, base, composição, contribuição, limite/clamp, resultado enviado a GPU/áudio, envolvente grave e capacidade ativa. Resultado enviado não equivale a luminância/volume físico medidos. |
| Contexto | Intervalos 2/10 s e memória usada, fonte de cada descritor, ausências/incertezas, episódios anteriores com planos efetivos e observações posteriores. Hipóteses do modelo em campo separado. |
| Decisão/plano | Decisor/versão, ID do contexto, elegibilidade, motivo escolhido, abstenção/confiança, prazo, ID idempotente, origem autónoma/contingente/histórica, ancestralidade, envelope e condição. |
| Aplicação | Instantes de receção/validação/início/fim, rejeição/substituição/cancelamento, estado efetivo, latência técnica de cada trecho e atraso artístico deliberado separado. |
| Integridade | Filas limitadas, descartes, pausas, mudanças de origem, interrupções, recargas tentadas, falhas de áudio/rede/XR, cobertura por oportunidade e resultado completo/incompleto. |
| Participante | Descrição livre antes das perguntas dirigidas, exemplos ancorados em episódios, respostas separadas às dimensões, incerteza/não aplicável, exposição prévia/conhecimento suspeitado. Dados subjetivos não entram no decisor. |

Guardar descritores suficientes para o contexto e planos efetivos para reprodução. Uma sessão diagnóstica pode conservar poses/articulações; definir retenção e consentimento antes de recolha. O caminho remoto, se introduzido, recebe resumos delimitados, não esqueletos frame a frame nem entrevista.

## Dois tipos de reprodução

**Reprodução técnica:** observações registadas → descritores → decisões locais → planos; verifica equivalência de cálculos dentro de tolerâncias. **Condição experimental independente:** planos efetivos registados → execução audiovisual, sem voltar a decidir com os sinais da pessoa atual. Conservar ambos evita que uma reprodução supostamente independente se torne contingente.

Teste técnico de contingência: trocar apenas observações válidas deve poder alterar uma continuação autorizada em T/O; não deve alterar a agenda autónoma nem planos R. Essa verificação demonstra dependência computacional, não intenção percebida nem relação causal psicológica.
