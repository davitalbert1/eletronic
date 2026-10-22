# Logic Gate Simulator & Synthesizer

Sistema interativo de criação, simulação, visualização e síntese automática de circuitos lógicos e digitais em C++ com SFML e ImGui.

---

## 1. Requisitos do Sistema e Ferramentas

Para compilar e executar o projeto no Windows, você precisa das seguintes ferramentas instaladas:

- **Compilador C++17** (GCC / MinGW-w64 10+ ou MSYS2)
- **CMake 3.15+**
- **Ninja** ou **MinGW Makefiles**
- **SFML 3.0+** (Graphics, Window, System)
- OpenGL e bibliotecas de sistema Windows (`opengl32`, `gdi32`, `dwmapi`)

> **Dica (MSYS2 MinGW64):** Se você utiliza o MSYS2 instalado em `D:\msys64` (ou `C:\msys64`), certifique-se de que o diretório `mingw64\bin` esteja no seu PATH ou utilize o script de inicialização do ambiente.

---

## 2. Como Compilar o Projeto

Abra o terminal (PowerShell, Prompt de Comando ou terminal MSYS2) na pasta raiz do projeto (`F:\eletronic`):

### Passo 1: Configurar as variáveis de ambiente (se usando MSYS2)
No PowerShell:
```powershell
$env:PATH = "D:\msys64\mingw64\bin;D:\msys64\usr\bin;" + $env:PATH
```

### Passo 2: Gerar os arquivos de build com CMake
Utilizando o gerador **Ninja**:
```powershell
cmake -B build -G "Ninja"
```
*(Ou, se preferir o MinGW Makefiles: `cmake -B build -G "MinGW Makefiles"`)*

### Passo 3: Compilar o executável
```powershell
ninja -C build
```
*(Ou, se gerou com MinGW Makefiles: `cmake --build build`)*

O executável `simulator.exe` será gerado dentro da pasta `build/`.

---

## 3. Como Executar o `.exe`

Após a compilação, você pode iniciar o simulador diretamente pelo terminal ou clicando duas vezes no executável:

### Pelo PowerShell:
```powershell
.\build\simulator.exe
```

### Execução Direta:
Basta navegar até a pasta `build/` e abrir o arquivo `simulator.exe`. Caso alguma DLL do SFML ou do compilador seja solicitada, garanta que o diretório `bin` do MinGW esteja nas variáveis de ambiente do sistema ou copie as DLLs necessárias para a pasta `build/`.

---

## 4. Como Usar a Síntese Lógica Automática de Circuitos

A **Síntese Lógica Automática** permite que você forneça apenas os **nomes das entradas**, os **nomes das saídas** e o **processamento esperado** (por meio de expressões booleanas ou de tabela verdade). O algoritmo gera o circuito completo (portas padrão ou chips criados por você), posiciona os componentes na tela, faz a fiação automática e aplica o resultado diretamente à bancada de trabalho.

### Acessando a Ferramenta na Interface
1. Abra o `simulator.exe`.
2. No menu superior ou no painel lateral de Ferramentas Textuais, certifique-se de que a opção **"Janela Universal de Circuitos"** está aberta.
3. Na janela do editor universal, clique na aba **"Síntese Lógica Automática (AI/Heurística)"**.

---

### Opção A: Utilizando Presets Prontos
No menu suspenso **"Carregar Exemplo / Preset"**, escolha um dos circuitos pré-configurados:
- **Meio Somador (Half Adder)**: Gera a soma de 2 bits (`S = A ^ B`, `Cout = A & B`).
- **Somador Completo (Full Adder)**: Gera soma de 3 bits com Carry In (`A, B, Cin`).
- **Multiplexador 2:1 (MUX)**: Seleciona entre duas entradas conforme a linha de seleção.
- **Demultiplexador 1:2 (DEMUX)**: Direciona uma entrada para uma das duas saídas.
- **Comparador de 1 bit**: Gera saídas para igualdade (`Eq`), maior (`Gt`) e menor (`Lt`).
- **Porta XOR usando apenas NAND**: Demonstra a sintetização com conjunto reduzido de primitivas.

Ao selecionar um preset, todos os campos são preenchidos automaticamente. Basta clicar em **"Sintetizar Circuito Automaticamente"**.

---

### Opção B: Criando Circuitos Personalizados

#### 1. Entradas (Inputs)
Informe os nomes das variáveis separados por vírgula.
* Exemplo: `A, B` ou `A, B, Cin` ou `Sel, D0, D1`

#### 2. Saídas (Outputs)
Informe os nomes das saídas esperadas separados por vírgula.
* Exemplo: `S, Cout` ou `Y`

#### 3. Processamento Esperado
Você pode definir o comportamento de duas maneiras:

* **Modo Expressões Booleanas:**
  Escreva cada equação de saída (uma por linha) usando operadores convencionais:
  * `&` ou `&&` : Operador **AND**
  * `|` ou `||` : Operador **OR**
  * `^` : Operador **XOR**
  * `!` ou `~` : Operador **NOT**
  * `(` e `)` : Parênteses para definir precedência

  *Exemplo:*
  ```text
  S = A ^ B
  Cout = A & B
  ```

* **Modo Tabela Verdade (Truth Table):**
  Defina a sequência de bits de saída correspondente a todas as combinações binárias das entradas em ordem crescente (`00`, `01`, `10`, `11`...).
  * `0` : Nível lógico baixo
  * `1` : Nível lógico alto
  * `X` ou `?` : Don't-care (o sintetizador otimiza para o menor circuito possível)

  *Exemplo para porta XOR de 2 entradas:*
  ```text
  TABLE_Y = 0110
  ```

#### 4. Portas e Chips Permitidos
* **Portas Primitivas Permitidas:** Liste as portas padrão que o algoritmo pode usar: `AND, OR, NOT, XOR, NAND, NOR, XNOR`.
* **Permitir Chips Criados por Usuários:** Marque esta opção para que o sintetizador pesquise os chips salvos na sua biblioteca (`ChipLibrary`). Se uma subfunção do processamento puder ser resolvida por um chip já criado (ex: um Meio Somador reutilizado dentro de um Somador Completo), o algoritmo o incluirá no circuito.

#### 5. Execução e Resultados
* Clique no botão **"Sintetizar Circuito Automaticamente"**.
* O algoritmo avaliará os candidatos, minimizando erro de tabela verdade e número de componentes (inspirado no algoritmo de busca de `dice.cpp` e método Quine-McCluskey).
* O circuito gerado é exibido instantaneamente na bancada: os pinos de entrada, componentes intermediários e pinos de saída já estarão conectados por fios e funcionando de forma interativa.
* Um sumário estatístico informará a **precisão alcançada (100% ou aproximação)**, a quantidade de componentes e conexões criadas, além de disponibilizar o código em **Circuit Description Language (CDL)** correspondente.