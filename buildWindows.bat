@echo off


setlocal

set platform=x64

if "%1"=="x86" set platform=x86

set warnFlags=/W3 /wd4201 /FC
set includeFlags=/I..\SDL-mirror\include /I..\glew\include /I..\src\dirent_win
set libsFlags=..\SDL-mirror\VisualC\x64\Release\SDL2.lib ..\glew\lib\Release\x64\glew32.lib opengl32.lib

if not exist build mkdir build
pushd build
rem All cl options https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category?view=vs-2019
rem /Zi option produces a separate PDB file that contains all the symbolic debugging information for use with the debugger.
rem /MTd whether a lib is static or dll and if production or debug.
if "%1"=="debug" (
  rem Fill me in.
) else (
  rem cl ..\src\main.cpp -nologo /MT /GR- /EHsc /EHa- /Oi /Z7 /Fmmfile.map %warnFlags% %includeFlags% /link %libsFlags%
  cl ..\src\SDL_layer.cpp ..\src\node_editor.cpp -nologo /MT /GR- /EHsc /EHa- /Oi /Z7 /Fmmfile.map /DNODE_EDITOR_CAREFUL=1 %warnFlags% %includeFlags% /link %libsFlags% /OUT:NodeEditor.exe
  copy /Y ..\SDL-mirror\VisualC\x64\Release\SDL2.dll .
  copy /Y ..\glew\bin\Release\x64\glew32.dll .
)
popd