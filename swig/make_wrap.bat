@echo off
call set-java-path.bat
set JAVA_INCLUDE=%JAVA_DIR%\include
set SWIG=..\..\swig\swigwin-3.0.2\swig.exe

set MODULE_NAME=test
set JAVA_NAME=Test
set IF_NAME=%MODULE_NAME%
set PACKAGE_NAME=mcl.%MODULE_NAME%
set PACKAGE_DIR=%PACKAGE_NAME:.=\%

echo [[run swig]]
mkdir %PACKAGE_DIR%
echo %SWIG% -java -package %PACKAGE_NAME% -outdir %PACKAGE_DIR% -c++ -Wall %IF_NAME%.i
%SWIG% -java -package %PACKAGE_NAME% -outdir %PACKAGE_DIR% -c++ -Wall %IF_NAME%.i
echo [[make dll]]
mkdir ..\bin
cl /MD /DNOMINMAX /DNDEBUG /LD /Ox /EHsc %IF_NAME%_wrap.cxx -I%JAVA_INCLUDE% -I%JAVA_INCLUDE%\win32 /link /LIBPATH:../../cybozulib_ext/mpir/lib /OUT:./%IF_NAME%_wrap.dll

call run-%MODULE_NAME%.bat

echo [[make jar]]
%JAVA_DIR%\bin\jar cvf %MODULE_NAME%.jar mcl
