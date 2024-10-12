set ldm="..\Release\SolidBooleanSolver.exe"

:%ldm% -union_auto sweep_001.obj -np 40 -thread 1
%ldm% -sweep bunny.obj -start_p 0 0 70 1 1 1 -end_p 1 0 70 1 1 -0.5 -num 100 -np 20 -thread 1

pause
