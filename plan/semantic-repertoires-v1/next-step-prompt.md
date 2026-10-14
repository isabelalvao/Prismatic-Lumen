# Prompt do próximo passo — primeira e única sessão

Prompt preparado na conversa de desenho e conservado para execução posterior. O nome de apresentação é Prismatic Lumen; a pasta e os identificadores técnicos anteriores permanecem Fantasynt.

```text
Trabalha no projeto Prismatic Lumen (anteriormente Fantasynt), em C:\ORCA\Fantasynt.

O objetivo desta tarefa é rever e concretizar o desenho da experiência para que possa ser avaliada por participantes numa primeira e única sessão, sem conhecimento dos mecanismos internos nem comparação prévia com outras versões.

Produz uma especificação de experiência e um plano técnico executável. Nesta tarefa, altera apenas documentação; não implementes ainda alterações no motor.

1. CONTEXTO E FINALIDADE

Prismatic Lumen é um ambiente audiovisual imersivo não figurativo, implementado em C++, SDL3, bgfx e OpenXR, utilizado com Quest 3 através de PCVR.

O artefacto será um campo de ensaio para investigar se um ambiente com comportamento coerente, parcialmente contingente e causalmente opaco pode favorecer:
- A atribuição de agência ao próprio ambiente.
- Uma experiência ambígua entre essa atribuição e o prolongamento da agência do utilizador.

Esta é uma hipótese a investigar, não um efeito que deva ser assumido ou garantido.

A entidade computacional deve fazer uma leitura holística da relação entre utilizador e ambiente. Essa leitura integra expressão corporal, estado audiovisual efetivo, história da interação e consequências das ações anteriores do ambiente.

Restrições confirmadas:
- Não usar controladores.
- Usar seguimento das mãos e da cabeça.
- Não reduzir a interação a um vocabulário de gestos-comandos.
- Não centrar toda a interpretação no seguimento das mãos.
- Voz pode ser acrescentada posteriormente; fisiologia é uma extensão futura.
- Não depender de conversa para a experiência inicial.
- Não inferir emoções ou intenções como factos a partir dos movimentos.
- Não introduzir avatar, personagem ou representação antropomórfica como solução para tornar a agência percetível.
- Distinguir aquisição das mãos da sua representação visual.

Jev é um candidato à decisão semântica, substituível por regras ou outro modelo. A execução audiovisual, os limites e as transições permanecem locais.

2. DOCUMENTAÇÃO E CÓDIGO A CONSULTAR

Lê primeiro:
- plan/semantic-repertoires-v1/implementation-plan.md
- plan/semantic-repertoires-v1/repertoire.md
- plan/semantic-repertoires-v1/notes.md
- plan/semantic-repertoires-v1/task_plan.md
- plan/semantic-repertoires-v1/research-context.md
- README.md
- visual-config.json

Consulta apenas o código necessário para verificar capacidades e dependências, nomeadamente src/control/, src/vr/, src/audio/, src/render/art_direction.cpp, src/render/sequence.cpp e src/app/main.cpp.

Distingue claramente funcionalidade existente e verificada, funcionalidade proposta, hipótese percetiva por testar e decisão experimental aberta.

Confirma a duração e a organização atuais da experiência no código/configuração. Não suponhas que todos os parâmetros aceites pelo protocolo produzem o mesmo efeito em todas as fases.

3. PROBLEMA CENTRAL A RESOLVER

Os comportamentos podem ser distintos para quem conhece o sistema e passar despercebidos a um participante. A coreografia e a música já produzem mudanças contínuas. Uma alteração adaptativa pode parecer apenas mais um acontecimento programado.

Desenha a sessão para que existam oportunidades de perceber relação, continuidade e iniciativa, sem ensinar os mapeamentos nem revelar a lógica causal. O participante não precisa de identificar os nomes dos comportamentos.

Não confundas opacidade causal com ausência de contingência, ambiguidade com aleatoriedade, surpresa audiovisual com agência atribuída, correlação temporal com prova de intenção, complexidade do modelo com qualidade da experiência ou latência de rede com mediação temporal artística.

4. REVER O REPERTÓRIO

O repertório contém sustentar, ressoar, iniciar, propagar e contrastar. O primeiro incremento considera sustentar, ressoar e iniciar. Avalia se estes três comportamentos são suficientes para uma primeira sessão. Podes simplificar ou reformular, justificando cada alteração.

Para cada comportamento, especifica a função, contexto, manifestação visual/sonora, elementos estáveis, oportunidade de participação, tratamento de imobilidade e perda de seguimento, consequência/memória, saída/transição e indício percetivo a testar.

5. DESENHAR UMA SESSÃO AUTOSSUFICIENTE

Propõe organização temporal compatível com a experiência atual. Tempos e intervalos são hipóteses ajustáveis, não valores cientificamente validados.

Inclui familiarização com o equipamento sem treino dos mapeamentos, continuidade audiovisual, oportunidades de reciprocidade, iniciativa do ambiente, persistência/transformação dos motivos e encerramento. Evita uma demonstração escolar de cada comportamento.

Em cada episódio descreve estado inicial, acontecimento do ambiente, possíveis respostas do participante incluindo ausência de resposta, observação e incerteza, decisão da entidade, transformação aplicada e vestígio conservado.

Inclui três percursos: exploração ativa; participante maioritariamente imóvel; movimentos difíceis de interpretar. A experiência deve continuar coerente nos três casos sem classificar imobilidade como desinteresse.

6. FRONTEIRA ENTRE MODELO E EXECUÇÃO

Especifica contexto temporal, decisões autorizadas, variáveis fixadas experimentalmente, continuidade/memória, composição com coreografia, tradução para parâmetros e tratamento de decisões atrasadas/incompatíveis/de baixa confiança.

Jev seleciona ou gradua comportamentos delimitados. Cálculos exatos, temporização, validade dos sensores e limites ficam no código. Não traduzas probabilidades diretamente em intensidades audiovisuais.

Para capacidades inexistentes, identifica módulo a alterar e alternativa inicial realizável. Não declares espacialização sonora ou controlo por regiões quando só há modulação global.

7. AVALIAÇÃO DE PRIMEIRA EXPOSIÇÃO

Propõe um piloto em que cada participante conhece apenas uma condição: contingência transparente, contingência opaca ou reprodução independente com características audiovisuais comparáveis.

Explica como comparar duração, conteúdo, estimulação, familiarização e latência técnica. Identifica diferenças inevitáveis e fatores de confusão. Distingue recorrências dentro da sessão, repetições da sessão pela mesma pessoa e comparações entre participantes.

Começa a recolha pós-sessão por descrição aberta e não sugestiva. Considera depois medidas separadas de agência própria, agência atribuída ao ambiente, controlo, previsibilidade e ambiguidade. Não uses as interpretações do modelo como evidência subjetiva nem otimizas online relatos de agência durante o ensaio confirmatório.

Define evidência favorável, inconclusiva ou contrária, incluindo efeitos apenas após explicação ou repetições, atribuição semelhante na reprodução independente, responsividade sem agência e descrição apenas como coreografia musical.

Não inventes escalas validadas, efeitos ou tamanho de amostra. Consulta fontes primárias quando precisares de fundamentação e explicita limites da transferência para este artefacto.

8. ENTREGÁVEIS

Guarda em plan/semantic-repertoires-v1/: especificação da primeira sessão; matriz comportamento → contexto → decisão → transformação → indício percetivo → registo; protocolo preliminar; lista priorizada de alterações técnicas com dependências e aceitação.

Atualiza documentos anteriores onde necessário, preserva decisões válidas e regista revisões. Evita versões contraditórias.

Fecha com decisões/justificação, riscos por testar, menor implementação para a questão central e primeira tarefa de programação com módulos e conclusão objetiva.

Avança com pressupostos explícitos quando possível. Reserva perguntas para decisões que alterem materialmente a hipótese ou experiência. O resultado deve orientar implementação e manter as hipóteses percetivas abertas à refutação.
```
