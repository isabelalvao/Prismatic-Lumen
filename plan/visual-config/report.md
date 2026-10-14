# Configuração visual editável — 0.7.0

Pedido: iniciar movimento, animação e iluminação sem atraso e permitir afinação visual sem recompilar. Implementação concluída em 2026-10-13.

O arranque é automático, sem espera nem rampa inicial. A presença luminosa começa reduzida e cresce; as formas arredondadas continuam ligadas aos pulsos graves. A entrada das pirâmides mantém-se aos 80 s. Espaço pausa/retoma; Q reinicia; F5 força a releitura.

`visual-config.json` contém 150 opções numéricas, booleanas e enumeradas, dez cores, sete parâmetros globais e a escolha do áudio. Abrange percurso, animações, cronologia, distribuição e corredor, famílias geométricas, materiais, detetor de graves, céu, névoa, chão/reflexos, pós-processamento e projeção. As unidades e limites estão em `assets/visual-config-guide.md`.

Na pasta do projeto, `Fantasynt.cmd` usa o JSON da raiz. No pacote portátil, usa-se o JSON junto do executável. As alterações válidas aplicam-se ao guardar, por defeito com verificação a cada 0,25 s. Alterações às velocidades preservam posição da câmara e fases das animações. A distribuição pode mudar imediatamente ao alterar o layout. A preparação do detetor suspende áudio e animação em conjunto. Um JSON inválido conserva o estado anterior e produz um erro no registo.

Resolução VR, dimensões da janela, cena e reprodução inicial e caminho do áudio requerem reiniciar. A posição inicial da câmara aplica-se com Q. Malhas novas e efeitos novos ainda requerem desenvolvimento. O catálogo específico das famílias aplica-se a Opening; os controlos comuns também afetam as cenas de comparação. Não se declara fidelidade integral nem validação artística pelo utilizador.

## Alterações

- `src/config/visual_config.*` e `visual_settings.inc`: catálogo, validação estrita, serialização e observação do ficheiro.
- `src/app/main.cpp`: arranque imediato, seleção de configuração, recarga, F5, preservação do relógio e opções de arranque.
- `src/render/art_direction.*`, `visual_uniforms.*` e renderer: aplicação aos movimentos, fases, distribuição, câmara e iluminação; origem real por olho no descarte de geometria.
- Shaders Opening, chão e composição: parâmetros GPU para aparência e efeitos.
- Áudio: deteção de graves configurável e duração mínima correspondente à experiência selecionada.
- CMake, lançadores e empacotamento: instalação do catálogo e guia, preservação do JSON afinado nas compilações, pacote com valores iniciais separados.
- Testes C++ e Python, README e guia portátil atualizados. Cópias de segurança em `plan/visual-config/backup`.

## Verificação

Dez testes C++ e 40 testes Python passaram (`verification.log`). Foram exercitados arranque imediato, validação e rejeição sem alterar o estado, recarga efetiva, continuidade de movimentos, efeitos gráficos das cores, áudio curto compatível, integração JSON de rede e VR simulado. Os testes de arranque e do áudio curto foram registados a falhar antes das respetivas correções (`red.log`, `short-audio-red.log`).

A compilação incremental final passou. O ZIP final extraído passou os ensaios desktop, estéreo OpenXR simulado e cliente PowerShell JSON, com hashes do manifesto válidos (`package-check.json`). O arranque pelo JSON da raiz foi confirmado: sem pausa, resolução VR 1,0 e percurso a 1,5 m/s já avançado 150 cm no instante 1 s (`root-config-check.log`). A cópia de recursos preservou um JSON previamente afinado (`config-preserve-check.log`). Revisão independente aprovada; a nota do guia sobre altura desktop foi clarificada.

O headset físico não foi ensaiado nesta versão; o desempenho no equipamento final e a avaliação artística ficam para o teste do utilizador.

## Entrega e preservação

ZIP: `dist/Fantasynt-PCVR-Win11-x64-0.7.0.zip`.

SHA-256: `0adffd18cfaa2ecf7cb952044a966299d445f69117a9af98b36795a592d0cbee`.

Os ZIPs anteriores foram preservados. SHA-256 da 0.6.0 confirmado: `682b09e0b2b2df93f20f47f3226041c2d61d61c890b2eeee74db8d9079ea3b7c`.

Áudio MASTER preservado: `60dc74309ab1fbae42394228ddfc098a09a1e7a36ae739c7f5ee7e1adafb9fd9`. Jogo Steam, UE 4.13 e recursos recuperados não foram alterados. Não existe repositório Git e não foi inicializado. Os diretórios temporários do empacotamento e verificação foram removidos pelos respetivos scripts.
