# Prioridade do estalo sobre o lápis

15 de outubro de 2026. O utilizador reportou que preparar/executar um estalo também iniciava escrita com o indicador. A partir de `index-only-trail-3`, os dois efeitos têm prioridade por mão quando ativados em conjunto. Sem commit/push; cópias delimitadas anteriores em `backup/`.

O contacto polegar–médio em preparação de estalo reserva imediatamente essa mão, antes dos 45 ms necessários para armar o disparo. Para distinguir a preparação de um polegar pousado no médio já recolhido, exige-se que a ponta e a distal do médio estejam pelo menos a 40/25 mm da palma, ajustados à escala da mão. A reserva fica mantida até abandonar o candidato; esta distinção só governa a prioridade, sem alterar o disparo do estalo. O lápis não inicia nem continua durante a preparação, durante a janela de libertação armada, nem nos 550 ms após um estalo reconhecido (intervalo já usado pelo detetor). Não se exige que o estalo chegue a disparar para bloquear durante a preparação. Uma preparação abandonada liberta a mão quando o detetor abandona o candidato.

Se já havia escrita, termina com evento `ended`/`snap_priority`; o traço existente desvanece normalmente. A candidatura ao lápis e o filtro são interrompidos. Depois do bloqueio, é necessária nova confirmação da pose durante 40 ms e começa outro segmento, sem ligar posições através do estalo. O bloqueio não afeta a outra mão e o tempo vem de XR, incluindo a pausa musical. A perda de seguimento conserva as interrupções existentes; não permite contornar o intervalo após disparo.

Os limiares, deteção e aspeto do estalo não foram alterados. O lápis usado sozinho conserva a pose/filtro da versão anterior. Prioridade automática apenas com `--hand-snap` e `--hand-light-pencil` juntos; sem novas opções. Manifesto regista `snap_priority_enabled`, `snap_cooldown_ms` e versão do lápis. Configuração da raiz, áudio, anéis e restante direção artística preservados.

## Testar

Build Release de desenvolvimento: `C:\DEV\Prismatic\build\Release\fantasynt.exe`. Na raiz:

```powershell
.\build\Release\fantasynt.exe --config .\visual-config.json --vr --paused --hand-snap --hand-light-pencil
```

Preparar/manter o contacto polegar–médio com o indicador esticado: não deve escrever nessa mão. Executar o estalo: deve aparecer só a pequena explosão. Depois de aproximadamente meio segundo, voltar a apontar para escrever. Experimentar também começar pelo lápis e preparar o estalo, e escrever com uma mão enquanto a outra estala. Espaço inicia/pausa a experiência; os efeitos das mãos funcionam em pausa. É criado automaticamente um novo registo em `logs/`.

## Verificação

Resultados e hashes em `verification.json`; logs nesta pasta. Teste de regressão usa deliberadamente uma pose elegível para o lápis tanto no contacto como após libertar o médio. Com prioridade, não há início de lápis; com apenas o lápis, a mesma pose inicia. Outro teste integrado verifica terminar escrita já ativa, emitir o estalo e só retomar após 550 ms + nova confirmação. Os testes anteriores de coexistência usavam poses separadas e não cobriam esta sobreposição real; permanecem como regressão dos casos separados.

Release recompilada e revista: **22/22 CTest**, **um teste gráfico separado** e **99/99 testes Python** aprovados. Capturas em `temp/hand-priority-8d0ab7aa/`, revistas visualmente: durante o estalo, a imagem com ambos os efeitos é idêntica à de estalo isolado; após o intervalo, a escrita retoma. Os três registos passam o inspetor de integridade. SHA-256 do executável: `04693aeef7b73c4105e98312d7b2b30229ca844abebe41f7b9809f1fff70319f`. MASTER e configuração da raiz conservam os hashes iniciais. Evidência histórica preservada; nenhuma medição física nova.

Testes C++ verificam reserva no primeiro frame de contacto, preparação abandonada, intervalo após disparo/perda, independência das mãos e segmentos separados. Compilação e testes são locais/simulados. A separação percetiva dos gestos e eventual ajuste do intervalo precisam de novo ensaio no Quest; não se declaram fisicamente validados.
