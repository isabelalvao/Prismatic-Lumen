# Evidência e decisões — reconstrução completa

Estado inicial: 0.3.0 tem túnel radial; o campo continua genérico. 19 recursos do original previamente lidos com hashes verificados. Shaders, render targets e coreografia integral não descodificados. Referências existentes: cinco screenshots oficiais e trailer de 40 s. As limitações não equivalem a autorização para parar o trabalho.

O utilizador confirmou 0.3.0 em VR e considera que estamos no caminho certo. Não há aprovação de fidelidade completa nem medições de latência/frequência.

A pesquisa atual encontrou a aplicação web Fantasynth de 2026 no site dos autores. É outro produto; não substituir o alvo instalado Fantasynth One de 2017, nem atribuir os seus mecanismos ao original.

## Implementação 0.4.0

timeline.json recupera PlaybackRange 424 s. DirectorSection associa 80 s ao index 3, 112 s ao index 4 e 224 s ao index 8. action-sequencers.json associa esses índices a Forest Start, Intro Stop e Tunnel Start. Não houve tracing da invocação real; a utilização destes tempos é uma inferência estática identificada como tal.

src/render/sequence.cpp usa essas fronteiras e valores de FogInscatteringColor; janelas de transição e interpolação suaves são próprias. Não utiliza todos os tangentes/modos UE4 nem a trajetória original da câmara. O SceneController conserva a propriedade exclusiva dos parâmetros JSON, evitando reenvio automático de predefinições durante a sequência.

As novas malhas são próprias. Normais dos leques calculadas pelas tangentes, cones separados em sete frustums com caps emissivos, placas octogonais, diamantes suspensos e postes. Capturas finais e comparação com as cinco imagens oficiais registadas em full-fidelity-audit.md. Esta cobertura funcional não equivale a fidelidade integral.
