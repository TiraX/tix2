# tix2
TiX V2.0

## 10.OceanFFT启动步骤说明

1, 编译 02.ResConverter release 版本
需要先把VS升级到16.9.2的版本

2, 设置启动项 10.Ocean FFT as start project

3, Cook 资源
运行10.OceanFFT/Content下的CookResources.bat文件;该步骤会调用1步骤中生成的ResConverter进行资源cook; cook结束后会在Content下生成一个Cooked文件夹;
如果发生cook错误, 需要把Cooked文件夹清掉重来;

4, 设置10.OceanFFT项目的工作目录, 
OceanFFT的项目属性-->调试-->工作目录设置为:$(ProjectDir)/Content

5,编译, 运行10.OceanFFT工程
如果发现有WinPixEventRuntime.dll的错误, 需要在系统里面copy 一份WinPixEventRuntime.dll到OceanFFT/Content下