@echo off
setlocal

g++ -std=gnu++17 -O2 -Wall -Wextra stub.cpp sixseven.cpp eightseven.cpp -o Cake_4.exe
if errorlevel 1 exit /b %errorlevel%

endlocal
