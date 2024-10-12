obj2pov.exe %1 tmp
type mesh_model__begin.txt > %1_main.pov
type tmp >> %1_main.pov
type mesh_model__end.txt >> %1_main.pov
