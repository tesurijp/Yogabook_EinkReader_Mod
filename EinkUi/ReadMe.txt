Aug.01,2012 Ax
	目前设计的Domodal和Callbackbywinthread机制，会很容易导致死锁，
	特别是callback时，需要等待，Opthread将会接收到全部的输入消息，这些消息很可能会导致新的callback发生。
	如果此时，有定时器导致新的modal窗口打开，就不能关闭各种输入消息，而modal窗口又将导致新一层调用嵌入。
	所以，目前上没有妥善解决这个问题，仅仅是对CALLBACK调用处，人为地加以注意，将可能导致死锁的调用
	改为不等待的方式。

Nov.27,2017 Ax
	修改出现两个分支，
		此分支提供给Eink Panel使用，通过联想许灵均提供的访问ITE8951的接口写在Eink Panel上，那个版本去除了绘制在B面普通屏幕的功能。
			名字改为 EinkUi.dll Ectl.dll，注册表的默认存放位置是HKEY_LOCAL_MACHINE\SOFTWARE\Lenovo\EinkXctl
		另一个分支，精简了部分程序，只提供写到Layered Window的显示方式
			名字仍然使用 Xui.dl Xctl.dll，注册表的默认存放位置是HKEY_LOCAL_MACHINE\SOFTWARE\Lenovo\EsiXui
	

