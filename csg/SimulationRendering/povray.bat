set ldm="C:\Program Files\POV-Ray\v3.7\bin\pvengine64.exe"

copy pov\%1 tmp.pov /v /y

:delete vertex color
clearVertexColor.exe pov\%1 > tmp.pov

%ldm%  /EXIT +Itmp.pov +O%2 +W512 +H384
copy %2.png .\png /v /y
del %2.png
del tmp.pov

:%ldm%  /EXIT +Ipov\%1 +obmp\%1.bmp +W512 +H384
:%ldm%  +Ipov\%1 +obmp\%1.bmp 
