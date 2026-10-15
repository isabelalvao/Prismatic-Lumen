# Evidência e decisões de partida

Data: 2026-10-14. Este documento distingue observações do projeto de propostas de implementação.

## Finalidade confirmada pelo utilizador

O Fantasynt será um campo de ensaio da atribuição de agência a um ambiente virtual imersivo não figurativo e da ambiguidade entre essa atribuição e o prolongamento da agência do utilizador. A opacidade causal faz parte da hipótese. A leitura da entidade deve ser holística: corpo, ambiente e história da interação. Não usar controladores; usar seguimento das mãos e da cabeça dos Quest 3. Voz é uma possibilidade, fisiologia uma extensão futura. Jev foi sugerido como candidato à decisão semântica.

## Estado observado no código

| Evidência | Implicação para o plano |
|---|---|
| `src/control/parameters.hpp` expõe paleta, intensidade luminosa, velocidade, pulsação, nevoeiro, densidade e bloom. | A primeira experiência pode aproveitar controlos globais; não existe ainda controlo espacial completo por região. |
| `src/control/scene_controller.cpp` interpola atualizações parciais a partir do estado corrente. | Reutilizar a interpolação; acrescentar composição e ciclos de vida sem substituir o percurso existente. |
| `src/control/protocol.cpp` admite apenas `apply_parameters` na versão 1. | Consultas, capacidades, telemetria e políticas exigem evolução explícita do contrato. |
| `src/app/main.cpp` devolve `accepted`, revisão, estado corrente e destino; o recibo antecede o fim da transição. | Distinguir aceitação, aplicação, conclusão, substituição e expiração. |
| `src/render/art_direction.cpp` mistura a paleta recebida com paletas temporais da coreografia. | A sobreposição semântica deve ser observável no resultado efetivo; um comando aceite não comprova efeito. |
| `src/vr/openxr_session.cpp` usa poses previstas para apresentação e calcula uma origem comum aos olhos. | Acrescentar aquisição dedicada e validade das mãos/cabeça; conservar os tempos e referências espaciais. |
| `src/audio/player.hpp` expõe pausa, posição, duração e procura temporal sobre um WAV. | Ganho e modulação sonora necessitam de implementação; mistura de pistas não está disponível nesta interface. |
| `src/audio/beat_envelope.hpp` fornece envolvente e eventos derivados do WAV. | Contexto musical local possível; separar fonte original e estado sonoro depois dos efeitos. |
| `CMakeLists.txt` declara 0.12.0 e testes de núcleo, áudio, renderização e runtime OpenXR simulado. | Acrescentar testes às fronteiras existentes e validar posteriormente no headset real. |

Os caminhos acima são relativos à raiz `C:/ORCA/Fantasynt`. A documentação histórica contém estados anteriores; as interfaces de código foram consultadas para este plano.

## Fontes técnicas consultadas nesta conversa

- [Primitivas Jev](https://docs.typesafe.ai/primitives): Choice, Score e Noul; perguntas da mesma chamada são avaliadas independentemente. Não equivale a garantir coerência conjunta.
- [Estado Jev](https://docs.typesafe.ai/concepts/state): entrada textual/JSON; áudio, imagem e vídeo não são entradas diretas documentadas.
- [Limitações Jev 1.13](https://docs.typesafe.ai/model-jaggedness/jev-1.13): precisão numérica, ordem das opções e indireção requerem avaliação. Não usar Score como medição física.
- [Confiança Jev](https://docs.typesafe.ai/confidence): estatística da distribuição de respostas; limiares dependem da avaliação no domínio.
- [VDXR](https://github.com/mbucchia/VirtualDesktop-OpenXR/blob/main/virtualdesktop-openxr/instance.cpp): extensão de seguimento das mãos no código; disponibilidade local ainda por verificar.
- [Validade de velocidades OpenXR](https://registry.khronos.org/OpenXR/specs/1.1/man/html/XrHandJointVelocityEXT.html): os indicadores de validade fazem parte da interpretação dos dados.
- [Ganho SDL3](https://wiki.libsdl.org/SDL3/SDL_SetAudioStreamGain): capacidade de ganho por fluxo; transições e comportamento audível precisam de validação no produto.

Não foram medidos desempenho Jev, latência PCVR ou efeitos percetivos do repertório. Os números no plano são valores iniciais de engenharia, não resultados nem limiares científicos.

## Memória de projeto

A regra de projeto levou à consulta das competências `obsidian-project-bootstrap` e `obsidian-project-memory` e à execução de `project_kb.py detect`. A deteção assinalou candidato de investigação não registado. Não existe caminho de vault fornecido nem `OBSIDIAN_VAULT_PATH` definido; a competência de bootstrap requer uma destas fontes. Por isso, o plano fica integralmente preservado em `plan/`, sem criar um vault arbitrário.

O script instalado procurou `.claude/project-memory/registry.yaml`, enquanto as instruções do utilizador indicam `.Codex/project-memory/registry.yaml`. Esta divergência deve ser resolvida antes de um futuro bootstrap. Não foram escritos registos em nenhuma dessas localizações. A preparação do plano não depende dessa ligação.
