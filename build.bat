@echo off
echo Compilando Logic Gate Simulator...
g++ src/main.cpp -o simulator.exe -Iinclude -lsfml-graphics -lsfml-window -lsfml-system -mwindows
if %errorlevel% neq 0 (
    echo Erro na compilacao! Verifique se a SFML esta instalada no MinGW64.
    pause
    exit /b %errorlevel%
)
echo Compilacao concluida com sucesso!
echo Execute 'simulator.exe' para iniciar.
pause
