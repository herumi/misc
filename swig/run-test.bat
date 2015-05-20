@echo off
echo [[compile %JAVA_NAME%Test.java]]
%JAVA_DIR%\bin\javac %JAVA_NAME%Test.java

echo [[run %JAVA_NAME%Test]]
%JAVA_DIR%\bin\java -classpath .\ %JAVA_NAME%Test %1 %2 %3 %4 %5 %6
popd
