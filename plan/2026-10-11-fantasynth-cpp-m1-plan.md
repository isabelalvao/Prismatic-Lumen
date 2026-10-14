# Fantasynt C++ — plano de implementação M1

> **Para quem executar:** usar `superpowers:executing-plans` na execução nativa, ou `superpowers:subagent-driven-development` se o utilizador escolher delegação. Implementar cada tarefa com os testes abaixo e rever o conjunto antes da entrega.

**Objetivo:** entregar uma aplicação Windows C++ com uma cena procedural luminosa, câmara móvel, pulsação sintética e parâmetros alteráveis por JSON durante a execução.

**Arquitetura:** núcleo de controlo independente da GPU; transporte TCP local; renderer bgfx integrado numa janela SDL3. O ciclo principal aceita comandos validados e atualiza as transições enquanto a thread de rede gere a receção e as respostas.

**Tecnologias:** C++20, SDL3 3.2.30, bgfx.cmake e submódulos fixados por commit, nlohmann/json 3.12.0, CMake, MSVC x64, CTest. Cliente de demonstração Python com biblioteca padrão.

**Especificação:** [desenho C++](2026-10-11-fantasynth-cpp-design.md).

**Método recomendado:** execução nativa nesta sessão, com revisão independente do código no fim. Os componentes partilham interfaces pequenas e podem ser implementados e verificados por ordem. O repositório ainda não contém código do produto.

**Estado em 2026-10-12:** M1 implementado e verificado; detalhes, limitações de medição e comparação visual em [relatório de validação](2026-10-12-m1-validation.md). As verificações de tamanho zero usam directamente o renderer; não foi automatizada a minimização da janela visível.

## Restrições globais

- Windows x64; C++20; primeiro backend gráfico Direct3D 11.
- Monitor primeiro; VR, áudio de ficheiro e ligação ao LLM pertencem a marcos posteriores.
- TCP `127.0.0.1:47631`; porta alterável por argumento de arranque.
- JSON UTF-8, delimitado por LF; aceitar CRLF. Máximo 16 KiB por mensagem, sem delimitador.
- Fila de entrada limitada a 64 mensagens; fila cheia produz rejeição explícita.
- Mensagens rejeitadas não alteram valores nem revisão; atualizações válidas são parciais e atómicas.
- Transições entre 0,1 e 15 segundos, dependentes do tempo decorrido e contínuas quando interrompidas.
- Até 4 096 elementos; cena reproduzível com semente `20261011`.
- Recursos gráficos e shaders próprios; artefactos da investigação separados da aplicação.
- Sem chamadas ao LLM no ciclo de renderização.
- Dependências fixadas pelos commits da especificação; não obter revisões flutuantes durante a compilação.
- Comentários e documentação próprios em português; identificadores em inglês.

## Casos a verificar na revisão

1. TCP fragmentado ou com vários comandos juntos: cada mensagem completa é processada uma vez e os fragmentos não são interpretados como JSON autónomo.
2. Atualização a meio de uma transição: não há salto para um estado anterior, nem reposição dos campos omitidos.
3. Janela minimizada, redimensionada ou de tamanho zero: não são criados alvos inválidos e a rede mantém-se funcional.
4. Cliente desligado ou saída da aplicação: sockets, thread de rede e recursos gráficos são libertados sem bloquear o encerramento.
5. JSON aparentemente válido com valores de tipo errado, chaves repetidas, sequência UTF-8 inválida ou limites ultrapassados: rejeição completa e estado intacto.

## Mapa dos ficheiros

| Ficheiro ou grupo | Responsabilidade |
| --- | --- |
| `CMakeLists.txt`, `cmake/Dependencies.cmake` | Compilação reproduzível, dependências fixadas e opção de testar sem GPU. |
| `src/control/parameters.hpp` | Tipos do estado, comando parcial e resultado de validação. |
| `src/control/protocol.hpp`, `.cpp` | Leitura JSON, validação e recibos. |
| `src/control/scene_controller.hpp`, `.cpp` | Estado atual/pretendido, revisão e transições. |
| `src/net/line_framer.hpp`, `.cpp` | Enquadramento LF, limites e recuperação após excesso. |
| `src/net/control_server.hpp`, `.cpp` | Servidor loopback, filas limitadas e ciclo de vida da thread. |
| `src/render/renderer.hpp`, `.cpp` | Contexto bgfx, pipeline, redimensionamento e captura de frame. |
| `src/render/geometry.hpp`, `.cpp` | Grelha procedural e instâncias reproduzíveis. |
| `src/render/post_process.hpp`, `.cpp` | Alvos HDR, extração de brilho, desfoque e composição. |
| `src/app/main.cpp` | Argumentos, janela SDL3, eventos e coordenação. |
| `shaders/` | Shaders próprios da cena e do pós-processamento. |
| `tests/` | Testes de comportamento e integração. |
| `tools/build.ps1`, `tools/send_command.py` | Compilação e cliente de controlo. |
| `examples/` | Comandos para estados calmo, equilibrado e energético. |
| `README.md`, `.gitignore` | Instruções, limites do protótipo e exclusão de artefactos de compilação. |

## Tarefa 1 — contrato e transições sem GPU

**Criar:** ficheiros de compilação, `src/control/` e `tests/control_tests.cpp`.

**Interfaces:**

```cpp
namespace fantasynt {
struct Color { float r, g, b; };
struct Parameters {
    std::array<Color, 2> palette;
    float light_intensity, motion_speed, pulse_strength;
    float fog_density, geometry_density, bloom;
};
struct ParameterPatch {
    std::optional<std::array<Color, 2>> palette;
    std::optional<float> light_intensity, motion_speed, pulse_strength;
    std::optional<float> fog_density, geometry_density, bloom;
};
struct Command { std::string id; double transition_seconds; ParameterPatch parameters; };
struct ValidationError { std::string code; std::vector<std::string> fields; };
using ParseResult = std::variant<Command, ValidationError>;
ParseResult parse_command(std::string_view payload);
class SceneController {
public:
    SceneController();
    void submit(const Command& command);
    void advance(double elapsed_seconds);
    const Parameters& current() const;
    const Parameters& target() const;
    std::uint64_t revision() const;
};
}
```

As cores são armazenadas em RGB linear; a leitura `#RRGGBB` faz a conversão sRGB → linear. Os valores iniciais seguem a especificação. `advance` recebe duração não negativa e finita de um frame; uma duração inválida é ignorada, sem contaminar o estado. Os comandos só chegam a `submit` depois da validação.

- [x] Escrever testes de aceitação de um comando parcial, rejeição de campos desconhecidos e rejeição de tipos booleanos em parâmetros numéricos.
- [x] Criar apenas as declarações e implementações mínimas que permitam executar os testes; observar a falha por comportamento ainda não implementado antes de implementar o analisador.
- [x] Implementar a leitura com nlohmann/json. Detetar chaves duplicadas durante a análise antes de perder essa informação no objeto JSON; manter um conjunto de chaves por objeto.
- [x] Testar versão igual a 1 inteira, identificadores de 1–64 caracteres, limites de transição, cor hexadecimal, pelo menos um parâmetro, valores não finitos, UTF-8 inválido e tamanho máximo.
- [x] Implementar transições e testar valores de início, ponto intermédio e destino; depois interromper uma transição com uma atualização parcial.

Exemplo de teste de continuidade; `CHECK` e `CHECK_NEAR` são utilitários dos testes que acumulam falhas e devolvem código de saída não nulo:

```cpp
SceneController scene;
Command first{"first", 2.0, {}};
first.parameters.light_intensity = 0.0f;
scene.submit(first);
scene.advance(1.0);
const auto mid = scene.current().light_intensity;
Command second{"second", 2.0, {}};
second.parameters.light_intensity = 1.0f;
scene.submit(second);
CHECK_NEAR(scene.current().light_intensity, mid, 0.0001f);
CHECK_NEAR(scene.target().fog_density, 0.2f, 0.0001f);
scene.advance(2.0);
CHECK_NEAR(scene.current().light_intensity, 1.0f, 0.0001f);
CHECK(scene.revision() == 2);
```

Comandos de verificação na máquina atual:

```powershell
$cmakePath = 'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
& $cmakePath -S . -B build -G 'Visual Studio 18 2026' -A x64 -DFANTASYNT_BUILD_APP=OFF
& $cmakePath --build build --config Release
& $cmakePath --build build --config Release --target RUN_TESTS
```

**Resultado verificável:** `fantasynt_control_tests` passa sem SDL, bgfx, janela ou instalação de Ghidra.

## Tarefa 2 — transporte local e cliente manual

**Criar:** `src/net/`, `tests/network_tests.cpp`, `tools/send_command.py`, exemplos JSON.

**Interfaces:**

```cpp
struct FrameResult { std::optional<std::string> payload; bool oversized; };
class LineFramer {
public:
    std::vector<FrameResult> feed(std::string_view bytes);
};
struct PendingMessage { std::uint64_t client_id; std::string payload; };
class ControlServer {
public:
    explicit ControlServer(std::uint16_t port);
    std::uint16_t port() const;
    std::vector<PendingMessage> drain();
    void reply(std::uint64_t client_id, std::string payload);
    ~ControlServer();
};
```

Permitir a porta `0` apenas para seleção automática nos testes. `client_id` identifica uma ligação ainda ativa; respostas a ligações entretanto terminadas são descartadas. `drain` entrega as mensagens completas à aplicação; a aplicação chama `parse_command` e responde só depois de aceitar ou rejeitar o comando. Usar Winsock no Windows e RAII para os recursos.

- [x] Escrever e observar falhar um teste de enquadramento fragmentado e um teste de recuperação após uma mensagem de 16 385 bytes.
- [x] Implementar o enquadramento, incluindo CRLF e várias mensagens na mesma leitura; limitar a memória do fragmento em construção.
- [x] Escrever testes reais com sockets loopback: envio fragmentado, dois comandos juntos, ligação fechada a meio da mensagem, saturação da fila e porta ocupada.
- [x] Implementar o servidor com filas de entrada e saída limitadas; não manter uma thread dependente de uma receção bloqueante sem mecanismo de paragem.
- [x] Implementar um cliente Python que lê um ficheiro JSON UTF-8, envia uma linha, espera um recibo até ao tempo limite e devolve código de saída não nulo em rejeição, falha de ligação ou tempo limite.
- [x] Verificar a libertação da porta após o destrutor e o encerramento em menos de dois segundos sem cliente ligado.

Teste mínimo do enquadramento:

```cpp
LineFramer framer;
CHECK(framer.feed("{\"id\":").empty());
auto frames = framer.feed("\"a\"}\r\n{\"id\":\"b\"}\n");
CHECK(frames.size() == 2);
CHECK(frames[0].payload.value() == "{\"id\":\"a\"}");
CHECK(frames[1].payload.value() == "{\"id\":\"b\"}");
```

**Resultado verificável:** CTest passa os testes de transporte sem GPU e o cliente recebe confirmações de uma instância de teste do controlador.

## Tarefa 3 — cena, shaders e aplicação gráfica

**Criar:** `src/render/`, `src/app/main.cpp`, `shaders/` e `tests/geometry_tests.cpp`.

**Interfaces:**

```cpp
struct Instance { std::array<float, 16> transform; float color_mix; float phase; };
std::vector<Instance> make_instances(std::uint32_t seed, std::size_t count);
class Renderer {
public:
    Renderer(void* native_window, std::uint16_t width, std::uint16_t height);
    void resize(std::uint16_t width, std::uint16_t height);
    void draw(const Parameters& state, double time_seconds, double camera_position);
    void request_capture(const std::filesystem::path& path);
    ~Renderer();
};
```

- [x] Testar primeiro a geração de geometria: a mesma semente produz as mesmas instâncias, sementes diferentes alteram a distribuição e a contagem nunca excede 4 096.
- [x] Integrar SDL3/bgfx, passando o identificador de janela nativa da SDL3. Compilar shaders próprios através de shaderc para Direct3D 11.
- [x] Criar uma grelha de colunas/formas simples com materiais emissivos, percurso contínuo de câmara e pulsação sintética a 120 BPM.
- [x] Implementar os sete parâmetros definidos na especificação; as alterações de densidade têm orçamento limitado por frame.
- [x] Implementar alvo HDR/profundidade, extração de brilho, desfoque horizontal/vertical, nevoeiro por distância e composição com tone mapping.
- [x] Implementar redimensionamento, pausa da renderização em dimensão zero e captura PNG através do renderer.
- [x] Coordenar a rede e o controlador no ciclo principal. O recibo inclui identificador e revisão; aceitação não significa fim da transição.
- [x] Suportar `--port`, `--frames`, `--hidden` e `--capture` para verificações limitadas, além de execução interativa normal.
- [x] Teclas: `1` calmo, `2` equilibrado, `3` energético, `Space` pausa da câmara, `R` estado inicial, `Esc` sair.

Compilar a aplicação:

```powershell
& $cmakePath -S . -B build -G 'Visual Studio 18 2026' -A x64 -DFANTASYNT_BUILD_APP=ON
& $cmakePath --build build --config Release
& $cmakePath --build build --config Release --target RUN_TESTS
.\build\Release\fantasynt.exe --hidden --frames 240 --capture plan\m1-default.png
```

**Resultado verificável:** executável arranca, produz um PNG não vazio e termina após 240 frames com código zero. Inspecionar a imagem antes de descrever a qualidade visual como validada. Medir o desempenho separadamente; uma execução finita não demonstra a meta de 60 FPS.

## Tarefa 4 — integração, demonstração e documentação

**Criar:** `tests/test_end_to_end.py`, `README.md`, `tools/build.ps1`, `.gitignore` e registo de validação em `plan/`.

- [x] Escrever primeiro um teste de integração que lança o executável com janela oculta e duração limitada, envia uma atualização válida e espera `accepted`.
- [x] Enviar um comando inválido; verificar `rejected` e a manutenção da revisão através do recibo/telemetria da aplicação. Depois enviar outro comando válido e confirmar incremento de uma unidade.
- [x] Verificar que o processo permanece ativo durante uma transição e após o cliente desligar; observar a captura final para detetar frames vazios ou erros gráficos.
- [x] Gerar capturas dos três estados e comparar efeito de cada parâmetro. Registar quais critérios visuais foram efetivamente verificados.
- [x] Medir tempos de receção/aceitação, frames e transições a 1080p na GPU NVIDIA; guardar resumo das condições e valores medidos. Não substituir medições por estimativas.
- [x] Documentar compilação, execução, mensagens de exemplo, controlo manual, limites M1 e erros comuns, incluindo porta ocupada e shader em falta.
- [x] Confirmar que falhar o servidor ou inicialização gráfica mostra uma mensagem explícita e devolve código de saída não nulo.
- [x] Executar a suite CTest e a integração; fazer revisão independente do código, corrigir achados relevantes e repetir os testes afetados.
- [x] Limpar apenas os ficheiros temporários criados durante esta execução, preservando os diagnósticos e evidência de investigação.

Comandos de demonstração interativa:

```powershell
.\build\Release\fantasynt.exe
```

Num segundo terminal:

```powershell
python tools\send_command.py examples\calm.json
python tools\send_command.py examples\energetic.json
python -m unittest discover -s tests -p 'test_*.py' -v
```

**Resultado verificável:** executável utilizável, cliente manual funcional, testes com comandos reais, capturas verificadas e limitações documentadas. Ainda sem integração de LLM nem análise de WAV: são entregáveis M2/M3.

## Auto-revisão e cobertura

- Critérios 1–6 da especificação são cobertos por contrato, controlador, transporte e integração M1.
- Critério 7, análise de áudio, pertence a M2; a pulsação sintética M1 não é apresentada como deteção musical.
- Critério 8 é parcialmente preparado pela telemetria; reprodução completa de sessões pertence a M2.
- Critérios 9–10 pertencem à integração LLM e avaliação final M3; a comparação visual M1 verifica apenas a direção inicial.
- Os cinco casos de revisão acima têm testes atribuídos às tarefas 1–4; redimensionamento/minimização exige verificação gráfica adicional.
- Os exemplos JSON da especificação conservam os mesmos nomes e limites nesta mudança de tecnologia.
- A cadeia de compilação foi identificada, mas dependências e aplicação ainda não foram compiladas. Não existe repositório Git; o plano não pressupõe commits ou worktrees que ainda não existem.
