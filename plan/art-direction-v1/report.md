# Direção artística v1 — versão 0.6.0

O utilizador passou a dirigir o resultado visual através de quatro screenshots anotados. As referências foram preservadas em `references/director-1.png` a `director-4.png`. A entrada das pirâmides aos 80 segundos foi explicitamente confirmada antes da implementação. O perfil aplica-se a Opening, a experiência predefinida de 120 segundos; N/L/T/F continuam a selecionar cenas anteriores para comparação.

## Orientações implementadas

| Orientação | Comportamento preparado |
| --- | --- |
| Pontos a cair lentamente, em ciclo sem salto | Pontos pequenos da cena descem a 10 cm/s num intervalo de 60 m, período de 600 s. Apagam-se suavemente junto aos extremos antes de reaparecer. Os pontos atmosféricos descem lentamente com orientação presa ao mundo; índices periódicos preservam continuidade vertical e ao olhar para trás em VR. |
| Formas arredondadas só visíveis ao pulsar | As malhas identificadas como Pulse não desenham corpo nem reflexo quando o pulso é nulo. A luz branca/fria acende durante os transientes graves do WAV. Laranja é a cor das setas, não uma nova cor dos objetos. |
| Avanço aos 10 s, um pouco mais lento | Câmara estacionária até aos 10 s; aceleração suave de dois segundos até 150 cm/s, 25% abaixo dos 200 cm/s anteriores. O seguimento da cabeça permanece livre. |
| Nenhum obstáculo no percurso | Corredor central com 4 m de largura. Foram deslocadas lateralmente 2 549 instâncias, considerando volumes e a rotação completa das pirâmides, para libertar o percurso à altura do observador. |
| Tracejados a subir/descer lentamente | Máscaras luminosas deslocam-se verticalmente a 10 cm/s; as colunas alternam o sentido e conservam o suporte geométrico. O movimento partilha o relógio da experiência. |
| Pirâmides surgem por acendimento, sem subir | Entram aos 80 s. As linhas acendem progressivamente até aos 88 s; a altura dos vértices e da base não é animada. Rotação própria a 0,25 graus/s sobre o eixo vertical, em sentidos alternados, em torno do centro da base. |
| Começo mais minimal | Primeiro surgem poucos pontos, com brilho baixo. As outras famílias começam a entrar depois dos 10 s e a presença cresce gradualmente até aos 60 s. As pirâmides mantêm a entrada aos 80 s. |

A antecipação do avanço deixava a introdução vazia nas primeiras capturas. Intro/Dome foram prolongados em cinco setores separados por 32 m, com fases de presença diferentes. A base serializada conserva 42 458 instâncias/55 malhas; o runtime prepara 51 414 instâncias. As posições deslocadas e extensões pertencem à direção artística desta versão, e não são apresentadas como recuperação do comportamento original.

## Pulsos e relógio

`BeatEnvelope` analisa o PCM escolhido, filtra a banda grave 35–120 Hz, identifica aumentos locais de energia e prepara envelopes com ataque de 25 ms e cauda até 360 ms. Não utiliza uma pulsação periódica fixa. No MASTER foram detetados 733 transientes no ficheiro completo. Trata-se de deteção automática; a correspondência musical pode ser afinada pelo diretor visual depois de ouvir a versão.

Áudio e animações usam o consumo de amostras SDL como relógio. Pausa e saltos no tempo conservam a fase dos pulsos e movimentos. `--audio` seleciona também a fonte da análise. `--silent`/`--fixed-time` mantêm os pulsos do WAV no instante escolhido sem reproduzir áudio. A latência física do dispositivo e da transmissão Virtual Desktop continua sem compensação.

`motion_speed=0,3` mantém o percurso aprovado; zero imobiliza a câmara sem parar a música. Valores diferentes multiplicam o deslocamento mantendo continuidade. Opening para aos 120 s. Sequence continua depois dos 120 s a partir da posição alcançada, usando as curvas anteriores com esse deslocamento. Os materiais/animações das cenas antigas mantêm os seus comportamentos anteriores.

## Validação e capturas

- Compilação Release: `verification.log`, nove testes C++ e 35 Python aprovados.
- Testes de direção: avanço/arranque, alturas das partículas, período de 600 s, preservação da altura das pirâmides e ausência de interseção com o corredor durante a rotação.
- Deteção áudio: duas batidas sintéticas de 65 Hz detetadas; silêncio e transiente agudo de 6 kHz não produzem pulsos. Pulsos terminam entre batidas.
- Renderização D3D11: pulsação/paleta/densidade continuam controláveis; entre batidas, desligar `pulse_strength` não altera os pixels. O teste entre batidas foi incluído na compilação final.
- Revisão independente: rotação, normais, culling, ciclo dos pontos, análise áudio, limite de buffers, recursos e pacote. Costura do céu em VR e quantização temporal do GIF corrigidas.
- ZIP extraído: hashes válidos, monitor, imagens estéreo OpenXR simulado e cliente JSON PowerShell aprovados (`package-check.json`). A nova versão ainda não foi avaliada no headset físico pelo utilizador.

Capturas finais em `captures/final-0.png`, `final-12.png`, `final-40.125.png`, `final-90.125.png` e `final-119.125.png`. Os registos adjacentes conservam tempo, câmara, intensidade do pulso e diagnóstico.

`intro-motion.gif` mostra 34–46 s com câmara fixa, para isolar pontos, tracejados e pulsação. `pyramid-lighting.gif` mostra 78–90 s, também com câmara fixa, para observar o acendimento e a rotação. São capturas reais do renderer, sem áudio, reproduzidas uma vez. Cada excerto tem 12 segundos; a alternância de durações GIF de 80/90 ms conserva a velocidade média de 12 fps na introdução. A avaliação com áudio e percurso ativo faz-se no executável.

## Entrega e preservação

Pacote: `dist/Fantasynt-PCVR-Win11-x64-0.6.0.zip`, com shaders, recursos, MASTER, loader OpenXR e bibliotecas de execução. Resolução predefinida 1,0. Em VR, Espaço inicia a experiência; os dez segundos de espera contam no relógio da experiência, depois de iniciar a reprodução.

SHA-256: `682b09e0b2b2df93f20f47f3226041c2d61d61c890b2eeee74db8d9079ea3b7c`.

Os ZIPs anteriores foram preservados. Cópias do código anterior em `backup/`. As fontes locais do jogo/UE 4.13, áudio original e tabelas recuperadas não foram modificadas. Não foi inicializado Git. A avaliação técnica não substitui a aprovação artística do utilizador.
