# Validação M1 — 12 de outubro de 2026

## Avaliação do utilizador após a entrega

O utilizador rejeitou a fidelidade visual: faltam cor e efeitos, e os volumes semelhantes a prédios não correspondem ao original. A entrega valida a infraestrutura técnica, mas não satisfaz a reconstrução estética pretendida. A análise do original foi parcial: versão do motor, inventário do PAK, inspeção estática de recursos do executável e imagens oficiais. Não foram reconstruídos os grafos dos materiais, as sequências/Blueprints ou a dinâmica completa dos efeitos. O próximo trabalho visual deve partir dessa análise e de referências temporais do original, em vez de acrescentar variantes à cena genérica.

O executável próprio continua sem suporte VR. Foi encontrado um runtime OpenXR Oculus configurado no Windows; isso não demonstra headset ligado nem compatibilidade VR da aplicação.

## Entrega

Aplicação própria Windows C++20, SDL3 e bgfx/Direct3D 11. Executável `build/Release/fantasynt.exe`; lançador `Fantasynt.cmd`. Cliente manual `tools/send_command.py`; três exemplos JSON. Todos os recursos do produto são próprios, sem reutilização do pacote Fantasynth.

## Verificação técnica

- Compilação Release por `tools/build.cmd -Test`, com dependências fixadas.
- CTest 4/4: contrato/transições, sockets reais, geometria reproduzível e renderer com redimensionamento, dimensão zero e capturas PNG.
- Python 8/8: cliente e aplicação real, rejeição atómica, continuidade do processo após cliente desligado, entrada UTF-8 inválida seguida de comando válido, erros explícitos de porta ocupada e shader em falta.
- `tools/check_incremental_resources.py`: altera apenas shader e exemplo, em passos separados, compila e compara os recursos junto do executável; repõe sempre as fontes. Verificação adicional aprovada.
- Câmaras e efeitos verificados em capturas com tempo/posição fixos em `plan/controls`. Os ficheiros diferem quando se altera paleta, luz, pulso, nevoeiro, densidade ou bloom. O avanço da câmara tem captura separada. Estas comparações verificam efeito no renderer; não constituem uma avaliação automática de fidelidade.

## Medições

Windows x64; MSVC 14.51; GPU Direct3D 11 `vendor=0x10de device=0x1f12` (RTX 2060 Max-Q). Resolução 1920×1080, janela oculta, VSync configurado e cadência limitada a aproximadamente 60 FPS. Sem outros testes gráficos em paralelo na medição final.

`m1-benchmark.json`: 600 frames, 10,0002013 s, 59,998792 FPS. Aceitou 43 comandos: os três exemplos enviados pelo cliente CLI e 40 atualizações adicionais com transições de 0,3 s; a densidade atingiu o máximo. Entrega ao controlador: média 7,971642 ms, máximo 16,4788 ms. Ida e volta observada pelo cliente nos 40 comandos adicionais: máximo 46,9833 ms. Não confundir estas métricas com o instante em que o monitor mostra a primeira alteração.

`m1-benchmark-with-capture.json`: 600 frames, 10,3357253 s, 58,051078 FPS, 40 comandos, entrega máxima 17,284 ms. Inclui captura/escrita PNG e descarga final do renderer. As capturas dos três estados também incluem escrita PNG e registaram aproximadamente 57 FPS médios. São ensaios curtos; não foi medida estabilidade prolongada, consumo térmico ou VR.

## Comparação visual

As capturas equilibrada, calma e energética foram abertas e inspecionadas, assim como as oito capturas de controlo. Comparação com `reference/steam-4.jpg`: mantêm fundo negro, estruturas monumentais triangulares, chevrons brancos/azulados, variação de luz, brilho HDR e piso escuro refletor. A fase energética aplica a segunda paleta, com magenta/ciano; a calma reduz luz, movimento, pulso e densidade.

Diferenças visíveis: a distribuição é mais regular, os materiais são mais simples, as reflexões são planares aproximadas e o brilho não reproduz toda a dispersão/iluminação volumétrica do original. Há uma só cena, sem túneis, sequência musical original ou animações específicas de cada recurso. A fidelidade é uma direção inicial verificada por inspeção, não uma reprodução integral nem uma pontuação de semelhança validada pelo utilizador.

## Revisão independente e correções

1. **Important — registo de UTF-8 inválido:** reproduzido pelo teste `test_invalid_utf8_does_not_terminate_application`, que falhou após a primeira rejeição porque o processo terminava. O registo passa a usar `payload_hex`, preservando todos os bytes. O mesmo teste passou após a correção.
2. **Important — recursos incrementais:** o revisor identificou a dependência de `POST_BUILD`, que segundo a documentação local do CMake só se executa quando o alvo é reconstruído. O teste com Visual Studio passou também antes da alteração; não foi reproduzida a falha neste gerador e não se declara RED→GREEN deste caso. A preparação foi tornada explícita num alvo dependente da aplicação, com `copy_if_different`; a regressão separada para alterações de shader e exemplo passou. Outros geradores não foram executados.
3. **Minor no parecer, elevado a Important pela continuidade visual pedida — densidade máxima:** a geometria reservava limiares até 1 e o shader forçava altura completa a partir de 0,9999. O novo teste dos limiares falhou em 64 instâncias. Os limiares passaram para uma distribuição que termina antes de 1−1/64, e o ramo abrupto foi removido; o teste passou. Não ficaram achados menores adiados.

A falha nativa inicial foi reproduzida no teste gráfico e na aplicação. A causa era copiar um `SwapChain` vazio sobre os formatos/profundidade predefinidos de `bgfx::Init`. Preservar os valores predefinidos resolveu o arranque; os testes gráficos posteriores passaram.

## Limites e continuação

A pulsação é sintética a 120 BPM. Não há análise de áudio, adaptador LLM ou repetição completa de sessões nesta entrega. São M2/M3 da especificação aprovada. A pausa gráfica em dimensão zero foi testada directamente no renderer; não foi automatizado um ensaio de minimizar/restaurar a janela visível do Windows durante uma ligação TCP.

O diretório não é Git; não foram criados commits, branches ou worktrees. Os diagnósticos, referências e planos foram preservados. O protótipo está pronto para avaliação visual pelo utilizador antes de expandir a coreografia e ligar o LLM.
