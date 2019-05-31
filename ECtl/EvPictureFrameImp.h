/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EVPICTUREFRAMEIMP_H_
#define _EVPICTUREFRAMEIMP_H_



// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvPictureFrame将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。
DECLARE_BUILTIN_NAME(PictureFrame)
class CEvPictureFrame :
	public CXuiElement<CEvPictureFrame ,GET_BUILTIN_NAME(PictureFrame)>
{
friend CXuiElement<CEvPictureFrame ,GET_BUILTIN_NAME(PictureFrame)>;
public:

	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

protected:
	CEvPictureFrame();
	virtual ~CEvPictureFrame();

	//装载配置资源
	virtual ERESULT LoadResource();
	//绘制
	//virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	//切换显示帧,第一帧为1
	virtual ERESULT OnChangeIndex(LONG nlIndex = 0);
	//更换显示图片
	virtual ERESULT OnChangePic(wchar_t* npswPicPath = NULL,bool nbIsFullPath = false);
	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);
	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
	);
	//通知元素【显示/隐藏】发生改变
	virtual ERESULT OnElementShow(bool nbIsShow);
private:
	LONG mlCurrentIndex;			 //当前显示第几帧
	LONG mlMaxFrame;				 //最大帧数
	D2D1_SIZE_F mdFrameSize;		 //每帧的真实尺寸
	ULONG mulMethod;				 //采用什么缩放方式
	FLOAT mfBeginPos;				 //绘制时的X坐标起始点
	bool mbIsAutoPlay;				//是否自动播放
	bool mbIsPlayLoop;				//是否循环播放
	LONG mlPlayTimerElapse;			//播放动画的定时器时间

	//重新计算帧大小,nbResize为真才重新设置大小
	bool Resize(bool nbResize = true);
};


#define TF_ID_PIC_FRAME_COUNT L"FrameCount"	//该图有几帧
#define TF_ID_PIC_AUTO_PLAY L"AutoPlay"	//1自动播放，0关闭
#define TF_ID_PIC_PLAY_LOOP L"PlayLoop"	//1循环播放，0关闭
#define TF_ID_PIC_PLAY_TIMER_ELAPSE L"PlayElapse"	//切换每帧的间隔，ms单位

#define TF_ID_PIC_TIMER_ID_PLAY 1 //定时器ID，播放动画

#endif//_EVPICTUREFRAMEIMP_H_
