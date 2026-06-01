@echo off
cd %~dp0
chcp 65001 1>nul
@REM chcp 936 1>nul
set "Package_Path=%cd%"

@REM 客户名称
set "Customer=JL"
@REM 项目名称
set "Project=YY-XX"
@REM 主控
set "IC=AC696X"
@REM SDK版本
set "SDK_VERSION=1.7.0"
@REM 发布文件目录
set "RELEASE_PATH=release"
@REM 打包源
set "PACKAGE_SOURCE=cpu\br25\tools"
@REM Lod bat
set "LOD_BAT=cpu\br25\tools\download.bat"
@REM FW文件
set "FW_FILE=cpu\br25\tools\soundbox\standard\jl_isd.fw"
@REM Log 文件
set "LOG_FILE=cpu\br25\tools\soundbox\standard\info.log"
@REM 是否开启git 远程仓库检测
set "GET_CHECK_EN=N"
@REM 不需要打包的文件类型 只能设置一个类型更多类型可以在C:\UserTool\7z\7z.bat中设置
@REM set "EXCLUDE_TYPE=*.key"

@REM 运行打包处理
call  C:\UserTool\run.bat


pause
exit
