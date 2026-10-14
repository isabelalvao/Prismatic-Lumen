# Fantasynt

Protótipo C++ de um ambiente audiovisual procedural controlado por JSON, inspirado no Fantasynth One de 2017. A versão 0.4.1 liga um campo noturno de formas curvas luminosas, uma paisagem de cones segmentados e elementos suspensos e o túnel radial numa sequência visual de 424 segundos. As cenas podem ser selecionadas separadamente; o campo antigo continua acessível para comparação.

A fidelidade visual da primeira cena foi rejeitada pelo utilizador. A versão 0.2.0 acrescentou execução PCVR, que o utilizador confirmou no Quest 3 via Virtual Desktop, incluindo funcionamento dos comandos. As notas mais recentes registam também a confirmação da versão 0.3.0 em VR e da direção visual. As novas cenas e transições 0.4.1 ainda precisam de avaliação no headset; a reconstrução integral continua pendente.

O renderer usa SDL3, bgfx e Direct3D 11. O contrato e as transições são independentes da GPU. Os recursos e shaders são próprios; as imagens do original em `plan/reference` servem apenas como referência de investigação.

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

O executável fica em `build/Release/fantasynt.exe`. Mantém as pastas `shaders` e `examples` ao lado do executável. A máquina de desenvolvimento usa Build Tools 2026, MSVC 14.51 e CMake 4.3.1.

## Executar e controlar

O pacote portátil para Windows 11 x64 está em `dist/Fantasynt-PCVR-Win11-x64-0.4.1.zip`. Extrai-o inteiro e abre `Launch-VR.cmd` para VR ou `Launch-Desktop.cmd` para monitor. Inclui shaders, exemplos, loader OpenXR e DLLs Visual C++; o PC de destino não precisa de compilador nem Python. O runtime simulado dos testes fica fora do pacote. Os ZIPs 0.2.0, 0.3.0 e 0.4.0 foram conservados.

Para **Quest 3 via Virtual Desktop**, seleciona **VirtualDesktopXR/VDXR** nas definições do Virtual Desktop Streamer, liga o headset ao PC e inicia o lançador a partir do ambiente de trabalho remoto. Confirma `Runtime: VDXR` no overlay do Virtual Desktop. Este percurso suporta aplicações OpenXR sem SteamVR, segundo a [documentação VDXR](https://github.com/mbucchia/VirtualDesktop-OpenXR/wiki).

O modo VR usa tracking previsto, projeções independentes por olho e imagens D3D11 do runtime. A sequência e o avanço da câmara começam pausados; Espaço retoma-os. Os controladores Quest ainda não estão mapeados. O teclado atua quando a janela do programa tem foco. Para ajustar a resolução, executa `Launch-VR.cmd --vr-scale 0.5` ou `--vr-scale 1.0`; o valor inicial é 0,7. `Diagnose-VR.cmd` verifica runtime e disponibilidade do headset e guarda o resultado em `logs/xr-probe.log`.

Na árvore de desenvolvimento, o equivalente é `build/Release/fantasynt.exe --vr`. `--xr-probe` faz apenas o diagnóstico. O controlo JSON continua disponível; o pacote inclui `Send-Parameters.cmd examples/calm.json`, que usa PowerShell e não requer Python.

**Validação VR:** testes locais com runtime simulado e leitura das duas imagens na GPU; funcionamento da versão 0.2.0 confirmado pelo utilizador no Quest 3/Virtual Desktop. As notas posteriores registam a confirmação 0.3.0 em VR. As cenas e transições 0.4.1 foram testadas com o runtime simulado e continuam por avaliar no headset. Relatório: [validação PCVR](plan/2026-10-12-quest3-vr-validation.md).

Abre `Fantasynt.cmd`, ou executa:

```powershell
.\build\Release\fantasynt.exe
```

Teclas: **1** calmo, **2** equilibrado, **3** energético, **Q** reinicia a sequência, **N** campo noturno, **L** paisagem colorida, **T** túnel, **F** campo anterior, **Space** pausa/retoma a sequência e a câmara, **R** repõe o estado equilibrado, **Esc** sai. A pausa mantém a rede e as transições JSON; na sequência congela também o relógio da animação. `--scene sequence|night|landscape|tunnel|field` seleciona a composição no arranque; a sequência é o valor inicial. N/L/T/F saem da sequência. A seleção é local e não altera o contrato JSON.

A sequência dura 424 segundos e termina com um escurecimento; Q reinicia-a. `--start-time 180` começa numa fase intermédia. `--fixed-time 228` fixa o tempo, a animação e a câmara para capturas reproduzíveis. `--play` inicia o avanço em VR; `--paused` mantém a pausa também no monitor. A entrada da paisagem aos 80 s e do túnel aos 224 s foi inferida de dados estáticos do original. As transições 80–112 e 224–232 s e a distribuição de instâncias são próprias. A posição inicial da câmara e a cor de névoa usam chaves recuperadas, com modos Linear/Constant/Cubic confirmados por análise nativa REA. O percurso mantém a primeira posição até aos 112 s; saltar no tempo posiciona a câmara nesse ponto. O parâmetro motion_speed modula o deslocamento: 0 imobiliza a câmara, 0,3 segue a velocidade recuperada e outros valores multiplicam-na proporcionalmente, conservando o relógio da sequência. A geometria continua a repetir-se a cada 41600 cm. Detalhes e limites em [investigação REA](plan/rea-investigation/curve-development.md). A sequência não sobrescreve os parâmetros escolhidos por JSON.

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
| `motion_speed` | 0–1: cenas manuais até 250 cm/s; na sequência, 0,3 segue a curva recuperada, 0 para e 1 multiplica o avanço por 3⅓. |
| `pulse_strength` | 0–1: intensidade da pulsação sintética. |
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

A versão 0.4.1 cobre três composições e uma sequência de cerca de sete minutos, mas não reproduz integralmente a coreografia nem todos os recursos do Fantasynth. A fidelidade visual é avaliada comparando as capturas com as imagens oficiais, separadamente dos testes técnicos. As metas de desempenho devem ser verificadas na GPU e resolução efetivamente usadas.

Desenho: [especificação C++](plan/2026-10-11-fantasynth-cpp-design.md). Plano: [M1](plan/2026-10-11-fantasynth-cpp-m1-plan.md). Progresso e decisões: [registo de execução](plan/2026-10-11-m1-execution.md).

## Validação histórica M1 — versão 0.1.0

Compilação Release e revisão independente concluídas. CTest: quatro testes aprovados, incluindo redimensionamento e capturas na GPU. Python: oito testes aprovados, incluindo comunicação com o executável, rejeição atómica, recuperação após UTF-8 inválido, porta ocupada e shader em falta. A preparação incremental dos shaders e exemplos foi verificada separadamente.

Na RTX 2060 Max-Q, uma execução de dez segundos a 1920×1080, com janela oculta, sem captura e 43 comandos aceites, mediu **60 FPS médios** (59,999 antes do arredondamento). A entrega ao controlador teve média de 7,97 ms e máximo de 16,48 ms; o máximo medido pelo cliente para envio e resposta foi 46,98 ms. A execução com captura PNG incluiu o custo de escrita e mediu cerca de 58 FPS. Estes valores não medem a latência física do monitor nem demonstram desempenho prolongado em VR.

Capturas verificadas: [equilibrado](plan/m1-balanced.png), [calmo](plan/m1-calm.png) e [energético](plan/m1-energetic.png). A aproximação visual cobre os contornos triangulares, a paleta fria, o contraste e os reflexos; ainda difere do original na disposição, nos materiais, na iluminação volumétrica e na coreografia. Evidência e limites: [relatório M1](plan/2026-10-12-m1-validation.md).

## Afinação 0.3.0 — 12 de outubro de 2026

A investigação recuperou propriedades de 19 recursos cooked, com SHA-1 validado: materiais dos painéis e carcaças, emissão, névoa, referências de componentes e uma curva de cor. [Achados e limites](plan/fidelity-assets/findings.md). Não descodificou shaders, render targets dinâmicos, buffers das malhas ou coreografia integral. O produto usa somente malhas e shaders próprios; as propriedades recuperadas orientam escolhas, sem equivalência automática entre os parâmetros dos dois renderizadores.

Capturas da nova cena: [túnel energético](plan/fidelity-tunnel-final.png), [túnel calmo](plan/fidelity-tunnel-calm.png). Melhorias e diferenças restantes: [relatório visual](plan/2026-10-12-fidelity-validation.md). Os padrões continuam sintéticos e a disposição ainda é mais regular do que a referência; não se declara fidelidade integral.

## Composição contínua 0.4.1 — 12 de outubro de 2026

O campo noturno usa leques curvos com contornos luminosos, placas e postes. A paisagem acrescenta cones divididos em sete volumes, padrões coloridos, superfícies inferiores emissivas e elementos suspensos. O túnel conserva as famílias da versão anterior. Reflexos, névoa e céu acompanham a composição, com imagens e projeções próprias de cada olho em VR.

As cores de névoa recuperadas de `plan/full-reference/timeline.json` orientam a sequência; os tempos e ações estão documentados em `action-sequencers.json`. Estas associações são estáticas e não demonstram equivalência com a execução do original. A aplicação usa recursos próprios e não reproduz áudio.

Capturas: [campo noturno](plan/sequence-night-40.png), [paisagem](plan/sequence-landscape-180.png), [túnel](plan/sequence-tunnel-300.png). Verificação, comparação e diferenças pendentes: [auditoria 0.4.1](plan/full-fidelity-audit.md).
