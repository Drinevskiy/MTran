@echo off
setlocal enabledelayedexpansion
set "files="
for %%f in (D:\Study\6_semestr\MTran\Lab3\*.cpp) do (
    set "files=!files! %%f"
)
C:\msys64\mingw64\bin\g++.exe -I "D:\Study\6_semestr\MTran\Lab3\headers" -std=c++17 -g !files! -o "D:\Study\6_semestr\MTran\Lab3\main.exe"