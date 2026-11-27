@echo off
cd /d "%~dp0"
if not exist "build\bezier_editor.exe" (
    echo Build bezier_editor with CMake first. See README.md.
    pause
    exit /b 1
)
"build\bezier_editor.exe" --output "%~dp0profile.csv"
