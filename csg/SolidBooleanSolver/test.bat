set ldm="..\Release\SolidBooleanSolver.exe"

:%ldm% -union_auto sweep_001.obj -np 40 -thread 1
%ldm% -sweep test.obj -start_p 0 0 70 1 1 1 -end_p 200 0 70 1 1 1 -num 1000 -np 40 -thread 1

pause
