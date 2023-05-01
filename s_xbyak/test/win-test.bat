@echo off
set FILE=gen_ff_x64
call :test1
set FILE=gen_bint_x64
call :test1
set FILE=misc
call :test1

exit /b

:test1
echo test %FILE%
echo gas
python3 %FILE%.py -m gas > a.txt
diff %FILE%_gas.txt a.txt
echo nasm
python3 %FILE%.py -win > b.txt
diff %FILE%_win.txt b.txt
echo masm
python3 %FILE%.py -win -m masm > c.txt
diff %FILE%_masm.txt c.txt
