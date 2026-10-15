# Repertório de comportamentos — proposta v1

Data: 2026-10-14. Especificação para implementação; nenhum destes comportamentos está integrado como política semântica no motor atual.

## 1. Unidade de desenho

Um comportamento é uma transformação audiovisual com ciclo de vida, memória e condições de aplicação. Não é um preset emocional nem uma classificação do participante. A entidade escolhe como continuar a relação; o motor calcula a realização concreta.

O contexto inclui corpo, estado efetivo do ambiente, história da interação, capacidades disponíveis e condição experimental. A ausência de voz, fisiologia ou mãos visíveis é informação de disponibilidade, nunca uma medida de desinteresse ou imobilidade.

## 2. Cinco comportamentos

| ID e nome | Conduta | Realização visual proposta | Realização sonora proposta | Dependências |
|---|---|---|---|---|
| `sustain` — Sustentar | Prolongar a organização corrente, preservando continuidade e uma evolução limitada. | Manter estrutura e trajetória cromática; variar lentamente emissão dentro de uma faixa. | Preservar textura e continuidade; pequenas curvas de ganho. | Controlos globais e ganho suave. |
| `resonate` — Ressoar | Incorporar um padrão recente da interação sem copiar literalmente a mão ou a cabeça. | Retomar um motivo temporal observado, distribuindo-o pela emissão/pulsação. | Retomar o mesmo motivo com profundidade moderada na modulação sonora. | Descritores temporais locais, extração de motivo, envolventes coordenadas. |
| `initiate` — Iniciar | Introduzir um acontecimento cuja origem é o estado interno do ambiente, abrindo um novo episódio. | Fazer surgir uma variação de presença, cor ou densidade que não exige um gesto simultâneo. | Introduzir uma variação de textura ou um acento permitido pelo material sonoro. | Agenda autónoma local, semente registada, início/fim definidos. |
| `propagate` — Propagar | Desenvolver uma transformação de uma região para outras, preservando um motivo. | Progressão por grupos estáveis de elementos, com envolventes desfasadas. | Progressão entre fontes/camadas; espacialização apenas quando houver suporte real. | Seleção de regiões/grupos, áudio com fontes ou camadas; não disponível no contrato v1. |
| `contrast` — Contrastar | Introduzir uma diferença limitada em relação ao padrão estabelecido, mantendo uma continuidade identificável. | Alterar uma dimensão dominante, como distribuição ou densidade, conservando as restantes. | Alterar a textura ou articulação com ganho global aproximadamente controlado. | Composição de parâmetros, material sonoro apropriado e distinção percetiva validada. |

Sustentar não é silêncio total; iniciar não é ruído aleatório; contrastar não implica hostilidade ou uma inferência sobre a pessoa. Propagar pode desenvolver um motivo autónomo ou contingente, conforme a condição experimental. Os nomes são internos e não são apresentados aos participantes como descrições de uma entidade intencional.

Primeiro incremento: sustentar, ressoar e iniciar com controlo global. Propagar e contrastar entram depois de validada a distinção entre os três primeiros e de existirem os atuadores necessários. A implementação não deve simular propagação espacial com uma mudança global e declará-la concluída.

## 3. Ficha obrigatória de cada comportamento

Cada definição versionada deve conter:

- Identificador, descrição operacional e capacidades obrigatórias.
- Fontes de contexto utilizadas e dados que não pode inferir.
- Condições de entrada, permanência, saída e interrupção.
- Parâmetros que controla e parâmetros que preserva.
- Curvas de entrada, desenvolvimento e saída; duração mínima/máxima.
- Fonte do motivo temporal, quando existe, e tratamento de dados em falta.
- Mapeamento audiovisual por cena e fase; valores e unidades físicas no executor.
- Limites, frequência máxima de mudança, combinações compatíveis e política de expiração.
- Exemplo de episódio, contraexemplo e testes de aceitação.

As fichas propostas deverão ficar em `config/repertoires/v1/*.yaml` na implementação, com validação de esquema. Esses ficheiros ainda não foram criados.

## 4. Dimensões ortogonais

O ID seleciona a conduta principal. As dimensões abaixo graduam a execução, sem multiplicar dezenas de categorias:

| Dimensão | Significado operacional |
|---|---|
| Amplitude | Desvio permitido em relação ao estado base, com limites por atuador. |
| Continuidade | Quantidade de organização anterior que é preservada. |
| Extensão espacial | Localizada, entre regiões ou global, condicionada pelas capacidades. |
| Persistência | Quanto tempo o motivo continua a influenciar episódios posteriores. |
| Acoplamento | Quanto a evolução depende dos sinais válidos do participante. |

Acoplamento, opacidade, orçamento de iniciativa e envelope de estimulação podem ser fixados pela condição experimental. Nessa situação, Jev não pode alterá-los. A confiança do classificador não é amplitude, acoplamento, opacidade ou probabilidade de a hipótese científica estar correta.

Não misturar livremente os cinco comportamentos pelas probabilidades de classificação: uma média entre políticas incompatíveis pode não produzir um comportamento coerente. Na v1 existe um comportamento principal, com mistura limitada apenas durante transições aprovadas.

## 5. Contexto holístico

Construir `ContextSnapshot` com:

1. **Observações corporais:** características calculadas de cabeça e mãos; referência espacial, tempos, validade e cobertura por modalidade.
2. **Estado audiovisual efetivo:** cena/fase, relógio musical, parâmetros base e após composição, comportamentos ativos e capacidades.
3. **Episódios anteriores:** acontecimentos do ambiente e mudanças observadas a seguir, sem converter sucessão temporal em causalidade demonstrada.
4. **Memória da entidade:** motivo corrente, comportamento anterior, tempo de permanência, iniciativas recentes e consequências registadas.
5. **Expressão explícita opcional:** transcrição ou descritores vocais, se a captura for ativada; fisiologia futura com a mesma distinção de disponibilidade.
6. **Condição experimental:** restrições imutáveis do bloco e versão do repertório.

Guardar observações e hipóteses em campos separados. Uma interpretação passada do modelo não se torna um facto observado nas chamadas seguintes. A ordem, o tempo dos acontecimentos e as limitações dos sensores fazem parte do contexto.

O motor calcula velocidades, amplitudes, estatísticas, durações e relações espaciais. Para Jev enviam-se valores úteis e descrições ancoradas em critérios explícitos; não se delegam operações numéricas ao classificador. A normalização por participante é calibrada antes do bloco e fica congelada durante comparações, salvo se a adaptação da normalização for uma variável planeada.

## 6. Decisão e execução

`BehaviorDecision` contém comportamento candidato, referência ao contexto, distribuição/indicador de confiança quando disponível e versão do decisor. Os identificadores de sessão, validade temporal e versões autorizadas são atribuídos ou verificados pela aplicação.

`BehaviorPlan` é produzido localmente e contém instância única, início agendado, fim/expiração, amplitude, persistência, seleção espacial, motivo e parâmetros preservados. Só são permitidos comportamentos que o motor anuncia como disponíveis.

Ciclo de vida: elegível → agendado → entrada → desenvolvimento → saída → concluído. Substituição, cancelamento, rejeição e expiração são eventos distintos e registados. A perda de seguimento ou uma falha de rede é um estado técnico, nunca um novo comportamento artístico.

Políticas técnicas propostas:

- Sem dados suficientes: manter temporariamente a trajetória válida e sair gradualmente para a base; registar o motivo e invalidar segmentos quando necessário.
- Resposta remota ultrapassada: rejeitar por contexto/instância/versão, sem interromper a renderização.
- Falha remota: não inventar interpretação; continuar apenas a trajetória local já autorizada e limitada no tempo.
- Interrupção pelo participante ou operador: prevalece sobre políticas; controlo local sem LLM e sem controladores Quest.
- Pausa/reinício: suspender o relógio dos comportamentos com a sessão, invalidar respostas pendentes e impedir aplicação de uma decisão do episódio anterior.

## 7. Exemplo de episódio

O ambiente inicia uma transformação discreta. Registam-se a região, a envolvente e o estado sonoro efetivos. O participante orienta a cabeça nessa direção, muda a configuração das mãos e permanece imóvel. O contexto preserva esta sequência e os indicadores de seguimento.

Jev escolhe ressoar, segundo uma rubrica definida para prolongar padrões de interação observáveis. O executor recupera um motivo temporal calculado localmente e aplica uma transformação global limitada, preservando a velocidade de deslocação e o material musical. A resposta é registada; não se declara que houve atenção, compreensão, intenção ou atribuição de agência.

Se o episódio não contiver um motivo válido, ressoar é inelegível. Se a condição for reprodução independente, o mesmo registo audiovisual é executado sem usar os sinais atuais do participante para decisões, mantendo apenas a interrupção técnica.

## 8. Critério de conclusão do repertório

Cada comportamento deve ter execução reproduzível a partir de registos, efeito audiovisual verificável, duração limitada e um contraste percetivo a testar. Uma classificação correta segundo a rubrica não valida a experiência subjetiva de agência. As medidas dessa experiência são recolhidas separadamente e não são usadas como alvo de otimização durante o ensaio confirmatório.
