#pragma once
#include "math.h"

#define D2DDRAWX_PI       3.14159265358979323846f

//////////////////////////////////////////////////////////////////////////
// 本类实现用D2d Layer结合Geometry对贴图实现蒙版控制
class CEdxGeometryLayer
{
public:
	CEdxGeometryLayer(){
		mpGeomety = NULL;
		mpLayer = NULL;
		mpPushToTarget = NULL;
		mbEnalbed = true;
	}
	~CEdxGeometryLayer(){
		CMM_SAFE_RELEASE(mpGeomety);
		CMM_SAFE_RELEASE(mpLayer);
	}

	// 使能或者禁用，默认都是Enable状态
	void Enable(bool nbEnble){
		mbEnalbed = nbEnble;
	}

	// 绘制前调用，必须和Leave调用匹配
	bool Enter(ID2D1RenderTarget* npD2dTarget){
		CMM_SAFE_RELEASE(mpLayer);

		//if(mbEnalbed == false)
		//	return true;

		if(mpGeomety == NULL)
			return false;

		HRESULT hr = npD2dTarget->CreateLayer(NULL, &mpLayer);
		if(FAILED(hr))
			return false;

		npD2dTarget->PushLayer(
			D2D1::LayerParameters(D2D1::InfiniteRect(), mpGeomety),
			mpLayer
			);
		mpPushToTarget = npD2dTarget;

		return true;
	}

	// 绘制后调用，必须和Enter调用匹配
	void Leave(ID2D1RenderTarget* npD2dTarget){
		if(npD2dTarget != mpPushToTarget || mpGeomety==NULL/* || mbEnalbed != false*/)
			return;
		mpPushToTarget = NULL;

		npD2dTarget->PopLayer();
		CMM_SAFE_RELEASE(mpLayer);
	}

protected:
	bool mbEnalbed;
	ID2D1Geometry* mpGeomety;
	ID2D1Layer* mpLayer;
	ID2D1RenderTarget* mpPushToTarget;

};


//////////////////////////////////////////////////////////////////////////
// 钟表盘状蒙版
class CEdxDialMask : public CEdxGeometryLayer
{
public:
	
	// 设置基础值，每次调用SetBase后，必须调用至少一次SetAngle才能用于绘制
	bool SetBase(
		bool nbClockwise,			// sweep direction on dial，true for clockwise,opposite for counter clockwise
		const D2D1_POINT_2F& ndCenter,
		float nfRadius,
		float nfStartAngle // 初始角度，正数表示顺时针测量，负数表示逆时针测量，这与扫描方向无关，只是设定初始角度的方法
		)					// 0代表12点方向，180表示6点方向
	{
		if(nfRadius < 1.0f )
			return false;

		mbClockwise = nbClockwise;
		mdCenter = ndCenter;
		mfRadius = nfRadius;
		mfStartAngle = nfStartAngle*D2DDRAWX_PI/180.0f;
		if(nfStartAngle < 0.0f)
			mfStartAngle += D2DDRAWX_PI*2.0f;
		mfSweeped = 0.0f;

		return true;
	}

	__inline bool SetAngle(float nfSweep);	// 设置扫描幅度，0.0 - 1.0范围，表示一个正圆，小于0.0没有意义，大于1.0仍然是一个圆

	// 获取指示线上的点的坐标，圆心，扫描方向，其实角度都按照SetBase的设定为准
	D2D1_POINT_2F GetIndictorPosition(
			FLOAT nfRadius,		// 该点对于圆心的距离，及该点随着指示线扫描而成的圆的半径
			FLOAT nfSweep=-1.0f		// 扫描幅度，0.0 - 1.0范围，表示一个正圆，小于0.0表示取当前的幅度设置，0.0表示起始位置，大于1.0相当于划过整圆后继续划
			){

		D2D1_POINT_2F ldEndPoint;

		if(nfSweep < 0.0f)
			nfSweep = mfSweeped;

		if(mbClockwise != false)
		{
			ldEndPoint.x = mdCenter.x + sin(mfStartAngle+nfSweep*D2DDRAWX_PI*2.0f)*nfRadius;
			ldEndPoint.y = mdCenter.y - cos(mfStartAngle+nfSweep*D2DDRAWX_PI*2.0f)*nfRadius;
		}
		else
		{
			ldEndPoint.x = mdCenter.x + sin(mfStartAngle-nfSweep*D2DDRAWX_PI*2.0f)*nfRadius;
			ldEndPoint.y = mdCenter.y - cos(mfStartAngle-nfSweep*D2DDRAWX_PI*2.0f)*nfRadius;
		}

		return ldEndPoint;
	}

protected:
	D2D1_POINT_2F mdCenter;
	bool mbClockwise;
	float mfRadius;
	float mfStartAngle;
	float mfSweeped;

	ID2D1GeometrySink* CreatePathGeomety() {
		HRESULT hr;
		ID2D1GeometrySink* lpSink = NULL;
		ID2D1PathGeometry* lpPathGeomety=NULL;

		CMM_SAFE_RELEASE(mpGeomety);
		do 
		{
			hr = EinkuiGetSystem()->GetD2dFactory()->CreatePathGeometry(&lpPathGeomety);
			if(FAILED(hr))
				break;

			hr = lpPathGeomety->Open(&lpSink);
			if(FAILED(hr))
				break;

			lpSink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);
			mpGeomety = lpPathGeomety;
			lpPathGeomety = NULL;
		} while (false);

		CMM_SAFE_RELEASE(lpPathGeomety);

		return lpSink;
	}

};

bool CEdxDialMask::SetAngle(float nfSweep)	// 设置下止角度
{

	if(nfSweep < 0.0f)
		return false;
	
	if(abs(nfSweep - mfSweeped) < 0.001f)
	{
		if(mfSweeped!=0.0f)	// 表示已经调用过一次SetAngle函数了，那么就不需要改变
			return true;
		mfSweeped = 0.001f;
	}
	else	
	if(mfSweeped > 0.999f)	// 先前已经是一个圆了，就不用在修改了
	{
		mfSweeped = nfSweep;
		return true;
	}
	else
		mfSweeped = nfSweep;


	if(mfSweeped > 0.999f)	// it's a circle
	{
		HRESULT hr;
		ID2D1EllipseGeometry* lpEllipseGeomety=NULL;

		CMM_SAFE_RELEASE(mpGeomety);

		hr = EinkuiGetSystem()->GetD2dFactory()->CreateEllipseGeometry(D2D1::Ellipse(mdCenter,mfRadius,mfRadius), &lpEllipseGeomety);

		if(FAILED(hr))
			return false;

		mpGeomety = lpEllipseGeomety;
	}
	else
	{
		ID2D1GeometrySink* lpSink;
		D2D1_POINT_2F ldEndPoint;


		lpSink = CreatePathGeomety();
		if(lpSink == NULL)
			return false;

		lpSink->BeginFigure(
			mdCenter,
			D2D1_FIGURE_BEGIN_FILLED
			);

		lpSink->AddLine(D2D1::Point2F(mdCenter.x + sin(mfStartAngle)*mfRadius, mdCenter.y - cos(-mfStartAngle)*mfRadius));

		if(mbClockwise != false)
		{
			ldEndPoint.x = mdCenter.x + sin(mfStartAngle+mfSweeped*D2DDRAWX_PI*2.0f)*mfRadius;
			ldEndPoint.y = mdCenter.y - cos(mfStartAngle+mfSweeped*D2DDRAWX_PI*2.0f)*mfRadius;
		}
		else
		{
			ldEndPoint.x = mdCenter.x + sin(mfStartAngle-mfSweeped*D2DDRAWX_PI*2.0f)*mfRadius;
			ldEndPoint.y = mdCenter.y - cos(mfStartAngle-mfSweeped*D2DDRAWX_PI*2.0f)*mfRadius;
		}

		lpSink->AddArc(
			D2D1::ArcSegment(
			ldEndPoint,
			D2D1::SizeF(mfRadius,mfRadius),
			0.0f,
			mbClockwise?D2D1_SWEEP_DIRECTION_CLOCKWISE:D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
			mfSweeped>0.5f?D2D1_ARC_SIZE_LARGE:D2D1_ARC_SIZE_SMALL
			) );

		lpSink->AddLine(mdCenter);

		lpSink->EndFigure(D2D1_FIGURE_END_CLOSED);

		lpSink->Close();

		CMM_SAFE_RELEASE(lpSink);
	}


	return true;
}





