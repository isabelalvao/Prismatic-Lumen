# Build PCVR — Windows 11 x64 / Quest 3 / Virtual Desktop

Pedido autorizado: preparar um build transportável que permita testar a aplicação em VR na outra máquina. Alvo confirmado pelo utilizador: Quest 3 via Virtual Desktop, Windows 11 de 64 bits. Execução e escolhas de integração prosseguem dentro deste pedido; não há publicação, instalação remota ou alteração do runtime global.

## Resultado e limites

ZIP com executável Release, shaders, exemplos, loader OpenXR, runtime C++ redistribuível local, cliente de controlo e instruções. Lançador VR activa `--vr`; lançador de monitor mantém o fluxo existente. Não exige compilador na máquina de destino. VDXR é seleccionado pelo utilizador no Virtual Desktop Streamer. O pacote não inclui o runtime VDXR/Virtual Desktop nem exige trocar o runtime nesta máquina.

VR: duas imagens com perspectivas independentes, posição/orientação da cabeça previstas pelo runtime, espaço LOCAL, unidades OpenXR em metros convertidas para os centímetros da cena, sessão sincronizada por xrWaitFrame/xrBeginFrame/xrEndFrame. Renderização Direct3D 11 no adaptador exigido pelo runtime; bgfx recebe esse dispositivo e envolve as texturas externas sem assumir a propriedade dos recursos OpenXR. VR usa submissão bgfx síncrona para impedir libertar imagens antes de emitir os comandos. Janela de monitor mostra um espelho do olho esquerdo. Nenhuma inferência LLM no ciclo VR.

Modo VR começa com o percurso automático pausado; acompanhamento da cabeça permanece activo. Space retoma/pausa o percurso; 1/2/3 seleccionam estados; Esc termina. A rede continua em loopback. Controladores/manipulação de objectos e áudio real não são adicionados neste build. O visual actual foi rejeitado pelo utilizador e continua identificado como protótipo técnico; adicionar VR não resolve essa fidelidade.

Diagnóstico `--xr-probe`: identifica runtime/extensão D3D11 e disponibilidade de headset sem abrir a cena. Erros de runtime ausente, headset desligado, formato incompatível ou sessão perdida são explícitos. Não haverá fallback silencioso para monitor quando o utilizador pede VR.

## Verificação

Testes de matemática: transformações de cabeça, IPD, frustum assimétrico e mapeamento de profundidade. Runtime OpenXR simulado em testes: ordem das chamadas, duas imagens reais D3D11, submissão e encerramento; sem apresentar esse ensaio como teste num headset. Executar ainda os testes existentes de monitor/JSON. Extrair o ZIP para outra pasta e testar o próprio pacote. Teste final Quest 3/VDXR fica para a outra máquina do utilizador.

## Referências

- https://github.com/KhronosGroup/OpenXR-SDK — loader e cabeçalhos, commit fixado `97dcbd94ed2e006329e383a4de456e080338d2bb` (release-1.1.63).
- https://github.com/mbucchia/VirtualDesktop-OpenXR/wiki — VDXR incluído no Virtual Desktop Streamer; configuração e diagnóstico.
- Cabeçalhos/exemplos locais das dependências fixadas bgfx/bx — dispositivo externo e createTexture2D(..., external).
