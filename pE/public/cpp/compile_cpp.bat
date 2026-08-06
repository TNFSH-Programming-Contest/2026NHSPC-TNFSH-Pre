@echo off
setlocal

set GRADER=stub.cpp
set CODE1=sixseven.cpp
set CODE2=eightseven.cpp
set PROBLEM=Cake_4.exe

g++ -std=gnu++17 -O2 -Wall -Wextra %GRADER% %CODE1% %CODE2% -o %PROBLEM%

if errorlevel 1 (
    echo [ERROR] Compilation failed!
    pause
    exit /b %errorlevel%
)

echo [SUCCESS] Compiled successfully!
pause

endlocal
