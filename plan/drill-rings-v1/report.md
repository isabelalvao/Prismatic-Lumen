# Brocas, relógio e anéis — entrega 0.9.1

Data: 2026-10-13. Implementação da direção artística especificada pelo utilizador nos três desenhos enviados. A duração mantém-se em seis minutos; não foi acrescentada uma cena posterior aos anéis, cuja direção ainda não foi definida.

## Relógio

O contador arranca sempre desligado e só a tecla C altera a visibilidade. Um ficheiro antigo com timer.visible=true é aceite, mas não ativa o contador. Recarregar o JSON conserva a escolha feita no teclado. Escala, opacidade e posição continuam editáveis.

A duplicação tinha como causa o posicionamento em coordenadas normalizadas iguais nas duas lentes, ignorando a diferença de projeção. O contador passa a uma placa comum a dois metros da cabeça. O OpenXR calcula a posição central entre olhos e a orientação média normalizada; cada olho projeta a placa com a sua própria matriz, incluindo a diferença de posição, campo de visão assimétrico e inclinação. A homografia inversa desenha os glifos depois da composição visual. O monitor conserva o posicionamento normalizado anterior.

Os ensaios matemáticos verificam convergência a dois metros e os extremos da placa com olhos inclinados. A integração envia C à janela SDL do processo VR, confirma o estado e a diferença efetiva entre capturas equivalentes. Estes ensaios usam um runtime simulado com imagens D3D11 reais; não substituem a observação no Quest 3 físico.

## Cones

Começam aos 136 s, com desfasamento por cone até dois segundos. Emergência de 14 s, rotação comum de oito graus/s, separação de três segundos e folga adicional de 0,6 m entre módulos. Só depois da emergência e separação começam as rotações independentes de ±2 graus/s. Os sete módulos ficam juntos durante a subida; a ponta atravessa o piso primeiro e a base chega ao chão no fim. O material subterrâneo é recortado antes da escrita de profundidade no passe principal e no reflexo.

As velocidades de rotação têm histórias integradas independentes, preservando ângulos na recarga. Alterar tempos ou folgas reposiciona diretamente a coreografia. O áudio original foi preservado.

## Túnel

Mantêm-se a retirada da fase anterior entre 208–224 s, montagem do túnel entre 224–272 s e luz de fundo entre 272–284 s. Aos 270 s os outros artefactos começam a afastar-se radialmente e em profundidade, com desfasamentos individuais. Aos 300 s são retirados da imagem. Apenas os 64 anéis axiais permanecem, alinhados com o percurso e com perfil mais fino, formando o túnel por onde a câmara continua a avançar. A geometria da cena manual Tunnel foi preservada.

## Configuração

Sete novos controlos elevam o total a 203 opções tipadas, além das cores, parâmetros globais e áudio:

- cones.emerge_seconds, cones.separate_seconds, cones.drill_rotation_degrees_s, cones.module_gap_m.
- extension.tunnel_departure_start_seconds, extension.tunnel_departure_seconds, extension.tunnel_departure_distance_m.

Guardar visual-config.json ou carregar F5 recarrega os controlos suportados. Configurações inválidas conservam os valores anteriores. As afinações existentes foram conservadas, atualizando os novos campos e a predefinição do contador. O guia identifica timer.visible e cones.segment_interval_seconds como campos de compatibilidade; o primeiro é ignorado e o segundo deixou de controlar a emergência da broca.

## Verificação e artefactos

16 testes CTest aprovados; suite Python completa com 44 testes aprovada e depois suite VR com dez testes, incluindo um novo teste de C e captura. Total: 45 testes Python distintos aprovados. Depois de afinar o perfil dos anéis, os 16 testes CTest e regressões GPU foram repetidos; o teste do renderer foi executado novamente depois de acrescentar o modo de prévia. Resultados em verification.log, ring-verification.log, renderer-final.log, vr-timer-verification.log e gpu-validation.log.

Capturas 136, 140, 143, 148, 150, 153, 156, 224, 240, 270, 284, 300, 330 e 360 s guardadas em captures/. Inspecionadas imagens de emergência, módulos separados e anéis finais. O ensaio GPU demonstra que alterar a distância de afastamento não altera a imagem aos 270 s nem aos 300 s, mas altera-a durante a saída aos 284 s. Desativar a saída conserva os outros artefactos, produzindo uma imagem diferente. Não houve erros graves pendentes na revisão independente.

drill-preview.mp4 contém 80 fotogramas de renderização real a quatro imagens/s, com câmara fixa e sem áudio ou pulsos, mostrando os instantes 136–155,75 s durante 20 s à velocidade normal. As imagens originais do utilizador não foram alteradas.

O pacote extraído num caminho com caracteres portugueses passou em desktop, estéreo OpenXR simulado, cliente JSON PowerShell e fases da nova coreografia. Hashes do manifesto válidos. Ver package-check.json e package-check.log.

## Entrega e preservação

Pacote: dist/Fantasynt-PCVR-Win11-x64-0.9.1.zip.

SHA256: 314f94b65ac7aa3c287d708a219c3d51845f2bfe57d356da898cf05a8230f566.

O ZIP 0.9.0 foi preservado, com SHA256 48385a6150fe0ae3bef52031c139f0523781a7ea32a5ab7964665e6c5ac8cefd. MASTER preservado: 60dc74309ab1fbae42394228ddfc098a09a1e7a36ae739c7f5ee7e1adafb9fd9. Recursos recuperados em disco, instalação Steam, UE 4.13 e entregas anteriores conservados. Cópias dos ficheiros alterados em backup/.

Alterações principais em renderer, time_display, surface_choreography, tunnel_geometry, OpenXRSession, shaders, configuração, testes e empacotamento/documentação. Não existe repositório Git e não foi inicializado. A validação de fusão e conforto no Quest 3 físico fica para o teste do utilizador.
