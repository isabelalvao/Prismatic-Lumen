# Revisão

- Alterações comparadas com as cópias desta tarefa. Sem alterações aos limiares de classificação, movimento dos anéis, áudio ou configurações.
- A arbitragem reutiliza o estado já calculado do estalo, atualizado antes do lápis no mesmo frame. Não duplica o detetor nem introduz decisões por dados ausentes.
- Reserva começa no contacto inicial de preparação, antes de armar; ponta/distal a pelo menos 40/25 mm da palma (com escala) distinguem o polegar pousado no médio já recolhido. Esta distinção só afeta a prioridade, sem mudar o disparo. Mantém a janela de libertação e o intervalo de 550 ms existente; perde-se com o candidato abandonado e conserva-se durante a perda no intervalo após disparo.
- Fim de escrita tem motivo próprio, não se repete por frame, preserva desvanecimento e exige nova candidatura. A passagem pelo bloqueio não liga segmentos. Cada mão tem estado próprio.
- Sem `--hand-snap`, o vetor de bloqueio é falso e o lápis mantém a lógica anterior. Com apenas estalo, não se executa o lápis. A ordem de atualização está explícita no ponto de integração.
- Regressão nova reproduz sobreposição de poses que os testes anteriores não cobriam, sem usar o JSONL histórico como calibração. Quest ainda necessário para validar o comportamento real.
