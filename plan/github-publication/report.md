# Publicação de Prismatic Lumen — relatório

## Resultado

Repositório privado criado e carregado na conta JoaoAz:

- [Prismatic Lumen](https://github.com/JoaoAz/Prismatic-Lumen)
- [Release 0.12.0](https://github.com/JoaoAz/Prismatic-Lumen/releases/tag/v0.12.0)
- Branch principal: `master`.
- Commit inicial: `c3f687d9ffdad2a3eae20adf6a4f9eb1d72848e5`.

O nome de apresentação foi atualizado no README e no GitHub. Os identificadores técnicos, executáveis e lançadores Fantasynt permanecem compatíveis com a versão existente. A visibilidade privada foi expressamente confirmada pelo utilizador.

## Conteúdo

A publicação inicial contém 4 035 ficheiros, incluindo 88 documentos Markdown. Este relatório e as cópias de segurança documentais são acrescentados num commit de conclusão. O [índice documental](../../docs/INDEX.md) inclui toda a documentação Markdown versionada.

O código, configurações, exemplos, ferramentas próprias, relatórios, planos, dados de investigação, capturas, vídeos e áudio necessário à execução foram conservados. Os 2 047 caminhos LFS referenciam 1 909 objetos únicos, todos confirmados disponíveis no servidor depois do envio. As diferenças entre estes números resultam de recursos duplicados em evidências e cópias históricas.

Não integram o histórico as compilações locais, caches, dependências descarregadas, ferramentas binárias externas ou os diretórios temporários. A árvore documental permanece acompanhada pelas evidências, exceto artefactos de ferramentas/dependências abrangidos pelas exclusões descritas no índice.

O último prompt e as decisões de investigação desta conversa foram preservados em `plan/semantic-repertoires-v1/next-step-prompt.md` e `research-context.md`. Não foi executado o desenho pedido por esse prompt nem implementada a infraestrutura semântica durante a publicação.

## Pacote portátil

A Release publica o pacote Windows/PCVR existente 0.12.0, com o nome de recurso `Prismatic-Lumen-PCVR-Win11-x64-0.12.0.zip`, e um ficheiro SHA-256 correspondente. O conteúdo interno do ZIP permanece o pacote original. O ZIP tem 82 697 346 bytes e contém 144 entradas, incluindo lançadores VR/desktop, executável, configuração e loader OpenXR.

Este pacote não implementa ainda Jev/LLM nem o seguimento dedicado das mãos planeado. Não foi recompilado na publicação.

## Validação

- 16 testes CTest aprovados sobre os binários existentes, abrangendo áudio, núcleo, composição, geometria, configuração, controlo, rede e matemática VR.
- O teste de captura `renderer_resize` não foi executado para evitar sobrescrever a evidência histórica nessa publicação documental.
- Verificação local `git lfs fsck` concluída com sucesso após o commit inicial.
- Verificação remota dos 1 909 objetos LFS únicos, em pedidos limitados a 100 objetos.
- Inventário remoto de 4 035 ficheiros igual ao inventário do commit inicial local, sem truncagem da árvore GitHub.
- SHA do commit remoto igual ao SHA local e README remoto com o título Prismatic Lumen.
- Recursos de Release em estado `uploaded`, tamanho do ZIP confirmado e digest remoto compatível com o SHA-256 local.
- Verificações de padrões conhecidos de credenciais não detetaram ocorrências nos ficheiros de texto candidatos. Credenciais utilizadas apenas em memória e pelo gestor de credenciais Git; não foram incluídas no repositório.

Os testes não constituem nova validação no headset, nem evidência percetiva de agência.

## Recuperar e continuar

```powershell
git lfs install
git clone https://github.com/JoaoAz/Prismatic-Lumen.git
cd Prismatic-Lumen
git lfs pull
```

O repositório privado exige autenticação autorizada. O primeiro passo de desenvolvimento continua descrito no plano dos repertórios e no prompt de primeira exposição. As cópias do README e das regras de exclusão anteriores à publicação estão em `backup/`.
