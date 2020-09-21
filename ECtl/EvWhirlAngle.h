#pragma once

DECLARE_BUILTIN_NAME(WhirlAngle)
class CEvWhirlAngleImp :
	public CXuiElement<CEvWhirlAngleImp, GET_BUILTIN_NAME(WhirlAngle)>
{
	friend CXuiElement<CEvWhirlAngleImp, GET_BUILTIN_NAME(WhirlAngle)>;
public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	);

protected:
	CEvWhirlAngleImp(void);
	~CEvWhirlAngleImp(void);

	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	ERESULT ParseMessage(IEinkuiMessage* npMsg);

	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);

	//virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);

	//元素拖拽
	virtual ERESULT OnDragging(const STMS_DRAGGING_ELE* npInfo);

	//拖拽开始
	virtual ERESULT OnDragBegin(const STMS_DRAGGING_ELE* npInfo);

	//拖拽结束
	virtual ERESULT OnDragEnd(const STMS_DRAGGING_ELE* npInfo);

	//鼠标进入或离开
	virtual void OnMouseFocus(PSTEMS_STATE_CHANGE npState);

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);

	// 描述：
	//		更改图片帧数
	void SetPicIndex(
		IN size_t niIndex			// 索引
	);

	// 描述：
	//		通过当前位置，计算旋转了的弧度值
	double GetRadian(
		IN D2D1_POINT_2F nfCrtPoint
	);

	// 描述：
	//		获取两点之间的距离
	double GetDistance(
		IN D2D1_POINT_2F ndfPointA,
		IN D2D1_POINT_2F ndfPointB
	);

	// 描述：
	//		更新旋转点位置
	void UpdateDotPosition(
		IN double ndbRadian
	);

	// 描述：
	//		弧度转换成角度
	double RadianToAngle(
		IN double ndbRandian
	);

	// 描述：
	//		角度转换成弧度
	double AngleToRadian(
		IN double ndbAngle
	);

private:
	void LoadResource();

	IEinkuiIterator* mpoWhirlBg;			// 旋转背景
	IEinkuiIterator* mpoWhirlDot;			// 旋转点

	D2D1_POINT_2F mdWhirlCenter;		// 旋转中心	
	D2D1_POINT_2F mdDotPoint;			// 旋转点坐标（默认为旋转点的中心坐标）

	double mdbRadian;						// 当前旋转的弧度
};

#define CONST_VALUE_PI					3.141592


#define ID_OF_SUB_WHIRL_BG				1
#define ID_OF_SUB_WHIRL_DOT				2

#define TF_ID_WHIRLANGLE_WHIRL_CENTER_X			L"WhirlCenterX"			// 旋转中心X
#define TF_ID_WHIRLANGLE_WHIRL_CENTER_Y			L"WhirlCenterY"			// 旋转中心Y
#define TF_ID_WHIRLANGLE_DOT_POINT_X			L"DotPointX"			// 旋转点X
#define TF_ID_WHIRLANGLE_DOT_POINT_Y			L"DotPointY"			// 旋转点Y




