crie forma de importar projetos criados pelo "digital logic sim", convertendo estes itens em padrão deste projeto.

crie forma de usuário transformar uma lógica em texto, com esse texto conseguindo descrever exatamente o que está acontecendo (como uma pseudolinguagem de código)
faça também que seja possível importar essa linguagem para o ambiente, garantindo que seja possível remontar exatamente o que existira antes.
crie também uma pequena documentação explicando os itens desta documentação.
essa linguagem deve comportar estruturas complexas e compeltas reais.

itens para adicionar: memória ROM, display

ao iniciar app, deve abrir página perguntando sobre qual projeto que quer abrir, antes de abrir algum projeto. garanta que o salvamento e o carregamento do projeto esteja funcionando corretamente.

erros ao iniciar o app ao clicar 2 vezes no exe diretamente:

simulator.exe - Erro do sistema
A execução de código não pode continuar porque libsfml-system-3.dll não foi encontrado. Reinstalando o programa para corrigir o problema.
simulator.exe - Erro do sistema
A execução de código não pode continuar porque libsfml-graphics-3.dll não foi encontrado. Reinstalando o programa para corrigir o problema.
simulator.exe - Erro do sistema
A execução de código não pode continuar porque libgcc_s_seh-1.dll não foi encontrado. Reinstalando o programa para corrigir o problema.

o app deve ser autossuficiente, tendo internamente tudo o que precisa.

Quero implementar um sistema de **Entradas e Saídas Unificadas (Barramentos)** no simulador.

Objetivo:
Substituir os componentes separados de múltiplas entradas e saídas (como 2 IN, 4 IN, 8 IN, 16 IN e equivalentes de saída) por apenas dois componentes: **Entrada** e **Saída**, que possam representar qualquer quantidade de bits.

Requisitos:

* Existirão apenas dois componentes:

  * Entrada (Input)
  * Saída (Output)

* Cada componente possuirá uma propriedade **Largura (Bit Width)** configurável pelo usuário.

* A largura poderá ser qualquer valor inteiro válido (por exemplo: 1, 2, 4, 8, 16, 32, 64...).

* Quando a largura for alterada:

  * o componente deve atualizar automaticamente sua quantidade de conexões (pinos);
  * a interface deve ser redesenhada dinamicamente;
  * todas as conexões devem permanecer válidas sempre que possível.

* O componente deve exibir visualmente a largura configurada.

* Cada bit deve possuir seu próprio pino físico para conexão, mantendo compatibilidade com componentes de 1 bit.

* O sistema deve permitir conectar barramentos entre componentes compatíveis.

* Caso duas conexões possuam larguras diferentes, a aplicação deve impedir a conexão ou apresentar uma forma segura de adaptação, conforme a arquitetura do simulador.

* Toda a lógica de simulação deve utilizar a largura configurada do componente, sem depender de versões específicas como "Entrada 4 bits" ou "Saída 8 bits".

* A implementação deve eliminar completamente a necessidade de componentes separados como:

  * Entrada 2 bits
  * Entrada 4 bits
  * Entrada 8 bits
  * Entrada 16 bits
  * Saída 2 bits
  * Saída 4 bits
  * Saída 8 bits
  * Saída 16 bits

* Todo o sistema deve utilizar apenas os componentes genéricos "Entrada" e "Saída", configuráveis por propriedade.

Implemente essa funcionalidade seguindo a arquitetura existente do projeto, reutilizando o máximo possível do código atual, mantendo compatibilidade com o sistema de simulação e preparando a base para futuras funcionalidades envolvendo barramentos.
