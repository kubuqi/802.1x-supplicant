@ echo off
rem *****************************************************
rem ** SimuNAS启动示范脚本
rem *****************************************************

SimuNAS -i 0 -r 10.1.10.46 -s testing123 -t 3

rem        |	    |             |         |
rem        |	    |             |         +--- Log 级别
rem        |	    |             +------------- Radius服务器的share secret 
rem        |	    +--------------------------- Radius服务器地址
rem 	   +------------------------------------ 使用第0个网络接口
