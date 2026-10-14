# Publicação de Prismatic Lumen

## Objetivo

Publicar o código, recursos de execução e toda a documentação/evidência associada na conta GitHub JoaoAz, com o nome Prismatic Lumen.

## Etapas

- [x] Inspecionar a pasta, confirmar autenticação e inventariar recursos/documentação.
- [x] Atualizar o nome de apresentação, regras Git/LFS e preservar o contexto de investigação.
- [x] Criar índice documental e verificar conteúdo a versionar.
- [x] Validar a aplicação existente e preparar o commit inicial.
- [x] Criar repositório, enviar Git/LFS e disponibilizar o pacote portátil.
- [x] Confirmar conteúdo remoto e entregar a ligação.

## Decisões

- Nome de apresentação: Prismatic Lumen. Nome GitHub: Prismatic-Lumen.
- Identificadores técnicos anteriores preservados para compatibilidade.
- Visibilidade privada, expressamente confirmada pelo utilizador.
- Toda a árvore documental `plan/` é conservada, incluindo capturas e registos; caches, dependências instaladas e binários de ferramentas são excluídos.
- Git LFS conserva os recursos audiovisuais e binários de evidência. Pacote portátil atual é um recurso de Release.

## Ocorrências resolvidas

- Token de ambiente recusado (401); autenticação JoaoAz confirmada através do gestor de credenciais Git.
- GitHub CLI não encontrado; usar API GitHub autenticada e Git instalado.
- Erro de sintaxe num inventário temporário; corrigido e inventário concluído.

## Estado

Concluído: repositório privado https://github.com/JoaoAz/Prismatic-Lumen e Release v0.12.0 publicados. Git, objetos LFS, inventário remoto, README e checksum do pacote verificados. 16 testes CTest dos binários existentes aprovados. Ver `report.md` para conteúdo e limites da validação. Nenhuma lógica de execução foi alterada.
