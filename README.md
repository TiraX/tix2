# tix2
TiX V2.0


1, 解决方案文件在 Projects/Windows 和 Project/iOS 下

2, 编译 02.ResConverter release 版本。需要先把VS升级到16.9.2的版本

3, Cook 资源，在各个Samples/xxxxx/Content，运行 CookResources.bat 文件;该步骤会调用1步骤中生成的ResConverter进行资源cook; cook结束后会在Content下生成一个Cooked文件夹;

4, 工作目录在各个Samples/xxxxx/Content目录下，设置 项目属性-->调试-->工作目录设置为:$(ProjectDir)/Content

5,编译, 运行。如果发现有WinPixEventRuntime.dll的错误, 需要在系统里面copy 一份WinPixEventRuntime.dll到Content下