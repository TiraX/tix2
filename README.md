# tix2
TiX v2.0


1. 解决方案文件在 Projects/Windows 和 Project/iOS 下
2. 编译 02.ResConverter release 版本。需要先把VS升级到16.9.2的版本
3. Cook 资源，在各个Samples/xxxxx/Content，运行 CookResources.bat 文件;该步骤会调用1步骤中生成的ResConverter进行资源cook; cook结束后会在Content下生成一个Cooked文件夹
4. 工作目录在各个Samples/xxxxx/Content目录下，设置 项目属性-->调试-->工作目录设置为:$(ProjectDir)/Content
5. 编译, 运行。如果发现有WinPixEventRuntime.dll的错误, 需要在系统里面copy 一份WinPixEventRuntime.dll到Content下

---

tix2/Samples

1. Startup ：引擎的启动，基本用法示例
2. MeshConverter ：用于将虚幻导出的资产转换为自定义的二进制格式的转换工具
3. SSSSSample ：皮肤材质，PC + iOS 版本
4. VTTextureBaker & VirtualTexture ： VT系统的实现，PC + iOS 版本
5. GPUDriven ：参考GDC2016 《Optimizing the Graphics Pipeline With Compute》实现的GPU Driven管线
6. GPUDriven 0.2 ：进阶优化版
7. GTAO ： 学习虚幻的Ground Truth AO Feature
8. SkyAtmosphere ：虚幻的天空大气层 Feature
9. RTAO ： 基于 RTX 的 AO
10. OceanFFT ：基于 Compute Shader 的实时 FFT 海洋测试
11. FluidSimulation ： 2d fluid 解算；3d Flip 和 Pbf solver
12. SkinnedAnimation ： 新同学渲染课作业
