# Prismatic Lumen

**Prismatic Lumen** é um ambiente audiovisual imersivo não figurativo e um artefacto de investigação sobre a atribuição de agência ao ambiente e a ambiguidade entre essa atribuição e o prolongamento da agência do utilizador. O nome anterior do projeto é **Fantasynt**, conservado nos identificadores técnicos, executáveis e registos históricos.

A base atual é um motor C++ para Windows/PCVR, com SDL3, bgfx e OpenXR. A infraestrutura semântica, a integração com Jev/LLM e o seguimento das mãos pertencem à fase planeada; não são apresentados como funcionalidades já implementadas. A leitura futura integra expressão corporal, estado audiovisual e história da interação. Não serão utilizados controladores Quest nessa fase.

Repositório: [JoaoAz/Prismatic-Lumen](https://github.com/JoaoAz/Prismatic-Lumen). Consulta o [índice de toda a documentação](docs/INDEX.md), o [plano dos repertórios](plan/semantic-repertoires-v1/implementation-plan.md) e o [contexto de investigação](plan/semantic-repertoires-v1/research-context.md).

## Obter uma cópia completa

As capturas, vídeos, áudio e evidências binárias são conservados com Git LFS. Instala Git e Git LFS e executa:

```powershell
git lfs install
git clone https://github.com/JoaoAz/Prismatic-Lumen.git
cd Prismatic-Lumen
git lfs pull
```

Um repositório privado exige autenticação com uma conta autorizada. A utilização de Git LFS é necessária para obter o conteúdo real dos recursos; ficheiros ponteiro não bastam para compilar ou executar a experiência. As compilações, caches e dependências descarregadas não são versionadas; os procedimentos seguintes recompõem o ambiente. Os pacotes portáteis disponibilizados ficam na secção Releases.

## Implementação atual e histórico

Ambiente audiovisual C++ controlado por JSON. A versão **0.12.0** permite afinar a direção artística num ficheiro editável, **sem recompilar**, com releitura automática ao guardar e F5 para forçar a recarga. Movimento, animações e presença luminosa começam sem espera; a experiência inicia automaticamente. Opening acompanha o MASTER completo até **7:04,18**, com fade para preto nos últimos 16 segundos, corredor livre, cones construídos peça a peça e resolução VR 1,0.

O contador segue o relógio da música, arranca desligado e C mostra/oculta; congela em pausa e volta a zero com Q. Em VR usa uma placa comum a dois metros da cabeça, projetada por olho. Os mini-pilares sem sapata deslocam-se rigidamente, com uma haste subterrânea fixa que mantém o contacto com o piso; os restantes pilares conservam a oscilação lenta. A fase colorida conserva cerca de 25% da geometria, ajustável em `extension.geometry_density_multiplier`. Formas sem material emissivo acendem linhas entre 104–116 s. As peças anteriores saem entre 208–224 s e o campo de artefactos surge entre 224–272 s. Aos 290 s começa o alinhamento e crescimento dos anéis, com dispersão individual dos restantes objetos; aos 306 s restam os anéis axiais. O brilho rosa do fundo acende entre 294–306 s. Tempos, cores e brilhos ficam no JSON.

Os cones têm base larga e ponta para cima; constroem-se em 17 peças: oito nervuras independentes primeiro, depois nove círculos da base à ponta, com entradas alinhadas aos ataques graves detetados no MASTER. A distribuição inclui 224 cones (+40%), espaçados e com três combinações: ciano/magenta, azul/branco e violeta/dourado. Os cristais são agora pequenas rochas facetadas de 25–70 cm, com proporções uniformes, bases no chão e afastamento dos cones. O campo de artefactos conserva cerca de 30% da densidade anterior; cada peça roda e deriva com fase e velocidade próprias. Depois da formação do túnel, ataques graves sucessivos acendem alternadamente anéis pares em ciano e ímpares em rosa. As entradas das peças usam construção ou opacidade; os anéis do túnel crescem durante o alinhamento. A transparência é ordenada globalmente por olho, depois do piso e das malhas opacas. O ambiente conserva a tonalidade quente, com saturação vermelha 0,32, névoa luminosa 10% e nevoeiro 28%, mantendo os reflexos. Triângulos sólidos e plataformas são retirados. A geometria permanece 60 m atrás do utilizador e dissolve-se nos 30 m seguintes; as distâncias ficam em `layout.retain_behind_m` e `layout.behind_fade_m`.

O utilizador dirige o resultado visual através de capturas anotadas e da afinação dos parâmetros. A implementação usa as 55 malhas recuperadas como base, prolonga Intro ao longo do percurso e desloca lateralmente obstáculos. O JSON controla velocidades, tempos, cores, densidades, materiais, reação aos graves, câmara, disposição, névoa, reflexos e pós-processamento. As versões anteriores continuam preservadas.

Na extensão, a paleta artística substitui gradualmente `parameters.palette`; afina as cores dessa fase em `extension.warm_first_colour/warm_second_colour` e `extension.cool_first_colour/cool_second_colour`.

O renderer usa SDL3, bgfx e Direct3D 11. O contrato JSON de rede mantém a versão 1. Para afinar a experiência, consulta o [guia completo do ficheiro](assets/visual-config-guide.md). A configuração do projeto fica em [visual-config.json](visual-config.json), usada por Fantasynt.cmd. No pacote portátil, o ficheiro editável fica junto de fantasynt.exe.

## Compilar

Requisitos: Windows x64, Visual Studio Build Tools 2022 ou mais recente com ferramentas C++, Windows SDK, CMake 3.24 ou mais recente e Git. Python 3 é necessário para o cliente manual e os testes de integração. A primeira configuração descarrega as dependências fixadas pelos commits em `cmake/`.

No PowerShell, dentro de `C:\ORCA\Fantasynt`:

```powershell
.\tools\build.cmd
```

O wrapper usa a política de execução apenas nesse processo PowerShell, sem alterar a configuração permanente do sistema. Para compilar e testar:

```powershell
.\tools\build.cmd -Test
```

Para testar o núcleo sem compilar bibliotecas gráficas:

```powershell
.\tools\build.cmd -CoreOnly -Test
```

O executável fica em `build/Release/fantasynt.exe`. Mantém as pastas `shaders`, `examples`, `audio` e `opening` ao lado do executável. A máquina de desenvolvimento usa Build Tools 2026, MSVC 14.51 e CMake 4.3.1.

## Executar e controlar

O pacote portátil para Windows 11 x64 está em `dist/Fantasynt-PCVR-Win11-x64-0.12.0.zip`. Extrai-o inteiro e abre `Launch-VR.cmd` para VR ou `Launch-Desktop.cmd` para monitor. Confirma **0.12.0** no título da janela e mantém a cena **Opening** (tecla O); N/L/T/F selecionam vistas manuais. Inclui shaders, recursos de Opening, áudio, exemplos, loader OpenXR e DLLs Visual C++; o PC de destino não precisa de compilador nem Python. O runtime simulado dos testes fica fora do pacote. Os ZIPs anteriores foram conservados.

Para **Quest 3 via Virtual Desktop**, seleciona **VirtualDesktopXR/VDXR** nas definições do Virtual Desktop Streamer, liga o headset ao PC e inicia o lançador a partir do ambiente de trabalho remoto. Confirma `Runtime: VDXR` no overlay do Virtual Desktop. Este percurso suporta aplicações OpenXR sem SteamVR, segundo a [documentação VDXR](https://github.com/mbucchia/VirtualDesktop-OpenXR/wiki).

O modo VR usa tracking previsto, projeções independentes por olho e imagens D3D11 do runtime. A experiência começa automaticamente; Espaço pausa/retoma. startup.auto_play=false ou --paused permitem começar em pausa. Os controladores Quest ainda não estão mapeados. O teclado atua quando a janela do programa tem foco. Para ajustar a resolução, executa `Launch-VR.cmd --vr-scale 0.5` ou `--vr-scale 1.0`; o valor inicial é 1,0. `Diagnose-VR.cmd` verifica runtime e disponibilidade do headset e guarda o resultado em `logs/xr-probe.log`.

Na árvore de desenvolvimento, o equivalente é `build/Release/fantasynt.exe --vr`. `--xr-probe` faz apenas o diagnóstico. O controlo JSON continua disponível; o pacote inclui `Send-Parameters.cmd examples/calm.json`, que usa PowerShell e não requer Python.

**Validação VR:** testes locais com runtime simulado e leitura das duas imagens na GPU; funcionamento da versão 0.2.0 confirmado pelo utilizador no Quest 3/Virtual Desktop. As notas posteriores registam a confirmação 0.3.0 em VR. As cenas e transições 0.4.2 foram testadas com o runtime simulado e continuam por avaliar no headset. Relatório: [validação PCVR](plan/2026-10-12-quest3-vr-validation.md).

Abre `Fantasynt.cmd`, ou executa:

```powershell
.\build\Release\fantasynt.exe
```

Teclas: **1** calmo, **2** equilibrado, **3** energético, **C** mostra/oculta o contador MM:SS.d, **F5** relê o ficheiro visual, **Q** reinicia a sequência selecionada, **O** Opening, **N** campo noturno, **L** paisagem colorida, **T** túnel, **F** campo anterior, **Space** pausa/retoma a sequência e a câmara, **R** repõe o estado equilibrado, **Esc** sai. A pausa mantém a rede e as transições JSON; na sequência congela também o relógio da animação. `--scene opening|sequence|night|landscape|tunnel|field` seleciona a composição no arranque; Opening é o valor inicial. N/L/T/F saem da sequência. A seleção é local e não altera o contrato JSON.

A sequência reproduz o MASTER e usa o consumo de amostras SDL como relógio. Espaço pausa áudio, animação e deslocamento; Q reinicia. Opening termina com o áudio aos 424,182 s. O fade de toda a imagem, incluindo o contador, começa aos 408,182 s e termina em preto. A câmara começa a avançar imediatamente a 150 cm/s, com espera e aceleração configuráveis e mantém um corredor central livre. Os sólidos triangulares estão removidos por predefinição; os cones luminosos constroem-se a partir dos 136 s. Os pontos caem continuamente; os tracejados sobem/descem; as formas arredondadas só acendem durante pulsos graves detetados no áudio.

--scene sequence seleciona a montagem original, com transição para as cenas anteriores aos 120–128 s. Opening é a composição afinada atual. --start-time 96 começa num ponto intermédio; --fixed-time 96 fixa também a animação e a câmara para capturas. --play inicia o avanço em VR; --paused mantém a pausa. motion_speed=0 para o deslocamento e 0,3 mantém o percurso aprovado; outros valores multiplicam-no proporcionalmente. A música conserva o seu relógio. Os comandos JSON não são sobrescritos pela sequência.

--silent e --fixed-time desativam a reprodução, mas conservam os pulsos derivados do WAV no tempo selecionado. --audio FILE.wav seleciona tanto a música como a fonte da deteção de graves; para reprodução, requer PCM16 estéreo 44100 Hz que cubra a duração selecionada. A saída usa o dispositivo de áudio predefinido do Windows. A latência física do dispositivo e da transmissão Virtual Desktop não é compensada.

Num segundo terminal:

```powershell
python tools\send_command.py examples\calm.json
python tools\send_command.py examples\energetic.json
```

Por omissão, a aplicação escuta apenas em `127.0.0.1:47631`. Para usar outra porta:

```powershell
.\build\Release\fantasynt.exe --port 47632
python tools\send_command.py examples\balanced.json --port 47632
```

## Mensagens JSON

```json
{
  "schema_version": 1,
  "id": "manual-001",
  "command": "apply_parameters",
  "transition_seconds": 3,
  "parameters": {
    "light_intensity": 0.5,
    "motion_speed": 0.2,
    "fog_density": 0.25
  }
}
```

As atualizações são parciais e atómicas: os campos omitidos conservam o destino anterior. O novo comando inicia a transição a partir do estado visual atual, mesmo se a transição anterior estiver a decorrer.

| Parâmetro | Valores |
| --- | --- |
| `palette` | Dois valores `#RRGGBB`; as cores são interpoladas em espaço linear. |
| `light_intensity` | 0–1: intensidade da emissão luminosa. |
| `motion_speed` | 0–1: cenas manuais até 187,5 cm/s; na sequência, 0,3 segue o percurso aprovado, 0 para e 1 multiplica o avanço por 3⅓. |
| `pulse_strength` | 0–1: intensidade dos pulsos; Opening usa transientes graves do WAV. |
| `fog_density` | 0–1: densidade do nevoeiro. |
| `geometry_density` | 0–1: proporção visual de elementos ativos no conjunto preparado. |
| `bloom` | 0–1: contribuição do brilho no pós-processamento. |

`transition_seconds` aceita 0,1–15 segundos. O identificador tem 1–64 caracteres ASCII entre letras, números, hífen e sublinhado. Propriedades desconhecidas, chaves repetidas, números não finitos, valores fora dos limites e mensagens maiores que 16 KiB são rejeitados.

O transporte usa JSON UTF-8 por linha, aceitando LF ou CRLF e mensagens fragmentadas. As confirmações incluem `status`, revisão, estado atual, destino e latência de entrega ao controlador. `accepted` confirma a aceitação e início da transição; não o seu fim. Erros do enquadramento e da fila são devolvidos pelo transporte e podem não incluir um identificador ou estado da cena.

Os comandos e recibos são registados em `fantasynt-events.jsonl`, na pasta de execução. `payload_hex` conserva os bytes recebidos em hexadecimal, incluindo entradas com UTF-8 inválido. O cliente não repete comandos automaticamente após uma falha de ligação.

## Capturas e execução limitada

```powershell
.\build\Release\fantasynt.exe --hidden --frames 240 --capture plan\m1-default.png
.\build\Release\fantasynt.exe --hidden --frames 240 --preset energetic --capture plan\m1-energetic.png
```

`--width` e `--height` configuram a janela. `--shaders` permite selecionar explicitamente outra pasta de shaders; um ficheiro em falta produz erro de arranque. `--help` lista os argumentos. No fim de uma execução limitada, a aplicação escreve um resumo JSON de frames, duração, FPS médio e latências dos comandos aceites.

## Âmbito histórico M1

A primeira versão implementou uma cena, controlo manual por JSON, transições, reflexão planar, nevoeiro e bloom. A pulsação é um sinal sintético de 120 BPM; não é análise de áudio. A ligação ao LLM usará o mesmo contrato, mas pertence ao marco M3. M2 introduzirá análise de áudio e reprodução de sessões.

Opening abrange os primeiros seis minutos, configuráveis em 0.10.0; a sequência de sete minutos e as cenas anteriores continuam disponíveis. Ainda não reproduz integralmente a coreografia nem todos os recursos do Fantasynth. A fidelidade visual é avaliada comparando as capturas com as imagens oficiais, separadamente dos testes técnicos. As metas de desempenho devem ser verificadas na GPU e resolução efetivamente usadas.

Desenho: [especificação C++](plan/2026-10-11-fantasynth-cpp-design.md). Plano: [M1](plan/2026-10-11-fantasynth-cpp-m1-plan.md). Progresso e decisões: [registo de execução](plan/2026-10-11-m1-execution.md).

## Validação histórica M1 — versão 0.1.0

Compilação Release e revisão independente concluídas. CTest: quatro testes aprovados, incluindo redimensionamento e capturas na GPU. Python: oito testes aprovados, incluindo comunicação com o executável, rejeição atómica, recuperação após UTF-8 inválido, porta ocupada e shader em falta. A preparação incremental dos shaders e exemplos foi verificada separadamente.

Na RTX 2060 Max-Q, uma execução de dez segundos a 1920×1080, com janela oculta, sem captura e 43 comandos aceites, mediu **60 FPS médios** (59,999 antes do arredondamento). A entrega ao controlador teve média de 7,97 ms e máximo de 16,48 ms; o máximo medido pelo cliente para envio e resposta foi 46,98 ms. A execução com captura PNG incluiu o custo de escrita e mediu cerca de 58 FPS. Estes valores não medem a latência física do monitor nem demonstram desempenho prolongado em VR.

Capturas verificadas: [equilibrado](plan/m1-balanced.png), [calmo](plan/m1-calm.png) e [energético](plan/m1-energetic.png). A aproximação visual cobre os contornos triangulares, a paleta fria, o contraste e os reflexos; ainda difere do original na disposição, nos materiais, na iluminação volumétrica e na coreografia. Evidência e limites: [relatório M1](plan/2026-10-12-m1-validation.md).

## Afinação 0.3.0 — 12 de outubro de 2026

A investigação recuperou propriedades de 19 recursos cooked, com SHA-1 validado: materiais dos painéis e carcaças, emissão, névoa, referências de componentes e uma curva de cor. [Achados e limites](plan/fidelity-assets/findings.md). Não descodificou shaders, render targets dinâmicos, buffers das malhas ou coreografia integral. O produto usa somente malhas e shaders próprios; as propriedades recuperadas orientam escolhas, sem equivalência automática entre os parâmetros dos dois renderizadores.

Capturas da nova cena: [túnel energético](plan/fidelity-tunnel-final.png), [túnel calmo](plan/fidelity-tunnel-calm.png). Melhorias e diferenças restantes: [relatório visual](plan/2026-10-12-fidelity-validation.md). Os padrões continuam sintéticos e a disposição ainda é mais regular do que a referência; não se declara fidelidade integral.

## Composição contínua 0.4.2 — 12 de outubro de 2026

O campo noturno usa leques curvos com contornos luminosos, placas e postes. A paisagem acrescenta cones divididos em sete volumes, padrões coloridos, superfícies inferiores emissivas e elementos suspensos. O túnel conserva as famílias da versão anterior. Reflexos, névoa e céu acompanham a composição, com imagens e projeções próprias de cada olho em VR.

As cores de névoa recuperadas de `plan/full-reference/timeline.json` orientam a sequência; os tempos e ações estão documentados em `action-sequencers.json`. Estas associações são estáticas e não demonstram equivalência com a execução do original. A aplicação usa recursos próprios e não reproduz áudio.

Capturas: [campo noturno](plan/sequence-night-40.png), [paisagem](plan/sequence-landscape-180.png), [túnel](plan/sequence-tunnel-300.png). Verificação, comparação e diferenças pendentes: [auditoria 0.4.2](plan/full-fidelity-audit.md).

## Áudio recuperado —0.4.2

O pacote inclui audio/ChezNous_MASTER.wav e a sua proveniência, convertidos do Ogg recuperado no PAK original. O PCM é idêntico ao descodificado na investigação (sem normalização/re-amostragem). A pulsação dos shaders continua sintética; não se implementou análise de batidas ou toda a coreografia original. Malhas/texturas de amostra estão recuperadas mas ainda não substituem os recursos procedurais no produto. Ver [desenvolvimento de áudio](plan/audio-integration/findings.md).
