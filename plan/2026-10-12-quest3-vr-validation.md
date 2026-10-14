# Validação PCVR — 12 de outubro de 2026

Objetivo: fornecer um pacote portátil Windows 11 x64 para testar a cena C++ no Quest 3 através de Virtual Desktop/VDXR. Não conclui a reconstrução visual do Fantasynth, que continua rejeitada na avaliação do utilizador.

Atualização após entrega: o utilizador confirmou «foi possível ver em VR, os comandos funcionam» em Quest 3 via Virtual Desktop. A versão 0.2.0 tem assim validação funcional física reportada pelo utilizador. As secções abaixo conservam o estado dos ensaios locais à data da entrega; não foram recebidas medições de frequência/latência ou aprovação da fidelidade gráfica.

## Implementação

OpenXR-SDK fixado ao commit `97dcbd94ed2e006329e383a4de456e080338d2bb` (release 1.1.63), usando API 1.0 e XR_KHR_D3D11_enable. Dispositivo D3D11 criado no adaptador exigido pelo runtime; duas swapchains, poses previstas e projeções assimétricas por olho; referência LOCAL; transformação metros→centímetros. Renderização síncrona bgfx e libertação das imagens antes de xrEndFrame. Composição com tratamento sRGB, nevoeiro na posição do olho e céu orientado pelo mundo. Espelho do olho esquerdo no monitor. O avanço começa pausado em VR, mantendo tracking e efeitos.

`--vr`, `--vr-scale` (0,4–1,0; inicial 0,7) e `--xr-probe`. Eventos sem renderização e tracking inválido entregam frames sem camadas, mantendo processamento da rede. Perdas de instância/sessão OpenXR produzem erro explícito; EXITING é encerramento normal. O contrato JSON existente permanece inalterado.

## Evidência local

- Matemática: testes primeiro com implementação vazia, falha observada em `vr-math-red.log`; implementação validada por CTest. Testa distância entre olhos, orientação, unidades e limites do frustum/NDC.
- Integração: falha inicial com executável sem argumentos VR em `vr-integration-red.log`; posteriormente renderização comprovada pelo runtime de teste. Usa texturas D3D11 reais inicialmente pretas, lê pixels após submissão e exige conteúdo e hashes distintos nos dois olhos. Verifica a ordem de aquisição, espera, libertação e fim do frame; não é apenas um teste de chamadas simuladas.
- Falha de encerramento: violação de acesso durante a libertação final do dispositivo D3D11 externo, após shutdown bgfx. Corrigida a ordem de vida: destruir invólucros gráficos e executar comandos pendentes, fechar XR, encerrar backend bgfx que mantém a última referência. A correção também cobre falha de shaders no construtor (`vr-cleanup-red.log`). A investigação não demonstrou qual componente do controlador originou a falha; não se atribui a causa a NVAPI.
- Revisão independente: sem bloqueios na integração ou pacote; duas observações corrigidas — diagnóstico de perda OpenXR e caminhos UTF-8 em pastas com acentos. A falha por perda silenciosa foi reproduzida em `vr-loss-red.log` antes da correção.
- CTest: 5/5. Python: 14 testes, incluindo seis testes VR (a perda de runtime usa dois subcasos). Logs finais: `vr-ctest-final.log`, `vr-python-final.log`.
- Pacote extraído fora de build, numa pasta com espaços e acento: execução desktop e VR simulado, screenshot e comando JSON enviado pelo cliente PowerShell distribuído. Conferência de todos os hashes do manifesto. Relatório: `vr-package-check.json`.
- Dependências PE conferidas: executável x64 e DLLs CRT locais; loader OpenXR importa apenas bibliotecas do Windows. Incluídos shaders, predefinições e licenças. O runtime simulado não é distribuído. ZIP e SHA-256 em `dist/`.
- Diagnóstico no runtime real desta máquina: `--xr-probe` devolve erro explícito; o registo Windows aponta para um manifesto Oculus que não existe. Evidência em `vr-real-probe.log`. Não se alterou a configuração do runtime do sistema.

## Limites e teste de destino

Não existe headset físico ligado nesta máquina; Quest 3/VDXR, qualidade da imagem no headset, frequência sustentável, latência de transmissão e tracking físico não estão validados. Os FPS do runtime de teste não constituem medição PCVR real. O pacote é uma aplicação de PC, não um APK.

No PC de destino, extrair o ZIP, selecionar VirtualDesktopXR/VDXR no Streamer, ligar Quest 3 e executar Launch-VR.cmd. Confirmar estéreo, orientação/posição da cabeça, Runtime: VDXR no overlay, estabilidade por alguns minutos e alterações de predefinição/JSON. Guardar logs/vr.log, logs/xr-probe.log e, em caso de erro de runtime, o OpenXR.log do Virtual Desktop.

A reprodução do ambiente multicolorido, materiais, efeitos volumétricos e coreografia do Fantasynth continua pendente. Não há controladores Quest mapeados, análise de áudio nem ponte LLM nesta versão. O controlo JSON é a interface preparada para essa futura ponte.

Documentação de referência: [VDXR](https://github.com/mbucchia/VirtualDesktop-OpenXR/wiki), [OpenXR-SDK](https://github.com/KhronosGroup/OpenXR-SDK), [redistribuição Visual C++](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files).
