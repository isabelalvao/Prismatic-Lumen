# Repertório de comportamentos — primeira sessão

**Estado de 2026-10-15:** o [P1 programado](../p1-motifs-2026-10-15/README.md) executa uma diminuição longa com entrada/saída de 2 s e retornos na agenda comum. Não implementa ainda seleção entre as três famílias, ressonância corporal ou condições experimentais. As fichas abaixo continuam a especificação do repertório completo.

Data: 2026-10-14 · Revisão 2 · **P: especificação, sem implementação semântica no motor.** E/H/A seguem a legenda de [first-session.md](first-session.md).

## Unidade e revisão do repertório

Um comportamento é uma conduta temporal com contexto, limites, memória e saída; não é um preset emocional nem uma classificação da pessoa. A leitura combina expressão corporal, estado audiovisual efetivo, história e consequências das ações aplicadas. Os nomes ficam internos.

Mantêm-se **sustentar, ressoar e iniciar** para a primeira exposição. A alteração principal é tornar a memória e a reaparição de motivos parte obrigatória dos três. Ressoar distingue incorporação de dados corporais de retorno autónomo de um motivo: este último pode ocorrer ao sustentar/iniciar, mas não conta como contingência atual. Propagar e contrastar continuam possíveis numa expansão; não bloqueiam o piloto nem se anunciam como capacidades existentes.

Os três bastam para construir um teste refutável de continuidade, reciprocidade e iniciativa. A sua suficiência percetiva é H. Se só o investigador os distingue, a implementação técnica terá funcionado, mas a manipulação experimental pode ter falhado. Não exigir que participantes reconheçam três rótulos.

## Realização mínima comum P

Um motivo contém ID, proveniência, relações temporais, curva normalizada `e(t)` entre 0 e 1, duração e referências a observações/planos. O executor gera curvas suaves; `e=0` é contribuição neutra. Biblioteca inicial: uma diminuição longa, duas diminuições separadas e um par com a segunda mais longa. São formas temporais abstratas, não gestos-comandos.

No mínimo, `light_effective = light_base - 0.06 * e(t)` e `gain_master = 1 - 0.12 * e(t)`. Base corrente 0,6; emissão limitada a [0,54; 0,60] enquanto esse valor base estiver congelado; ganho [0,88; 1]. Entrada/saída de 2 s, permanência mínima total de 4 s incluindo transições, duração total por episódio 6–12 s. São valores iniciais de engenharia; não limites percetivos validados nem dose final do piloto. Se forem inaudíveis/invisíveis no headset, ajustar as três condições em desenvolvimento e congelar novos valores. Adaptação desligada dá emissão original e ganho 1.

Não aumentar a energia com a intensidade de movimento. Graduação significa escolher `subtle`/`standard` (metade/envelope completo) ou `preserve`/`retime`; limites finais são locais. No piloto mínimo, amplitude `standard` e duração por oportunidade ficam fixadas pela condição comum. Não há misturas probabilísticas de políticas. Os motivos da biblioteca usados no piloto devem ser equalizados por integral e pico da atenuação por oportunidade; mudar a distribuição temporal sem variar arbitrariamente a dose.

O canal visual é global, não uma região controlada. O canal sonoro altera ganho estéreo do MASTER, não timbre, notas, camadas, fontes ou espacialização. A alternância musical dos anéis permanece própria da coreografia. O mapeamento não usa `pulse_strength` como garantia de alterar os pulsos finais.

## Sustentar — `sustain`

| Campo | Especificação P |
|---|---|
| Função | Estabelecer continuidade; deixar uma consequência amadurecer e proporcionar tempo para perceber uma transformação. Não congelar a coreografia. |
| Contexto necessário | Estado base/efetivo, fase e capacidades, motivo aplicado, tempo de permanência, janelas protegidas, qualidade corporal e histórico de planos. Sem leitura emocional. |
| Visual e som | Concluir suavemente o envelope em curso; conservar estrutura temporal e um vestígio limitado quando permitido. Na travessia coreográfica, contribuição neutra, MASTER original e memória conservada. |
| Estável | Percurso, relógio, material musical, famílias, direção artística, envelope experimental e orçamento de iniciativa. |
| Participação | Pode manter, mudar ou interromper o movimento e encontrar continuidade em vez de uma reação obrigatória a cada gesto. |
| Imobilidade | Continua. Baixa variação válida é observação; não desencadeia penalização, escalada ou procura insistente de resposta. |
| Perda de seguimento | Não inventar continuidade corporal. Motivo já calculado pode terminar; sem novo motivo válido, voltar à base em 2 s. Cabeça inválida/suspensão seguem pausa técnica, não política artística. |
| Memória | Conservar até dois motivos, versões e efeitos realmente aplicados; distinguir memória simbólica de modulação ainda presente. |
| Saída | Passar a ressoar numa oportunidade elegível ou iniciar na agenda autónoma; transição da curva atual, sem salto. No fade final, base neutra e encerramento. |
| Indício H | A relação parece perdurar quando a pessoa para; alternativa: não se percebe diferença em relação à coreografia. |

Exemplo: depois de E1, deixar M1 terminar e guardar a sua organização até E3. Contraexemplo: alterar continuamente cor/ganho com cada amostra e chamar a isso sustentar. Aceitação técnica: nenhum novo plano fora de oportunidades; continuidade numérica nas entradas/saídas; memória conserva apenas efeitos aplicados.

## Ressoar — `resonate`

| Campo | Especificação P |
|---|---|
| Função | Dar continuidade a um episódio incorporando organização temporal observada na relação, sem copiar uma mão ou prometer causalidade percetível. |
| Contexto necessário | Janela corporal válida, cabeça e mãos disponíveis com cobertura separada, variação temporal estável, motivo anterior, estado efetivo/música, planos recentes e limites da oportunidade. Cabeça sozinha pode sustentar elegibilidade; mãos sozinhas não resolvem orientação/referência. |
| Visual e som | Selecionar localmente uma curva de atenuação/recuperação compatível com o padrão observado; na condição opaca, conservar relações amplas, retemporizar e ligar ao motivo anterior. Emissão e ganho partilham curva e relógio. |
| Estável | Dose máxima e duração da oportunidade, geometrias, paletas artísticas, velocidade e relógio musical. Não remodelar a cena para perseguir a pessoa. |
| Participação | Alterar espontaneamente o ritmo de movimentos/pausas ou orientação pode influenciar a continuação. Não existe vocabulário de poses nem gesto necessário. |
| Imobilidade | Transição movimento→repouso válida pode fornecer um padrão. Repouso constante não é um motivo temporal: ressoar fica inelegível; sustentar ou retorno autónomo mantém a experiência. |
| Perda de seguimento | Não usar segmentos inválidos, saltos de origem ou velocidades inferidas através da perda. Usar fontes restantes se elegíveis; não renormalizar amplitude para compensar uma mão ausente. Motivo histórico válido pode retornar, identificado como histórico. |
| Memória | Referenciar observações e plano ancestral; guardar M1', com o que mudou e o que se conservou. Mudança posterior do corpo é consequência observada, sem atribuição de intenção. |
| Saída | Ao fim da oportunidade, sustentar. Se contexto se tornar incompatível antes da aceitação, rejeitar; depois da aplicação, concluir trajetória validada salvo interrupção técnica. Não repetir ilimitadamente para obter movimento. |
| Indício H | Pessoa reconhece influência/recorrência e alguma continuidade de autoria; alternativa: só sincronia musical ou controlo instrumental. |

Exemplo: depois de M1, movimento com duas fases separadas por repouso é resumido numa curva temporal, que reaparece em M1'. Contraexemplo: `probability(resonate)=0.8` produz ganho 0,8 ou um gesto fechado liga um efeito. Aceitação: alterar um registo corporal válido mantendo música/história fixas pode alterar a escolha/temporização autorizada; invalidar esse registo impede nova ressonância; não se declara o que a pessoa sentiu.

## Iniciar — `initiate`

| Campo | Especificação P |
|---|---|
| Função | Abrir um episódio sem exigir movimento desencadeante, preservando uma história que possa ser transformada pela pessoa. |
| Contexto necessário | Agenda/seed, fase artística, estado efetivo, motivo anterior, iniciativas aplicadas, limites e proximidade de transições. Observações corporais informam a continuação, não a quota nem os horários de iniciativa neste piloto. |
| Visual e som | Motivo temporal autorizado de emissão/ganho aos 50, 188 e 316 s; M2/M3 conservam uma relação de M1 ou introduzem uma diferença temporal limitada. Não surgem personagem, objeto dirigido ao olhar ou nova fonte sonora. |
| Estável | Dose e orçamento, coreografia, percurso, relógio, material e direção cromática. A novidade não cresce com silêncio ou atividade da pessoa. |
| Participação | Aceitar a continuidade, alterar o ritmo corporal, orientar-se, ficar quieto; uma oportunidade posterior permite incorporar um padrão válido. |
| Imobilidade | O acontecimento ocorre na agenda comum; prossegue sem resposta. Não se acrescenta «convite» mais forte. |
| Perda de seguimento | A agenda não dispara novas iniciativas por falha; pode executar plano autónomo autorizado se cabeça válida. Estado técnico interrompido prevalece. |
| Memória | Guardar proveniência autónoma, motivo escolhido e janela corporal antes/depois; mesmo sem alteração corporal, preservar o acontecimento para retornos. |
| Saída | Concluir, observar e sustentar; ressoar apenas na oportunidade seguinte se houver motivo elegível. Não esperar reconhecimento para terminar. |
| Indício H | Transformação parece ter origem própria e permanecer disponível à relação; alternativa: surpresa estética ou evento musical programado. |

Exemplo: M2 começa enquanto a pessoa repousa e transforma uma proporção de M1; aos 202 s há continuação limitada. Contraexemplo: qualquer perda de mãos desencadeia mudança aleatória, apresentada como iniciativa. Aceitação: trocar registos corporais mantendo a agenda não muda os três inícios; seed e planos permitem reprodução.

## Memória, interpretação e composição

Manter dois motivos correntes no máximo, três episódios detalhados e resumo factual da sessão. Um motivo pode persistir por 90 s sem reaplicação e ser renovado num retorno válido; curvas aplicadas, ID e ancestralidade ficam arquivados até ao fim da sessão. Um retorno mais antigo reconstrói-se desse arquivo, sem o apresentar como padrão corporal recente. Isto permite atravessar E5 e chegar a E7. Expirar a **elegibilidade de novos padrões corporais** depois de 10 s; não confundir esse prazo com apagar motivos já aplicados. Em reprodução independente, a ancestralidade é da sessão-fonte, não do participante atual.

Acoplamento, opacidade, duração, amplitude, dose, orçamento e janelas protegidas são congelados pela condição experimental. Jev pode escolher entre comportamentos/motivos localmente elegíveis e graduar só dimensões autorizadas. Confiança baixa implica abstenção, não menos luz. Interpretações permanecem num campo separado, sem regressarem como factos ao contexto.

Base artística → modulação local autorizada → limites/interrupção → parâmetros efetivos e registo. Memória escreve-se depois de aplicação confirmada. O [plano técnico](implementation-plan.md) define prazos, filtros, capacidades e sincronização; a [matriz](behavior-matrix.md) reúne os registos necessários.

## Expansão adiada

| Comportamento | Função futura | Dependência e alternativa inicial |
|---|---|---|
| Propagar | Desenvolver um motivo entre regiões estáveis. | Seleção por grupo/região no executor/renderizador e fontes/camadas sonoras verificadas. Hoje só se propõe um retorno global; não o chamar propagação espacial. |
| Contrastar | Introduzir diferença numa dimensão dominante conservando o resto. | Atuadores e material sonoro que permitam controlar contraste/dose. No mínimo, diferença temporal delimitada em iniciar/ressoar; não alteração de timbre inexistente. |

Revisão registada: fichas genéricas substituídas por realização global delimitada, memória obrigatória, imobilidade explícita, elegibilidade corporal e indícios refutáveis. Não se alterou a finalidade de investigação nem se implementaram os ficheiros YAML futuros.
