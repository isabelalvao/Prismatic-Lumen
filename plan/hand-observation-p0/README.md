# Aquisição e registo P0

Implementado em 2026-10-14, na compilação de desenvolvimento `build/Release/fantasynt.exe`. Sem política adaptativa ou representação visual das mãos. Os pacotes históricos permanecem preservados.

Pacote de teste preparado posteriormente a pedido do utilizador, agora com [pasta portátil r2](../../dist/Prismatic-Lumen-Q3-HandTracking-P0-2026-10-14-r2/) e [ZIP r2](../../dist/Prismatic-Lumen-Q3-HandTracking-P0-2026-10-14-r2.zip), com perfil atual da raiz, CRT x64, loader, recursos e scripts de diagnóstico/gravação/verificação/inspeção. Não substitui os ZIP históricos. No PC de destino, usar `Diagnose-VR.cmd` e `Test-HandTracking.cmd`; o resumo básico não exige Python. A verificação portátil em PowerShell5.1 e diretório com acentos está em [package-check-r2.json](package-check-r2.json).

Para corrigir o pacote original sem copiar novamente os recursos, extrair a [atualização de ferramentas](../../dist/Prismatic-Lumen-Q3-P0-Inspection-Update-2026-10-14.zip) na pasta que contém `fantasynt.exe` e `logs`. Acrescenta o inspetor, o seu módulo, o atalho `Fantasynt.cmd` e instruções/verificador revistos. Não contém executável, configuração ou registos. Os hashes dos ficheiros substituídos constam de `inspection-update.json`; o manifesto original conserva os hashes anteriores.

Na pasta do dist, sem referência à raiz do repositório:

```powershell
python .\tools\inspect_session.py .\logs\hand-tracking-01.jsonl --output .\logs\hand-tracking-01.analysis.json
```

Python 3.10+; sem bibliotecas adicionais. O relatório distingue percentagens de amostras, estimativas temporais nos intervalos até 250ms e lacunas maiores desconhecidas. Perdas são segmentadas quando há lacunas; não se declara recuperação através de dados ausentes. Inclui validade por articulação, estados, mudanças de origem, velocidades, duração de consulta e conversão QPC. Os dois últimos valores não medem latência física.

## Relato do teste real e identidade

O relato enviado em 2026-10-14 identifica VirtualDesktopXR, trackers preparados e dados válidos/seguidos das duas mãos nas quatro observações iniciais consultadas. Reporta `recording_result`: accepted=3859, complete=true, dropped=0, write_failed=false; FPS médio=119,4105 em 211,4639s. Estes valores são reportados pelo utilizador, não recalculados localmente a partir do registo completo.

O SHA-256 reportado coincide com o executável da compilação e dos pacotes P0: `d1c09d8a695a1af5d0833c6a6653625cfa8c74de9e02e36317795089407b66b0`. `bc86e22` é o commit Git de base, cujo conteúdo ainda não inclui a aquisição P0; esta compilação contém alterações locais não comprometidas. r2 só altera ferramentas/instruções, mantendo o executável. O formato schema_version=1 corresponde ao serializador atual.

O anexo recebido em 2026-10-14 continha apenas o relato, sem o JSONL. **Atualização de 2026-10-15:** o utilizador localizou o registo completo em Downloads; foi copiado sem alteração para `logs/hand-tracking-01.jsonl` e analisado. Integridade válida, 3 858 observações em 211,414 s, nenhuma lacuna de publicação acima de 250 ms; posição válida/seguida em 100% das amostras da cabeça, 81,57% da esquerda e 60,32% da direita. Há oito perdas/recuperações da esquerda e 38 da direita, além de picos de velocidade que não estão fisicamente validados. [Análise completa e limites](../migration-2026-10-14/xr-validation-2026-10-15/README.md). O utilizador não consegue confirmar os momentos de oclusão deliberada/recentragem e aceita adiar a afinação até haver mãos 3D visíveis para diagnóstico.

## Executar no Quest 3

Na raiz `C:\ORCA\Fantasynt`, com o PCVR ligado e runtime OpenXR disponível:

```powershell
.\build\Release\fantasynt.exe --xr-probe
.\Fantasynt.cmd --vr --paused --record-session logs\quest-p0-01.jsonl
```

O diagnóstico anuncia extensão/capacidade de mãos; o relatório de arranque confirma se os trackers foram criados. Um runtime sem mãos não é apresentado como funcionalidade operacional. O caminho de gravação deve ser novo; a aplicação recusa sobrescrever um ficheiro.

Espaço inicia/pausa a experiência; Esc fecha e finaliza o ficheiro. Para o ensaio técnico, mover cabeça/mãos, permanecer quieto, ocultar uma mão e ambas, recuperar o seguimento e testar recentragem. Não há comandos por gesto. A pausa audiovisual existente não congela o seguimento corporal e as transições manuais v1; isto permanece separado do executor semântico futuro.

Depois de fechar:

```powershell
python tools\inspect_session.py logs\quest-p0-01.jsonl
```

O inspetor valida estrutura/encerramento, IDs/tempos, contagens e reconstitui velocidades a partir das observações publicadas. `valid_record=true` significa integridade e equivalência de cálculo; não prova disponibilidade física de todas as fontes, latência percetiva ou efeito de agência. Verificar também `tracking.hand_trackers_ready` e os contadores de validade/atividade.

## Conteúdo do JSONL

- Manifesto: sessão, condição `observe_only`, versão/commit, SHA-256 de executável/configuração/áudio, configuração efetiva, runtime e capacidades.
- Observações: pose física da cabeça em VIEW relativo a LOCAL, 26 articulações por mão em metros, validade/seguimento separados, estado ativo/ausente/erro e origem segmentada.
- Relógios: tempo XR previsto, receção monotónica/QPC e frequência; conversão XR→QPC quando a extensão existe. Sem ela, conservar apenas pares de receção e declarar a limitação, sem subtrair relógios diferentes.
- Ambiente: tempo consumido SDL e instante da consulta, câmara virtual separada, parâmetros antes/depois da direção artística, pesos/fade, pulsos, ganho 1 e ausência de contribuição semântica. Em frames sem desenho, `render_submitted=false`.
- Mudanças de configuração/comandos e encerramento; fila SPSC limitada com escritor separado. Saturação/erro de escrita não bloqueiam a renderização; contam como perda/integridade incompleta. A sincronização final do ficheiro é confirmada pelo relatório `recording_result`, não apenas pelo marcador de encerramento.

Cadência alvo de publicação 20 Hz; perdas/recuperações de validade, mudança de origem ou estado técnico são publicadas imediatamente. Derivadas usam as amostras publicadas e tempo XR, nunca o avanço virtual; não atravessam perda, mudança de origem ou intervalo acima de 250 ms. Velocidade 0 com seguimento válido difere de velocidade ausente. Nenhuma emoção/intenção é inferida.

## Verificação realizada e pendente

Compilação Release concluída. CTest: 19/19; integração de observação: 13/13; integração VR anterior: 10/10. Incluem ausência de extensão/capacidade, falha de criação/localização, mãos ocultas, validade parcial da cabeça, recentragem no instante correto, perda XR, conversão temporal, preservação de ficheiros existentes e reconstituição. Revisão independente sem problemas críticos/altos. Uma cena fixa produziu o mesmo PNG com gravação ligada e desligada.

Exemplo produzido exclusivamente no runtime simulado: [example-session.jsonl](example-session.jsonl) e [example-inspection.json](example-inspection.json). Não contém movimentos de uma pessoa nem prova de funcionamento no Quest.

**Limitação desta máquina:** o diagnóstico local falhou porque o Windows referencia `C:\Program Files\Oculus\Support\oculus-runtime\oculus_openxr_64.json`, inexistente. Também não foi encontrada instalação em `C:\Program Files\Virtual Desktop Streamer`. Não foram alterados o registo Windows ou o runtime ativo. O teste no outro PC reportado pelo utilizador confirma disponibilidade inicial em VDXR; o ficheiro completo não foi enviado para análise local.

P0 está implementado e testado em simulação, mas o marco completo continua dependente de registo no Quest, cobertura real e comparação de tempos de frame/latência física. A próxima implementação adaptativa será o executor local de motivos, depois dessa verificação. Não há Jev, modulação de ganho ou novas políticas nesta entrega.

Revisão de inspeção: 15 testes controlados e 13 testes de integração aprovados. O dist r2 passa os cenários de aquisição normal, ausência de mãos e falha XR com o inspetor distribuído. A atualização pequena passa extração sobre o ZIP original, preservação independente do executável/recursos/JSONL, hashes e execução em pasta com acentos: [inspection-update-check.json](inspection-update-check.json). Esta evidência usa o runtime simulado, não o registo do utilizador.
