# Recuperação com UE4.13 — primeiro marco concluído

Instalação do utilizador: C:/Program Files/Epic Games/UE_4.13. Build.version: 4.13.2, CL3172292, ramo ++UE4+Release-4.13. O executável do Fantasynth contém ++UE4+Release-4.13-CL-3172292 nos offsets 30109008 e 41840164 UTF16. Correspondência de ramo/revisão observada; não exclui alterações específicas do jogo. Prova por leitura local em engine-version.json, não por nova análise REA.

## Resultados reais

- SM_Lower_1: exportação glTF+BIN com 121 vértices, 648 índices/216 triângulos, normais, tangentes e UV. Todos os índices válidos e dados finitos. Bounds comparados com metadados originais: extents 43.301266/12.500001/43.301253cm na base glTF. O exportador converte cm→m e os eixos Y/Z; a comparação não valida por si só os sinais/orientação. Material exportado é dummy_material_0: material original ainda não reconstruído.
- DiagonalLine32: PNG 256×256 RGB, 6 cores, carregamento e pixels verificados; imagem inspecionada visualmente. Uma textura não representa todos os materiais.
- ChezNous_MASTER: Ogg 14,939,326 bytes. Descodificação INTEGRAL com libvorbisfile_64.dll da instalação UE4, não apenas identificação de cabeçalho: 18,706,432 frames, estéreo 44100Hz, 424.182131519s. PCM16 total 74,825,728 bytes, RMS não nulo e digest guardado. Diferença para duração serializada: 2.613μs, abaixo de uma amostra. Não foi reproduzido no protótipo.

Logs export: mesh-export.log/texture-export.log/audio-export.log; saídas e hashes nos JSON correspondentes. sample-validation.json e audio-validation.json guardam verificações. source-payload-validation.json valida SHA1 dos três recursos diretamente no PAK contra inventário existente.

## Ferramentas realmente utilizadas

Extração através do UE Viewer/UModel oficial, build 1590, perfil explícito -game=ue4.13; compatibilidade observada nos três casos, sem generalização aos 1696 recursos. Download do site oficial devolveu HTTP404; binário adquirido do repositório oficial gildor2/UEViewer GitHub, com origem/Gitblob/SHA256 local em ueviewer-provenance.json. Não existe digest SHA256 publicado verificado; não o declarar.

UnrealPak 4.13 instalado listou 1696 ficheiros/257257886 bytes, em unrealpak-list.log, com código de saída 1. Resultado de listagem observado e conservado; significado do exit 1 não verificado. SourcesPrivate SoundWave.cpp/StaticMesh.cpp e headers foram consultados. A DLL Vorbis instalada fez a validação do áudio. Não se abriu nem modificou o projeto original.

Uma invocação UModel sem argumentos abriu a interface e atingiu prazo 15 s; o processo foi terminado pelo subprocess. A ajuda efectiva foi obtida com -help, exit 0. Pedidos seguintes foram exclusivamente CLI-export/list, com timeout, sem arranque do jogo.

## Continuação

- Exportar restantes famílias de malhas/texturas/áudio com a rota comprovada e inventário de falhas.
- Integrar geometria, materiais reconstruídos e reprodução/sincronização no C++.
- Recuperar distribuição e coreografia dos consumidores. Não foram resolvidas por estas três exportações.

O produto permanece 0.4.1: nenhum código, pacote ou contrato alterado nesta etapa. Recursos recuperados guardados em plan/ue413-recovery/sample-* para investigação. Instalações UE4 e Fantasynth preservadas; sem Git. O marco pequeno de extração foi atingido, não a fidelidade integral.
