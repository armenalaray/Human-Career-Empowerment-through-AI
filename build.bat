@echo off
cd c:\AIEduscore\ai\code\

set CommonCompilerFlags= -Od -MTd -nologo -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4100 -wd4189 -wd4838 -wd4065 -wd4505 -wd4201 -FC -Z7 
set CommonCompilerFlags= -DPLATFORM_MSVC=1 -DEAGLEFLY_INTERNAL=1 %CommonCompilerFlags% 
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build
del *.pdb > NUL 2> NUL

echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags%  ..\ai\code\ai_main.cpp -LD /link -incremental:no -opt:ref -PDB:ai_%random%.pdb -EXPORT:AIUpdateAndRender 
del lock.tmp

cl %CommonCompilerFlags% ..\ai\code\win32_ai.cpp -Fmwin32_eaglefly.map /link %CommonLinkerFlags%
popd 


