if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat" (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat"
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" (
	call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
)
:pause

rem MSBuildでビルドする
cd %~dp0

echo "build start" > log.txt

goto 1

:1
set build=.\QD\qd_2.3.4_windll\qd.sln
MSBuild %build% /t:clean;rebuild /p:Configuration=static_release;Platform="x64"
MSBuild %build% /t:build /p:Configuration=static_release;Platform="x64"
MSBuild %build% /t:build /p:Configuration=static_release;Platform="x64"
if %ERRORLEVEL% neq 0 (
    echo ErrorLevel:%ERRORLEVEL%
    echo ビルド失敗
    set errmsg="QD"
    goto err
)
echo "QD build success" >> log.txt


:2
set build=.\csg\kdtree\kdtree\kdtree.sln
MSBuild %build% /t:clean;rebuild /p:Configuration=release;Platform="x64"
MSBuild %build% /t:build /p:Configuration=release;Platform="x64"
if %ERRORLEVEL% neq 0 kdtree(
    echo ErrorLevel:%ERRORLEVEL%
    echo ビルド失敗
    set errmsg="kdtree"
)
echo "kdtree build success" >> log.txt

set build=.\csg\csg_solid_modeller.sln
MSBuild %build% /t:clean;rebuild /p:Configuration=release;Platform="x64"
MSBuild %build% /t:build /p:Configuration=release;Platform="x64"
MSBuild %build% /t:build /p:Configuration=release;Platform="x64"
MSBuild %build% /t:build /p:Configuration=release;Platform="x64"
MSBuild %build% /t:build /p:Configuration=release;Platform="x64"
if %ERRORLEVEL% neq 0 (
    echo ErrorLevel:%ERRORLEVEL%
    echo ビルド失敗
    set errmsg="voxcel"
)
echo "voxcel:csg_solid_modeller build success" >> log.txt

set build=.\csg\symplify\symplify.sln
MSBuild %build% /t:clean;rebuild /p:Configuration=release;Platform="x64"
MSBuild %build% /t:build /p:Configuration=release;Platform="x64"
MSBuild %build% /t:build /p:Configuration=release;Platform="x64"
if %ERRORLEVEL% neq 0 (
    echo ErrorLevel:%ERRORLEVEL%
    echo ビルド失敗
    set errmsg="symplify"
)
echo "symplify build success" >> log.txt

goto end
:err
echo %errmsg% >> log.txt
pause

:end

echo "build finish" >> log.txt
