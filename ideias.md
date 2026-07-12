crie forma de usuário transformar uma lógica em texto, com esse texto conseguindo descrever exatamente o que está acontecendo (como uma pseudolinguagem de código)
faça também que seja possível importar essa linguagem para o ambiente, garantindo que seja possível remontar exatamente o que existira antes.
crie também uma pequena documentação explicando os itens desta documentação.
essa linguagem deve comportar estruturas complexas e compeltas reais.

ao iniciar app, deve abrir página perguntando sobre qual projeto que quer abrir, antes de abrir algum projeto. garanta que o salvamento e o carregamento do projeto esteja funcionando corretamente.

Quero corrigir o processo de distribuição do aplicativo para que o executável seja **completamente autossuficiente**, sem exigir que o usuário instale bibliotecas, runtimes ou dependências manualmente.

Problema atual:

Ao executar o `simulator.exe` diretamente, o Windows exibe erros como:

* `libsfml-system-3.dll não foi encontrado`
* `libsfml-graphics-3.dll não foi encontrado`
* `libgcc_s_seh-1.dll não foi encontrado`

Isso indica que o executável depende de DLLs externas que não estão sendo distribuídas corretamente.

Objetivo:

O usuário deve conseguir baixar a pasta (ou o instalador), dar dois cliques em `simulator.exe` e o programa iniciar normalmente, sem qualquer instalação adicional.

Requisitos:

* Identificar todas as dependências necessárias em tempo de execução.
* Garantir que todas as DLLs obrigatórias sejam incluídas automaticamente no processo de build ou empacotamento.
* Sempre que possível, utilizar **linkagem estática** para reduzir dependências externas.
* Quando a linkagem estática não for viável, copiar automaticamente todas as DLLs necessárias para a mesma pasta do executável durante o processo de compilação.
* Configurar o sistema de build para que a pasta de distribuição seja totalmente funcional.
* Eliminar a necessidade de instalar SFML, MinGW, GCC Runtime, Visual C++ Redistributable ou qualquer outra dependência para executar o programa.
* Garantir que o aplicativo funcione em uma instalação limpa do Windows, onde nenhuma biblioteca de desenvolvimento esteja instalada.
* Revisar a configuração do CMake (ou do sistema de build utilizado) para produzir uma distribuição completa.
* Criar um processo de empacotamento automatizado para que futuras compilações já gerem uma versão pronta para distribuição.
* Verificar se existem outras dependências indiretas que também precisam ser incluídas.
* Explicar quais alterações foram realizadas e por que elas resolvem o problema.

Implemente essa correção seguindo as boas práticas de distribuição de aplicações desktop para Windows, tornando o executável completamente portátil e pronto para uso em qualquer computador compatível.


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
