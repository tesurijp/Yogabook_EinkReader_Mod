/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EVSTATICTEXTIMP_H_
#define _EVSTATICTEXTIMP_H_



// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvStaticText将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。
DECLARE_BUILTIN_NAME(StaticText)
class CEvStaticText :
	public CXuiElement<CEvStaticText ,GET_BUILTIN_NAME(StaticText)>
{
friend CXuiElement<CEvStaticText ,GET_BUILTIN_NAME(StaticText)>;
public:
	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

protected:
	CEvStaticText();
	virtual ~CEvStaticText();

	//装载配置资源
	virtual ERESULT LoadResource();
	//绘制
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);

private:
	wchar_t* mpswText;		 //显示文字
	//XuiPosition mdTextPosition;		 //文字在背景图中的位置
	//XuiPosition mdExtendPosition;	 //延展线坐标
	DWORD mdwColor;			//文字颜色
	DWORD mdwFontSize;		//字体大小
	wchar_t* mpswFontName;	//字体名称
	STETXT_BMP_INIT::eSIZELIMIT mLimit;
	STETXT_BMP_INIT::eTEXTALIGN mTalign;
	STETXT_BMP_INIT::ePARAALIGN mPalign;
	STETXT_BMP_INIT::eFONTWEIGHT mFontWidget;
	D2D1_RECT_F mdDrawRect;	//绘制区域大小，目标和源一样

	//更换显示文字
	bool SetText(wchar_t* npswText = NULL);
	//重新生成图片
	bool ReCreateBmp();

};


#define TF_ID_ST_TEXT L"Text"				//显示文字
#define TF_ID_ST_COLOR L"Color"				//文字颜色
#define TF_ID_ST_FONT L"FontName"			//字体
#define TF_ID_ST_FONT_SIZE L"FontSize"		//字号
#define TF_ID_ST_SIZE_LIMIT L"SizeLimit"	//
#define TF_ID_ST_TALIGN L"Talign"			//
#define TF_ID_ST_PALIGN L"Palign"			//
#define TF_ID_ST_FONTWIDGET L"FontWidget"	//


#endif//_EVSTATICTEXTIMP_H_
