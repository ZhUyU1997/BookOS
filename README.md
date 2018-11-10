# BookOS
BookOS Is a 32bit x86 operate system.用来开发一个可以在虚拟机和真机上运行得较小的系统。
支持分页内存管理，时间片多任务调度，图形界面，鼠标，键盘，控制台等驱动，以及一些应用程序。
主要是涉及的内容多，但深入不多。项目由boot（引导程序），kernel（内核核心），driver（驱动），
gui（图形界面），include（头文件），lib（库文件），command（命令），application（应用程序），
syscall（系统调用），print（打印函数），image（存放镜像文件），makefile（编译文件）等组成。
配置环境变量，把tools文件夹配置到环境变量里就行。
	编译方法，打开complect文件，键入make就可以开始编译，获得Mediocre.img文件，把它放到虚拟机运行就行。
	tools:gcc ld dd make nasm rm
	如有疑问请联系作者：2323168280@qq.com
