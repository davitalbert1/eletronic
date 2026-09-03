# Features Implementadas

## 1. Importação de Projetos do "Digital Logic Sim" (DigitalLogicSimImporter)

**Arquivos:**
- `include/DigitalLogicSimImporter.h`
- `src/DigitalLogicSimImporter.cpp`

**Funcionalidades:**
- Importação de projetos no formato JSON do Digital Logic Sim
- Suporte às versões 1 e 2 do formato
- Mapeamento de componentes DLS para tipos de gates internos
- Criação automática de gates e conexões (wires)
- Relatório de importação com componentes não convertidos
- Diálogo de resultado da importação

**Uso:**
```cpp
// Importar projeto DLS
workbench.importDigitalLogicSimProject("caminho/para/projeto.json");
```

## 2. Linguagem Textual para Descrever Circuitos (CircuitLanguageParser)

**Arquivos:**
- `include/CircuitLanguageParser.h`
- `src/CircuitLanguageParser.cpp`

**Sintaxe da Linguagem (CDL - Circuit Description Language):**
```
circuit NomeDoCircuito ["descrição"]
{
    gate TIPO id at (x, y)
    input id at (x, y)
    output id at (x, y)
    chip chip_id instance_id at (x, y)
    wire from_gate.from_pin -> to_gate.to_pin
}
```

**Tipos de Gates Suportados:**
- AND, OR, NOT, XOR, NAND, NOR, XNOR
- CLOCK, POWER, SWITCH, BUTTON, LED
- ROM, 7SEG, LCD, COUNTER, REGISTER

**Exemplo:**
```
circuit MeuCircuito "Um circuito simples"
{
    gate AND and1 at (100, 100)
    gate OR or1 at (300, 150)
    input in1 at (50, 80)
    input in2 at (50, 120)
    output out1 at (400, 150)

    wire and1.0 -> or1.0
    wire in1.0 -> and1.0
    wire in2.0 -> and1.1
    wire or1.0 -> out1.0
}
```

**Uso:**
```cpp
// Importar de arquivo CDL
workbench.importFromCircuitLanguage("circuito.cdl");

// Exportar para arquivo CDL
workbench.exportToCircuitLanguage("circuito.cdl");
```

## 3. Exportação para Imagem (PNG, SVG) (ImageExporter)

**Arquivos:**
- `include/ImageExporter.h`
- `src/ImageExporter.cpp`

**Funcionalidades:**
- Exportação para PNG ( rasterizada via SFML RenderTexture)
- Exportação para SVG (formato vetorial)
- Suporte a grid opcional
- Cálculo automático de bounding box
- Cores temáticas do simulador

**Uso:**
```cpp
// Exportar para PNG
workbench.exportToImage("circuito.png", "png");

// Exportar para SVG
workbench.exportToImage("circuito.svg", "svg");
```

## 4. Exportação para PDF com Documentação (PdfExporter)

**Arquivos:**
- `include/PdfExporter.h`
- `src/PdfExporter.cpp`

**Funcionalidades:**
- Geração de PDF com múltiplas páginas
- Página de título com metadados
- Página de estatísticas do circuito
- Lista de componentes
- Lista de conexões
- Formato PDF 1.4 válido

**Uso:**
```cpp
// Exportar para PDF
workbench.exportToPDF("documentacao.pdf");
```

## 5. Suporte a Formatos Abertos (JSON, XML) - Aprimorado

**Arquivos:**
- `src/Workbench.cpp` (métodos exportJSONEnhanced, exportXMLEnhanced)

**Funcionalidades:**
- JSON aprimorado com:
  - Metadados de exportação (versão, timestamp)
  - Estatísticas do circuito
  - Contagem de tipos de gates
  - Formatação legível (pretty print)

- XML aprimorado com:
  - Metadados estruturados
  - Estatísticas
  - Lista de componentes com posições
  - Lista de conexões

**Uso:**
```cpp
// Exportar JSON aprimorado
workbench.exportJSONEnhanced("nome", "categoria");

// Exportar XML aprimorado
workbench.exportXMLEnhanced("nome", "categoria");
```

## Integração com a Interface

Os novos recursos estão disponíveis no menu do simulador:
- Botões de exportação: PNG, SVG, PDF
- Botões de importação/exportação CDL
- Diálogo de resultado de importação

## Compilação

Para compilar o projeto com as novas funcionalidades, adicione os novos arquivos ao seu sistema de build:

```cmake
# CMakeLists.txt exemplo
set(SOURCES
    src/main.cpp
    src/Workbench.cpp
    src/Gate.cpp
    src/Wire.cpp
    src/ConfirmationDialog.cpp
    src/DigitalLogicSimImporter.cpp
    src/UndoRedoManager.cpp
    src/WorkbenchAccessibility.cpp
    src/WorkbenchInput.cpp
    src/WorkbenchPerformance.cpp
    src/WorkbenchProject.cpp
    src/WorkbenchUI.cpp
    src/CircuitLanguageParser.cpp    # NOVO
    src/ImageExporter.cpp            # NOVO
    src/PdfExporter.cpp              # NOVO
    src/chip/ChipDefinition.cpp
    src/chip/ChipFactory.cpp
    src/chip/ChipLibrary.cpp
    src/chip/Circuit.cpp
    src/chip/Json.cpp
    src/chip/CustomChip.cpp
)
```

## Dependências

- SFML 3.0+ (Graphics, Window, System)
- ImGui (com backends OpenGL3 + Win32/Sdl2)
- nlohmann/json (já incluso em vendor/)
- GLM (já incluso em vendor/)
- C++17 ou superior

## Notas

- O exportador PDF gera arquivos PDF 1.4 válidos mas simplificados
- O exportador SVG gera arquivos compatíveis com navegadores modernos
- O parser CDL suporta comentários (// e /* */)
- Todos os exportadores salvam na pasta "projetos/"