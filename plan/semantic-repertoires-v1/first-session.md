# Especificação da primeira sessão

Data: 2026-10-14 · Revisão documental 2 · Artefacto: Prismatic Lumen (identificadores Fantasynt) · Base: 0.12.0, `bc86e22`.

## Estatuto e documentos de referência

**Proposta de experiência**, ainda sem implementação nem resultados com participantes. A evidência de código e configuração está em [notes.md](notes.md); as fichas em [repertoire.md](repertoire.md), a cadeia operacional em [behavior-matrix.md](behavior-matrix.md), a implementação em [implementation-plan.md](implementation-plan.md) e a avaliação em [first-exposure-protocol.md](first-exposure-protocol.md). Estes são os documentos ativos; `backups/` conserva apenas o estado anterior.

Neste conjunto, **E** significa existente e verificado por inspeção estática; **P**, funcionalidade proposta; **H**, hipótese percetiva; **A**, decisão experimental aberta. E não significa funcionamento medido nesta instalação Quest 3. Todos os tempos de episódios, limites e mapeamentos P são hipóteses de desenho ajustáveis antes do congelamento do piloto.

**H central:** continuidade, contingência parcial e iniciativa num ambiente causalmente opaco podem favorecer agência atribuída ao ambiente e uma experiência de autoria ambígua. É admissível que o artefacto seja vivido apenas como música e coreografia. A experiência não precisa de tornar reconhecíveis os nomes do repertório.

## Pressupostos de trabalho e decisões

**P:** uma sessão audiovisual de 424,182 s, com a composição Opening e o MASTER atuais. A preparação do equipamento ocorre antes desse relógio; a entrevista depois. Não haverá segunda exposição nem comparação de versões pela mesma pessoa. Sem controladores, voz, conversa com o sistema, avatar ou personagem. Adquirir cabeça e mãos; conservar as mãos fora da imagem do participante, visíveis apenas no diagnóstico do operador. A representação das mãos permanece A para estudos futuros e fica igual nas três condições deste piloto.

**P:** postura sentada estável, sem deslocação física solicitada, com espaço livre para movimentos espontâneos de braços/cabeça. Conservar a deslocação virtual atual de 1,5 m/s a `motion_speed=0.3`. Esta opção limita variação entre condições, mas o fluxo ótico pode dominar a leitura corporal e causar desconforto: é um risco a testar tecnicamente, não uma garantia de conforto. Se for necessário alterar velocidade ou postura, alterar todas as condições antes do piloto e versionar o desenho.

**P:** manter sustentar, ressoar e iniciar. O incremento mínimo acrescenta uma memória operacional comum: um motivo conserva relações temporais reconhecíveis entre reaparições, embora mude de realização com a fase. Propagar fica adiado porque falta controlo semântico por regiões e fontes sonoras. Contrastar é uma propriedade limitada de iniciar/ressoar, não uma quarta política no piloto. Assim, não se confunde aumento de categorias com melhoria da experiência.

Os três comportamentos são suficientes para **pôr à prova** a questão, se uma consequência persistente e uma reciprocidade puderem ser detetadas na primeira exposição. Não são declarados suficientes para produzir agência. Sustentar pode não ser identificável isoladamente: a sua função é tornar uma mudança e o seu vestígio legíveis ao longo do tempo.

## Unidade percetiva: motivo e consequência

**P:** usar um motivo temporal global de atenuação e recuperação da emissão e do ganho do MASTER. Exemplo interno: uma diminuição longa seguida de duas recuperações separadas. Não o designar como respiração, humor ou resposta a um gesto perante o participante. A mesma organização temporal reaparece, mais espaçada ou reduzida, numa fase posterior; a música, o percurso e as famílias geométricas continuam.

A diferença em relação a um preset é a sequência **acontecimento → oportunidade corporal → continuação → vestígio**. A diferença em relação a imitação é preservar uma relação temporal ampla, sem seguir a posição de uma mão, reproduzir uma trajetória ou exigir uma forma corporal. A memória referencia observações e planos aplicados, nunca afirma que o participante quis responder.

**P:** toda a sessão oferece três iniciativas locais e cinco oportunidades de continuação/retorno. Os horários e o orçamento não dependem da quantidade de movimento. Uma oportunidade pode resultar em sustentar ou em desenvolvimento autónomo quando não há motivo corporal válido; registar essa distinção. Não fabricar ressonância para completar uma quota.

**H:** a reaparição de um motivo depois de o participante ter mudado ou parado pode dar continuidade a uma relação. Pode também parecer um refrão musical. São interpretações concorrentes a recolher.

## Antes do relógio audiovisual

**P, duração indicativa 2–4 minutos:** ajustar headset e áudio, explicar limites físicos e interrupção, verificar visão/seguimento e permitir olhar em volta numa vista neutra estática, sem MASTER nem motivos adaptativos. Não usar uma prévia de Opening: já seria uma exposição parcial ao artefacto. A vista neutra e o controlo de arranque supervisionado precisam de implementação; entretanto `--paused` serve apenas nos ensaios técnicos.

Instrução comum proposta: «Vai estar num espaço audiovisual durante cerca de sete minutos. Pode olhar e mover a cabeça e as mãos como lhe for confortável, ou permanecer quieto. Não há uma tarefa a completar nem movimentos corretos. Pode interromper a qualquer momento.» Não mencionar inteligência, entidade, agência, responsividade ou condições. Não pedir gestos de treino. Verificar o equipamento no diagnóstico do operador com movimento livre; usar limiares físicos comuns, sem calibrar uma personalidade por participante.

Testar uma chamada verbal simples ao operador para parar e remoção assistida do headset. Isto é uma via humana de interrupção, não captura de voz nem conversa necessária com o sistema. A ausência de fala não altera a experiência. O botão/tecla local do operador deve funcionar mesmo sem rede; o teclado atual exige foco e não constitui ainda essa garantia. Não iniciar recolha até validar a interrupção supervisionada, incluindo uma pessoa que não consegue falar.

## Sequência temporal proposta

O relógio abaixo é o tempo de sessão associado ao áudio, começando em zero após a preparação. Os limites coreográficos são E; episódios e pontos de aplicação são P. Não há títulos ou avisos visíveis. O participante encontra uma continuidade com acontecimentos intercalados, não três demonstrações sucessivas.

| Intervalo | Organização existente E e função proposta P | Episódio relacional |
|---|---|---|
| 0–40 s | Crescimento de pontos até 40 s, céu até 45 s e presença até 60 s; câmara já avança. | E0: chegada; apenas trajetória base e observação. |
| 40–76 s | Continuidade ainda em crescimento; sem mudança de cena solicitada. | E1: primeiro motivo autónomo aos 50 s, 12 s; oportunidade de continuação aos 66 s, 8 s. |
| 76–124 s | Fim Dome aos 80 s; Forest 80, Ground/Intro 112; linhas 104–116. | E2: conservar vestígio em memória; suspender novos acentos relacionais para não disputar revelações/saídas. |
| 124–164 s | Formas/cones e paleta quente começam aos 136 s, transição artística 12 s. | E3: retorno de motivo aos 152 s, 10 s; observar continuação com luz global, sem pretender controlar cores próprias dos cones. |
| 164–208 s | Módulos/paleta fria a partir de 172 s, transição 12 s. | E4: nova iniciativa aos 188 s, 12 s; continuação aos 202 s, 6 s. |
| 208–306 s | Saída 208–224, redução de energia 208–220, recuperação 224–232, túnel 224–272; alinhamento 290–306. | E5: atravessar transformações coreográficas; sustentar a relação em memória, sem novos acentos. Não congelar a música nem a coreografia. |
| 306–352 s | Anéis pares/ímpares respondem aos graves com canal próprio. | E6: iniciativa aos 316 s, 12 s; continuação aos 334 s, 10 s. Usar emissão global e ganho, preservando alternância e cores dos anéis. |
| 352–408,182 s | Fase final estabilizada, pulsos musicais continuam. | E7: reaparição aos 370 s, 12 s; deixar o motivo terminar até 392 s. Oportunidade final de influência sem prometer um efeito a cada movimento. |
| 408,182–424,182 s | Desvanecimento visual existente de 16 s, incluindo contador se ativo; áudio chega ao fim do MASTER. | E8: retirar contribuição adaptativa antes do fade; nenhuma nova decisão. Não acrescentar corte/fade sonoro sem testar o fim do ficheiro. |

Os horários 50/66/152/188/202/316/334/370 são sementes de um guião técnico fixo, não estimativas científicas. Nenhum episódio pode ultrapassar o intervalo protegido seguinte. Sustentar liga todos os acontecimentos; iniciar e ressoar não formam blocos didáticos. Se a música mascarar estes momentos, rever o guião nas três condições, mantendo registo da alteração.

## Episódios: observação, decisão e consequência

| Episódio e estado inicial | Acontecimento e participação possível | Informação observada e incerteza | Decisão e transformação P | Vestígio |
|---|---|---|---|---|
| E0, sem motivo aplicado | Crescimento e movimento virtual. Olhar, mover, parar ou apenas ouvir. | Poses válidas de cabeça/mãos, cobertura, cena, áudio e deslocação automática. Orientação de cabeça não equivale a olhar ocular; repouso não equivale a desinteresse. | Sustentar base; recolher descritores sem os transformar em perfis psicológicos. | Linha de base corporal local e ausência/presença de fontes, não uma etiqueta de pessoa. |
| E1, base parcialmente estabelecida | Primeiro motivo aparece sem gesto desencadeante. Pessoa pode orientar-se, mudar ritmo, parar ou nada alterar. | Padrões antes/durante/depois e plano realmente aplicado. Mudança posterior pode ter origem musical ou ocorrer por acaso. | Iniciar motivo M1; na oportunidade aos 66 s ressoar se existir padrão temporal válido ou sustentar/desenvolver M1 autonomamente. | M1, proveniência autónoma/contingente, sequência observada e validade. |
| E2, M1 concluído | Coreografia revela/retira elementos; pessoa explora ou permanece. | Separar revelações artísticas de planos relacionais. Referência espacial e movimento físico, sem confundir avanço virtual. | Sustentar; manter memória, contribuição física neutra nesta travessia. | Relação temporal de M1 permanece disponível mesmo sem objetos anteriores. |
| E3, nova fase e M1 na memória | M1 reaparece aos 152 s, com duração admissível à fase. Pessoa pode reconhecer, ignorar, mover-se de outra forma ou ficar quieta. | Memória de M1 e observações recentes, amplitude audiovisual efetiva, sobreposição musical. Não inferir reconhecimento. | Ressoar incorporando padrão válido recente em M1; se não existe, retorno autónomo explicitamente marcado. Mesma família de motivo, amplitude/dose limitada. | M1' e ligação ao plano anterior; conservar se a transformação foi aplicada e se mudou o padrão observado. |
| E4, continuidade depois da transição fria | M2 aos 188 s; aos 202 s a continuação conserva ou transforma parte de M1'/M2. | Ritmos de cabeça e mãos, qualidade separada, atuação anterior e oportunidades já usadas. Padrões discordantes não são emoções contraditórias. | Iniciar M2 sem necessidade de movimento; ressoar num motivo elegível aos 202 s ou sustentar. Não aumentar energia porque a pessoa se moveu mais. | Até dois motivos ativos com histórico de versões; nenhuma acumulação irrestrita. |
| E5, forte mudança artística | Túnel, dispersão e anéis avançam segundo a coreografia. Pessoa pode virar-se, acompanhar ou repousar. | Incerteza maior sobre qual acontecimento antecedeu um movimento. Canal de pulsação final distinto do parâmetro global. | Sustentar continuidade narrativa através da memória; contribuição adaptativa neutra. Não interpretar cada batida como resposta. | Motivos anteriores e estado de transição; não transformar ausência de modulação em ausência de história. |
| E6, anéis já estabelecidos | M3 autónomo aos 316 s; aos 334 s reaparece uma organização de M1/M2 ou incorpora um padrão recente. | Poses, música/pulsos efetivos, planos anteriores e cobertura. Cabeça orientada num sentido não prova atenção a um anel. | Iniciar e depois ressoar/sustentar; emissão e ganho globais. Nenhuma propagação de som/causalidade por região é anunciada. | M3 e retorno M1/M2 com proveniência identificada para análise. |
| E7, motivos conservados | Reaparição final sem reiniciar a sequência. Exploração, imobilidade ou resposta pouco legível são possíveis. | História de toda a sessão resumida, últimos 10 s, limites e prazo de encerramento. Não usar entrevista nem previsão de agência. | Ressoar se elegível; caso contrário, continuação autónoma de um motivo aplicado. Terminar até 392 s e regressar suavemente à base. | Encadeamento final e quantos episódios foram tecnicamente contingentes. |
| E8, contribuição neutra | Fade e fim do MASTER. Pessoa pode ficar quieta ou pedir para terminar. | Estado técnico e motivo de interrupção apenas. | Fechar sessão; recusar planos pendentes. Recolha aberta depois de retirar headset. | Registo fechado, sem avaliação subjetiva produzida pelo modelo. |

## Três percursos possíveis

**Exploração ativa.** Em E1 a pessoa alterna movimentos dos braços e orientação da cabeça. O sistema encontra um padrão temporal válido, relaciona-o com M1 e realiza a continuação. Em E3 retoma uma proporção temporal de M1' enquanto conserva as formas novas; em E4 inicia M2 apesar de uma pausa corporal. E6 pode incorporar outro ritmo sem substituir toda a história. O vestígio não desaparece quando os movimentos param. H: reciprocidade e iniciativa podem coexistir; alternativa: a pessoa sente um instrumento previsível ou apenas sincronia musical.

**Maioritariamente imóvel.** E0 confirma baixa variação com seguimento válido. E1, E4 e E6 continuam a acontecer nos mesmos horários, sem escalar o estímulo. Uma pequena orientação seguida de repouso pode fornecer um padrão válido; imobilidade constante sem mudança não fornece esse motivo. E3/E7 retomam motivos autónomos, sem falsa etiqueta de ressoar. O percurso continua completo e coerente, mas pode conter poucas oportunidades efetivamente contingentes. O resultado dessa pessoa informa a experiência de autonomia/coreografia; não testa isoladamente o mecanismo de reciprocidade. H: iniciativa com memória pode ser percetível sem exploração; ausência de atribuição também é um resultado válido.

**Movimentos difíceis de interpretar.** Cabeça e mãos apresentam ritmos diferentes, movimento irregular ou cobertura intermitente. O código conserva descritores por fonte e não força um padrão único. Usa um motivo apenas quando o critério local de estabilidade é satisfeito; se não, sustenta o motivo aplicado ou inicia na agenda comum. Uma mudança de origem invalida derivadas e abre um novo segmento. E3/E6/E7 preservam motivos anteriores válidos, sem intensificar para obter resposta. H: coerência pode sobreviver à incerteza interpretativa; reciprocidade pode falhar. Movimento irregular com boa cobertura e dados perdidos são casos distintos no registo e na análise.

## Falhas e continuidade

**P:** mãos perdidas impedem novos descritores dessas mãos; cabeça válida continua disponível. Manter um motivo já calculado até ao seu fim, sem extrapolar movimento invisível. Dados ausentes não geram iniciativa extra. Perda breve de ambas as mãos não exclui automaticamente a sessão. Cabeça sem pose válida ou suspensão XR exige tratamento técnico local: parar execução temporal/áudio de forma coordenada, invalidar pedidos e retomar apenas se a referência for consistente. Esta coordenação ainda não existe na base atual.

Falha sustentada é uma sessão interrompida/incompleta segundo critérios pré-fixados; não reiniciar do princípio com a mesma pessoa para a contar como primeira exposição. Rede lenta não alonga um motivo nem cria um atraso artístico. Os planos autorizados terminam localmente; depois sustentar a base. Todos os percursos têm encerramento, sem «recompensa» por atividade.

## Critérios para avançar

Antes do piloto, demonstrar no headset aquisição de cabeça/mãos e interrupção; verificar emissão e ganho nas três fases relevantes; reproduzir o guião com motivos e sensores sintéticos; verificar que a memória referencia efeitos realmente aplicados. Uma mera confirmação JSON não satisfaz estes critérios.

O piloto deve esclarecer se a condição transparente permite perceber relação sem explicação, se a opaca conserva alguma responsividade e se reaparições são reconhecidas espontaneamente. Se os relatos forem só sobre música/coreografia, não acrescentar um avatar para «salvar» a hipótese: rever o intervalo, material sonoro ou atuador em desenvolvimento e repetir com novas pessoas.

## Decisões, riscos e incremento mínimo

Decisões principais: manter a duração atual; proteger mudanças artísticas; usar três políticas com memória transversal; conservar movimento virtual e música; adotar controlo global de emissão e atenuação sonora; começar por regras locais substituíveis. A justificação é testar relação e continuidade com poucos graus de liberdade, em vez de testar simultaneamente fornecedor, espacialização e novos conteúdos.

Riscos abertos: atenuação parecer apenas dinâmica musical; motivos demasiado fracos ou demasiado evidentes; velocidade virtual dominar a experiência; retorno não ser reconhecido; seguimento insuficiente; transparência não ser detetável sem instrução; opacidade retirar toda a relação; gain global empobrecer o som. Não há efeito percetivo garantido.

A menor implementação investigável é aquisição normalizada, executor local de motivos com memória, dois atuadores globais, três condições com reprodução de planos e registos sincronizados. Não exige Jev, novos objetos ou fontes sonoras. A primeira tarefa de programação é P0 do [plano técnico](implementation-plan.md#p0--observação-sincronizada-e-manifesto-da-sessão), com conclusão objetiva antes de qualquer adaptação.
