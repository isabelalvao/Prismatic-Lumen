# Mãos visíveis a partir de articulações OpenXR

**Retorno do utilizador, 2026-10-15:** «funciona bem». Confirmação reportada de funcionamento da representação no seu teste. Não foram fornecidos novos registos ou medições de alinhamento, cobertura, recentragem, latência ou desempenho; não substituir esses valores pelo resultado dos testes simulados. Próxima implementação proposta: P1, executor local de motivos globais; afinação da leitura corporal com novo ensaio técnico antes de a ligar a decisões adaptativas.

Implementação pedida pelo utilizador e concluída em 2026-10-15, na raiz `C:\DEV\Prismatic`. A compilação de desenvolvimento desenha mãos em VR; os pacotes P0 históricos não foram substituídos. Cópias delimitadas do estado anterior em [backup/](backup/), incluindo as alterações locais já existentes antes desta tarefa. Sem commit/push.

## Comportamento

- Até 26 esferas e 25 segmentos por mão, com topologia de `XR_HAND_JOINT_SET_DEFAULT_EXT`. Esquerda azul/ciano; direita laranja (a composição final pode aproximá-la de amarelo).
- Usa as posições LOCAL adquiridas para o tempo previsto de cada frame XR, sem passar pela publicação de registos a 20 Hz. A mesma geometria entra nas projeções próprias de ambos os olhos e no espelho do olho esquerdo.
- Conversão de metros para centímetros e inversão de Z partilhadas com os olhos. Aplica os offsets VR da configuração; não aplica escala artística, avanço virtual ou wrapping da cena às mãos.
- Desenha apenas articulações com posição válida, seguida e finita de uma mão ativa, sem erro de localização. Orientação não é necessária para ligar posições. Um segmento exige ambas as extremidades utilizáveis. Não interpola nem retém poses através de perdas; a recuperação usa a amostra atual.
- Os raios do runtime servem a representação, limitados visualmente a 1–25 mm. Um raio não positivo/não finito usa 6 mm. Este tratamento não modifica aquisição, JSONL, velocidades ou limiares.
- Shader com contraste fixo, profundidade partilhada com a cena e submissão antes das transparências. Sem reflexos das mãos ou fog próprio. O pós-processamento existente, incluindo bloom e fade final, continua a abranger a imagem. Conserva o plano próximo configurado (10 cm na raiz); uma mão junto ao visor pode ser cortada.
- Pausa audiovisual mantém aquisição e desenho em movimento. Sem comandos por gesto, adaptação audiovisual ou Jev/LLM.
- O manifesto de gravação VR passa a `condition=hand_visualization`, `acquisition_only=false`, com representação e política `valid_and_tracked`. O indicador de arranque/manifesto `hands_rendered` identifica o modo visual habilitado; disponibilidade efetiva continua a depender de `tracking` e das articulações de cada observação. Um runtime sem extensão/capacidade continua a funcionar sem desenhar mãos e reporta a causa técnica.

Fontes: `src/render/hand_geometry.*`, `shaders/vs_hand.sc`, `shaders/fs_hand.sc`, integração em `src/render/renderer.*` e `src/app/main.cpp`, conversão em `src/vr/view_math.*`. Aquisição P0 preservada.

## Executar no equipamento real

Na raiz, com Quest ligado ao PC e encaminhamento de hand tracking ativo no runtime:

```powershell
.\build\Release\fantasynt.exe --config .\visual-config.json --vr --paused --record-session logs\vr-hands-01.jsonl
```

Usar um nome novo para cada gravação; a aplicação recusa sobrescrever. Espaço inicia/pausa a experiência; Esc encerra e finaliza o registo. Observar dedos, escala, alinhamento, movimento da cabeça com mãos quietas, perdas de uma/ambas as mãos, recuperação e recentragem. Para continuar a inspeção:

```powershell
python .\tools\inspect_session.py .\logs\vr-hands-01.jsonl --output .\logs\vr-hands-01.analysis.json
```

Não afinar limiares com o ensaio anterior sem protocolo. Dados ausentes continuam a ser desconhecidos, nunca repouso.

## Verificação realizada

- Compilação Release pelo fluxo de `tools/build.cmd`, com shaders e executável atualizados.
- **19/19 CTest aprovados**, excluindo `renderer_resize` para não substituir capturas históricas; incluem o novo teste de geometria. O teste de geometria foi repetido após acrescentar verificações de disparidade e recentragem.
- **Teste gráfico aprovado**, executado diretamente uma vez com saída em `temp/vr-hands-renderer/resize.png` e respetivas capturas de controlo em `temp/vr-hands-renderer/controls/`. Corresponde ao vigésimo teste C++ registado no CMake.
- **75/75 testes Python aprovados**. As duas verificações novas com imagens D3D11 comparam ausência de extensão/capacidade, falhas de criação/localização, inatividade, desaparecimento após frames válidos, aparecimento/recuperação em pausa, validade parcial, posições válidas sem seguimento e deslocamento das articulações. O runtime também verifica a submissão e a distinção das duas imagens estereoscópicas.
- Geometria: 26 articulações/25 segmentos, conversão de coordenadas e offsets, disparidade de 6,4 cm entre olhos, mudança comum de origem de olhos/mãos, validade de cada articulação independente, erro/inatividade, posições não finitas, raios não finitos e articulações coincidentes.
- Revisão local das diferenças contra as cópias anteriores: aquisição/derivadas preservadas; shader das mãos sem deslocamento virtual; profundidade antes da transparência; buffers limitados, índices de 16 bits e limpeza de recursos. Nenhum problema bloqueante identificado. `git diff --check` sem erros de whitespace.
- `visual-config.json` e `assets/visual-config.json` conservam o SHA-256 `01d5bbc348c3f25337affec6463d515abd2d17adcbf894e25934e67fa87faf17`. Opening, MASTER e percurso não foram editados.

O teste Python de integração existente voltou a gerar `plan/m1-integration.png` antes de se detetar que gravava na evidência histórica. A imagem anterior não tinha cópia delimitada nesta tarefa. O destino desse teste foi corrigido para `temp/m1-integration.png`; os seus quatro testes foram repetidos e aprovados. As restantes capturas de controlo anteriores não foram substituídas.

Resultados da bateria e hashes em [verification.json](verification.json) e [python-tests.log](python-tests.log). Executável verificado: `build/Release/fantasynt.exe`, SHA-256 `ff146a6deae649ddae866f51c5b3d454120d4af473b4cce2c9e8c5e6f6198956`.

## Capturas de simulação e limites

Capturas do espelho com pose sintética, **sem prova física no Quest**:

- [Esqueleto completo](hand-skeleton.png).
- [Indicadores sem posição utilizável](hand-skeleton-partial.png): removem-se também os segmentos incidentes.
- [Runtime sem extensão de mãos](no-hands.png): mesma cena sem mãos.

Os ficheiros JSON com os mesmos nomes contêm os relatórios de arranque/desempenho dessas execuções. As leituras GPU e a baixa resolução do runtime simulado impedem usar esses FPS como medição PCVR. Falta verificar fisicamente alinhamento, escala, precisão, oclusão, recentragem, conforto e desempenho no Quest/VDXR. O JSONL real anterior permanece em `logs/hand-tracking-01.jsonl`, sem alteração; não foi usado para calibrar ou substituir tracking em direto.
