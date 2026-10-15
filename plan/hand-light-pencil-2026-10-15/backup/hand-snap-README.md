# Estalo dos dedos — protótipo de deteção e pontos luminosos

Pedido explícito do utilizador em voz, 2026-10-15: começar pela deteção de um estalo e uma pequena explosão discreta de pontos junto à mão, deixando a direção visual mais elaborada para depois. A investigação anterior sobre fluidez foi adiada pelo utilizador; não se alteraram relógio/percurso/áudio nesta tarefa. Sem commit/push.

## Testar no Quest

Na raiz `C:\DEV\Prismatic`, com hand tracking encaminhado para o runtime OpenXR:

```powershell
.\build\Release\fantasynt.exe --config .\visual-config.json --vr --paused --hand-snap
```

O efeito pode ser testado com a música em pausa: manter a mão visível à frente do visor, preparar brevemente o contacto entre polegar e dedo médio e fazer o estalo com recolha rápida do dedo médio em direção à palma. Espaço inicia/pausa a experiência; Esc encerra. Ambas as mãos funcionam independentemente. Não é necessário usar o P1 programado. Sem `--hand-snap`, comportamento anterior preservado; a opção exige `--vr`.

Cria automaticamente um registo novo `logs/hand-snap-<id>.jsonl`, anunciado no arranque. Pode indicar `--record-session` com um caminho novo. Não substitui gravações. A configuração da raiz, incluindo a afinação atual do utilizador `motifs.light_attenuation=0.60` e `motifs.master_attenuation=0.90`, foi preservada; esses valores só atuam com `--motifs-scheduled`.

## O que reconhece e limites

**Aproximação cinemática de um estalo, não confirmação de um estalo físico/sonoro.** Sem microfone, modelo de aprendizagem ou Jev. Uma pose isolada não basta. O runtime pode perder/estimar articulações precisamente quando os dedos se tocam e o movimento pode ocorrer entre duas amostras; por isso, a fiabilidade real só se avalia no Quest e não é garantida por esta implementação.

O detetor usa todas as amostras XR dos frames disponíveis, não a publicação de observações a 20 Hz. Exige mão ativa e posições finitas, válidas e seguidas de palma, pulso, polegar e articulações proximal/distal/ponta do médio. Tem preparação por contacto polegar–médio e confirmação por separação, recolha rápida da ponta **e** recolha da articulação distal. As distâncias são relativas à palma; mover a mão inteira não fornece o gesto.

| Regra inicial de engenharia | Valor |
|---|---|
| Escala da mão | distância pulso–proximal do médio / 8 cm, limitada a 0,65–1,4; distância anatómica admissível 3,5–16 cm |
| Contacto polegar–médio | até 2,4 cm × escala, durante pelo menos 45 ms |
| Separação para disparo | pelo menos 3,5 cm × escala |
| Recolha da ponta / distal para a palma | pelo menos 1,8 cm / 1 cm × escala |
| Velocidade média de recolha da ponta | pelo menos 0,30 m/s × escala, desde o último contacto |
| Prazo desde último contacto | até 180 ms |
| Intervalo entre disparos por mão | 550 ms |
| Descontinuidade XR | acima de 80 ms, tempo repetido/regressivo, perda/erro ou mudança de origem reiniciam preparação |

Estes valores estão no detetor, versão `thumb-middle-release-1`; não foram calibrados com movimentos do utilizador nem com o JSONL histórico. Contacto mantido, separação sem recolha, dobra lenta e movimento rígido não disparam nos testes sintéticos. Pinças rápidas com organização semelhante podem produzir falsos positivos; estalos com perda de seguimento podem não ser detetados. Não se infere intenção ou estado emocional.

## Efeito inicial

14 pontos dourados claros por disparo, representados por pequenos octaedros emissivos e aditivos, durante 0,45 s, com expansão radial até cerca de 14 cm e redução suave de brilho/tamanho. Um efeito ativo por mão, no máximo dois; sem acumulação ilimitada. Reutiliza o shader das mãos e o bloom existente, sem alterar a composição global. Profundidade da cena preservada; sem reflexos/fog próprios e com fade final soberano.

Os pontos nascem na posição LOCAL da ponta do dedo médio no instante detetado. Expandem-se a partir daí, não seguem a mão nem recebem o avanço virtual da cena. Ambos os olhos e o espelho usam o mesmo efeito físico. Usam o tempo XR, pelo que continuam durante a pausa musical. Perda das articulações necessárias, frame indisponível, cabeça inválida ou mudança de origem retiram efeitos pendentes/ativos em vez de conservar uma mão desaparecida.

Manifesto identifica opção e versão; eventos `hand_snap` registam mão, tempo XR/receção, origem, posição física, recolha/velocidade que satisfizeram a regra e tempo da cena. Continua a gravação assíncrona P0 com integridade explícita.

## Verificação e evidência

[verification.json](verification.json) contém resultados finais e hashes: **21/21 CTest**, **teste gráfico separado** e **88/88 testes Python** aprovados. Logs em [ctest.log](ctest.log), [renderer.log](renderer.log) e [python-tests.log](python-tests.log). Capturas sintéticas finais com/sem efeito em `temp/hand-snap-final-7686a71b/`; a primeira comparação revista visualmente está em `temp/hand-snap-f334d20b/`. Os dois registos finais passam o inspetor de integridade P0. Não foram substituídas capturas históricas. Cópias delimitadas em `backup/`; [estado Git inicial](git-status-before.txt).

Testes C++ cobrem contacto/libertação, duas mãos, disparo único, pose mantida, translação da mão, separação sem dobra, confirmação distal, preparação curta, dobra lenta, perda de uma mão, falta de seguimento, NaN, mudança de origem, lacuna e regressão temporal. Geometria limitada, posições/índices finitos, conversão espacial e expiração também verificadas.

Integração com runtime simulado verifica eventos e diferença de imagem D3D11 durante a pausa, igualdade com a base após expiração, ausência de eventos numa pose mantida/perda/sem extensão, ativação explícita e exigência de VR. Os testes P1 passaram a usar uma cópia temporária com a dose de referência, para não dependerem da afinação deliberada do utilizador na raiz.

**Falta o ensaio físico:** confirmar quantos estalos reais são reconhecidos, pinças/recolhas que disparam indevidamente, posição dos pontos e dose visual. Relatar falhas com a mão usada e o registo do ensaio; não apresentar simulação como validação do gesto real.
