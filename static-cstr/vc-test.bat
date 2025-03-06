@echo off

set CFLAGS=/nologo /EHsc /Zi /c /O2

cl %CFLAGS% main.cpp
cl %CFLAGS% sub1.cpp
cl %CFLAGS% sub2.cpp

link /out:main1.exe main.obj sub1.obj sub2.obj
link /out:main2.exe main.obj sub2.obj sub1.obj
link /out:main3.exe sub1.obj sub2.obj main.obj
link /out:main4.exe sub2.obj sub1.obj main.obj

for %%i in (1 2 3 4) do (
  main%%i.exe
)


exit /b

:run
