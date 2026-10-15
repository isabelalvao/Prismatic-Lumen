# Afinação do lápis e movimento musical dos anéis

15 de outubro de 2026. Pedido após ensaio real: o lápis nem sempre arranca e fica instável; o deslocamento dos anéis finais deve acompanhar o crescendo e abrandar na coda. O relato invalida qualquer leitura da verificação sintética anterior como aprovação física do lápis. Implementação anterior preservada em `backup/`; sem commit/push.

## Testar esta compilação

Executável de desenvolvimento: `C:\DEV\Prismatic\build\Release\fantasynt.exe`. A configuração afinada da raiz mantém-se intacta. Na raiz do projeto:

```powershell
.\build\Release\fantasynt.exe --config .\visual-config.json --vr --paused --hand-snap --hand-light-pencil
```

Indicador esticado e restantes dedos recolhidos de forma natural; não é necessário fechar um punho apertado nem encostar o polegar à palma. O lápis funciona com a música em pausa. Verificar arranque repetido de cada mão, escrita lenta/rápida, mão quieta, levantar outro dedo, esconder a ponta e recuperar. Levantar outro dedo claramente ou recolher o indicador termina o segmento; perda do indicador apaga imediatamente o rasto. O estalo mantém a ampliação já aprovada para ensaio.

Para entrar diretamente antes do alinhamento dos anéis, com música:

```powershell
.\build\Release\fantasynt.exe --config .\visual-config.json --vr --start-time 298 --play --hand-snap --hand-light-pencil
```

Para rever só o abrandamento, substituir `298` por `390`. Espaço pausa/retoma a música e o movimento do túnel; o lápis continua a responder às mãos. Esc termina. A ativação dos efeitos cria automaticamente um JSONL com nome novo em `logs/`; não reutilizar manualmente um nome de gravação existente.

## Lápis v2

O registo real já disponível `logs/hand-effects-5371f2cbbde89409dae7a57368ad6533.jsonl` contém 76 inícios, 72 fins por `pose_released`, três resets por salto de ponta e um por mão inutilizável. Dezassete segmentos duram menos de 100 ms. Resumo em [real-before-summary.json](real-before-summary.json). É compatível com o relato de escrita fragmentada, mas sem referência física não se atribui cada fim a oscilação ou a um gesto intencional. Os limiares novos não foram calibrados com esse registo; as observações publicadas também não reproduzem a cadência completa do detetor XR.

`index-only-trail-2` separa qualidade da posição de qualidade da pose. Palma, pulso, base/PIP/ponta do indicador continuam a exigir posições finitas, válidas e seguidas em cada frame. A DIP do indicador deixa de ser uma dependência redundante; a direção da emissão usa PIP–ponta, com uma base mais longa. Cabeça/origem/tempo conservam as validações anteriores.

A pose aceita flexão parcial dos restantes dedos e polegar relaxado junto à mão. Há limiares diferentes para entrar e manter a escrita. Duração mínima de entrada reduzida de 60 para 40 ms; uma breve leitura incerta dos dedos auxiliares pode ser atravessada na candidatura por até 80 ms, mas só uma pose confirmada pode iniciar. Durante escrita confirmada, a incerteza auxiliar tem uma tolerância máxima de 120 ms. Não se interpreta um dedo ausente como recolhido e não se inicia apenas com dados ausentes. Outro dedo claramente esticado interrompe imediatamente. Perda da posição essencial continua a limpar o rasto, sem extrapolar nem ligar através da perda.

Filtro exponencial adaptativo: constante entre 8 e 35 ms, mais amortecimento em repouso e menos atraso durante movimento rápido. Amostragem espacial mínima de 2 mm em vez de 3 mm. São parâmetros do algoritmo, não medições de latência física. Mantêm-se cor, espessura, duração de 0,65 s, máximo de 128 pontos por mão, localização física LOCAL, segmentos separados, limites de saltos/tempo, bloom e fade final. Eventos de fim distinguem `pose_uncertain_timeout` de `pose_released`; o manifesto regista versão e tolerâncias.

## Deslocamento dos anéis

Perfil `master-crescendo-coda-1`, apenas na extensão dirigida de Opening. A formação/alinhamento conserva a trajetória anterior. Após o fim do alinhamento e início dos pulsos (5:06 na configuração atual), a velocidade relativa dos anéis aumenta suavemente de 1× a 3× em 28 s. Mantém 3× até aproximadamente 6:32,18; desce suavemente até 0,25× em 26 s, aproximadamente 6:58,18. A 1,5 m/s de base, corresponde a 1,5 → 4,5 → 0,375 m/s com os controlos de movimento normais.

Curvas smoothstep integradas analiticamente geram a distância adicional. Não se multiplica uma posição acumulada por uma velocidade: não há salto ao mudar de rampa, dependência da cadência de desenho ou deriva por pausa. Arranque diretamente numa secção e reinício reconstituem a posição. A velocidade de referência é capturada ao formar o túnel, tal como a origem existente. Os tempos seguem os campos de alinhamento/pulso/duração de Opening; em durações curtas, as rampas comprimem-se sem sobreposição.

É uma direção artística programada para acompanhar o crescendo/coda reportados, não um estimador de BPM. A análise existente de ataques graves do MASTER foi executada numa ferramenta temporária sem editar o áudio: mediana de cerca de 0,5 s entre ataques nas janelas de oito segundos entre 290 e 410 s, com lacunas; sem ataques depois de cerca de 415 s. Isto não demonstra uma aceleração de tempo musical nem valida o encaixe percetivo da curva. Esse encaixe deve ser ouvido e visto no Quest.

Só se altera a translação longitudinal dos anéis; mantêm-se espaçamento, raio, alinhamento, pulsos/cores, artefactos restantes, câmara/percurso, relógio/pitch, MASTER, composição, P1 e fade final. Com a afinação atual ainda restam anéis à frente até ao fim. A cadência visual continua sujeita ao relógio audiovisual existente; esta alteração não resolve a investigação de fluidez que o utilizador adiou.

O manifesto identifica o perfil e as observações/resultados incluem `ring_travel_speed_multiplier` (multiplicador da curva de referência; 1 fora de Opening dirigido). A alteração dos anéis não exige uma opção adicional. O lápis mantém ativação explícita por `--hand-light-pencil`.

## Verificação

Resultados, hash do executável e preservação de recursos em `verification.json`. Cópias delimitadas em `backup/`, estado inicial em `git-status-before.txt`. Capturas novas e ferramentas de análise ficam em `temp/`, sem substituir imagens históricas. Revisão específica em `review.md`.

Release compilada: **22/22 CTest**, **teste gráfico separado** e **96/96 testes Python** aprovados. Quatro capturas em `temp/pencil-tunnel-a9e304d5/` revistas visualmente; os respetivos registos passam o inspetor de integridade. SHA-256 do executável: `842da716f31fbeb2f18c2794173afde6cb828e46398161e43ed619bf8398446e`. Hashes do MASTER e configuração da raiz iguais ao início. Avisos de compilação nas dependências e APIs já usadas (`getenv`/`u8path`), sem erros. Não foram executados testes físicos no Quest.

Testes simulados verificam poses naturais, perda breve de articulações auxiliares, ausência de falsa ativação por dados ausentes, limiares de entrada/manutenção, interrupções explícitas/perda real do indicador, redução de oscilação, geometria e regressões do estalo. Testes dos anéis verificam continuidade/derivada da distância, aceleração/coda, estado estável em pausa, geometria restante intacta e sequência antiga sem alteração. A fiabilidade gestual, aspeto visual, conforto do movimento e coincidência musical continuam dependentes de ensaio real.
