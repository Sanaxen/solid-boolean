mkdir png

if not exist obj2pov_genbat.exe (
	copy ..\obj2pov\x64\Release\*.exe . /v /y
)
obj2pov_genbat.exe "..\..\NCsi\ncsimproto64\testdata\sim\samplekata_part\obj\solid" solid_stock 5 2 > Pretreatment.bat

:pause
call Pretreatment.bat
del Pretreatment.bat
