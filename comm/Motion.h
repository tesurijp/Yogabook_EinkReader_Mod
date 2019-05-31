/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


/*
	FileName: Motion.h
*/
#pragma once

#include <math.h>
#include "cmmstruct.h"

#define MOTION_PI       3.14159265358979323846

/******************************************************************
*                                                                 *
* Base Motion Class                                            *
*                                                                 *
******************************************************************/

template <class T>
class Motion
{
public:
    Motion(T start, T end, T duration) :
        m_Start(start),
        m_End(end),
        m_Duration(duration)
    {
    }

    void SetStart(T start)
    {
        m_Start = start;
    }

    T GetStart()
    {
        return m_Start;
    }

    void SetEnd(T end)
    {
        m_End = end;
    }

    T GetEnd()
    {
        return m_End;
    }

    void SetDuration(T duration)
    {
        m_Duration = max(0, duration);
    }

    T GetDuration()
    {
        return m_Duration;
    }

    T GetValue(T time)
    {
		time = min(max(time, 0), m_Duration);
		return ComputeValue(time);
    }

protected:
    virtual T ComputeValue(T time) = 0;

    T m_Duration;
    T m_Start;
    T m_End;
};

/******************************************************************
*                                                                 *
* Linearly Interpolate Between Start and End                      *
*                                                                 *
******************************************************************/

template <class T>
class LinearMotion : public Motion<T>
{
public:
    LinearMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    virtual T ComputeValue(T time)
    {
        return m_Start + ((m_End - m_Start) * (time / m_Duration));
    }
};

/******************************************************************
*                                                                 *
* Exponential Interpolate						                  *
*                                                                 *
******************************************************************/
template <class T>
class EaseInExponentialMotion : public Motion<T>
{
public:
    EaseInExponentialMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
        return m_Start + (m_End - m_Start) * pow(2, 10 * (time/m_Duration - 1));
    }
};

template <class T>
class EaseOutExponentialMotion : public Motion<T>
{
public:
    EaseOutExponentialMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
        return m_Start + (m_End - m_Start) * (-pow(2, -10 * time/m_Duration) + 1);
    }
};

template <class T>
class EaseInOutExponentialMotion : public Motion<T>
{
public:
    EaseInOutExponentialMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
        //compute the current time relative to the midpoint
        time = time / (m_Duration / 2);
        //if we haven't reached the midpoint, we want to do the ease-in portion
        if (time < 1)
        {
            return m_Start + (m_End - m_Start)/2 * pow(2, 10 * (time - 1));
        }
        //otherwise, do the ease-out portion
        return m_Start + (m_End - m_Start)/2 * (-pow(2, -10 * --time) + 2);
    }
};

/******************************************************************
*                                                                 *
* Sinusoidal Interpolate						                  *
*                                                                 *
******************************************************************/
template <class T>
class EaseInSinusoidalMotion : public Motion<T>
{
public:
    EaseInSinusoidalMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
		// -c * Math.cos(t/d * (Math.PI/2)) + c + b;
        return (m_Start - m_End)*cos(time/m_Duration * (T)(MOTION_PI/2)) + m_End;
    }
};

template <class T>
class EaseOutSinusoidalMotion : public Motion<T>
{
public:
    EaseOutSinusoidalMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
		// c * Math.sin(t/d * (Math.PI/2)) + b
        return (m_End - m_Start)*sin(time/m_Duration * (T)(MOTION_PI/2)) + m_Start;
    }
};

template <class T>
class EaseInOutSinusoidalMotion : public Motion<T>
{
public:
    EaseInOutSinusoidalMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
		// -c/2 * (Math.cos(Math.PI*t/d) - 1) + b;
		return (m_Start - m_End)/2 * (cos(time*(T)MOTION_PI/m_Duration)-1) + m_Start;
    }
};

/******************************************************************
*                                                                 *
* Quadratic  Interpolate						                  *
*                                                                 *
******************************************************************/
template <class T>
class EaseInQuadraticMotion : public Motion<T>
{
public:
    EaseInQuadraticMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
		// t /= d; c*t*t + b;
		time /= m_Duration;
        return (m_End - m_Start) * time * time + m_Start;
    }
};

template <class T>
class EaseOutQuadraticMotion : public Motion<T>
{
public:
    EaseOutQuadraticMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
		// t /= d;  -c * t*(t-2) + b;
		time /= m_Duration;
        return (m_Start - m_End) * time *(time - 2) + m_Start;
    }
};

template <class T>
class EaseInOutQuadraticMotion : public Motion<T>
{
public:
    EaseInOutQuadraticMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
		// t /= d/2;
		// if (t < 1) return c/2*t*t + b;
		// t--;
		// -c/2 * (t*(t-2) - 1) + b;
		time /= (m_Duration/2);
		if(time < 1)
			return (m_End - m_Start)/2 * time * time + m_Start;

		time -= (T)1.0;
		return (m_Start - m_End)/2 * (time * (time-2)-1) + m_Start;
	}
};

/******************************************************************
*                                                                 *
* Circular Interpolate						                  *
*                                                                 *
******************************************************************/
template <class T>
class EaseInCircularMotion : public Motion<T>
{
public:
    EaseInCircularMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
		// t /= d; -c * (Math.sqrt(1 - t*t) - 1) + b;
		time /= m_Duration;
        return (m_Start - m_End) * (sqrt(1- time * time) - 1) + m_Start;
    }
};

template <class T>
class EaseOutCircularMotion : public Motion<T>
{
public:
    EaseOutCircularMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
		//t /= d;
		//t--;
		//return c * Math.sqrt(1 - t*t) + b;
		time /= m_Duration;
		time -= (T)1.0;
        return (m_End - m_Start) * sqrt(1 - time * time) + m_Start;
	}
};

template <class T>
class EaseInOutCircularMotion : public Motion<T>
{
public:
    EaseInOutCircularMotion(T start=0, T end=0, T duration=0) :
        Motion(start, end, duration)
    {
    }
protected:
    T ComputeValue(T time)
    {
		//t /= d/2;
		//if (t < 1) return -c/2 * (Math.sqrt(1 - t*t) - 1) + b;
		//t -= 2;
		//return c/2 * (Math.sqrt(1 - t*t) + 1) + b;
		time /= (m_Duration/2);
		if(time < (T)1.0)
			return (m_Start - m_End)/2 * (sqrt(1 - time * time) - 1) + m_Start;
		time -= (T)2.0;
		return (m_End - m_Start)/2 * (sqrt(1 - time * time) + 1) + m_Start;
    }
};

// This class is applicable in multiscene condition. It has mutlti scene to play.It will play the next scene automatically when the previous is finished.
class CMultiSceneMotion
{
public:
	CMultiSceneMotion(){
		Begin();
	}

	// add an new scene to the end. 
	// return the number of the Scene added
	int AddScene(Motion<FLOAT>* npScene){
		return moScenes.Insert(-1,npScene);
	}

	// get the scene by it's number
	Motion<FLOAT>* GetScene(int niNumber){
		if(niNumber >= 0 && niNumber < moScenes.Size())
			return moScenes[niNumber];
		return NULL;
	}

	// clear all scene
	void ClearAllScene(){
		Begin();
		moScenes.Clear();
	}

	// get the scene count
	int GetSceneCount(){
		return moScenes.Size();
	}

	// return if the all scenes have finished
	bool IsEnd(){
		return mbEnd;
	}

	// reset current state to beginnig and keep scenes exist
	void Begin(){
		muBaseTick = 0;
		muSceneBase = 0;
		miCurrentScene = 0;
		muFrameCount = 0;
		mbEnd = false;
	}

	// get frame count played after start or reset, the count will not increase if it reached MAXULONG32
	ULONG GetFrameCount(){
		return muFrameCount;
	}

	// play the scenes and returns the number of current scene,failed return -1; 0 is the first scene, it returns as the count of all scenes if that all scenes have finished.
	int Play(
		ULONG nuClockTick,	// in,count in millisecond. 
		FLOAT* npValue,	// out,optional,value return by GetValue() of current scene. it'll keep the end value of the last scene 
		FLOAT* npAllElapsed,		// the tick count as the elapsed millisecond from the first call of 'Play' to this time. it will not increase if all scenes have finished.
		FLOAT* npSceneElapsed		// the tick count as the elapsed millisecond in current scene
		){
		FLOAT lfTick;

		if(moScenes.Size()<=0 || muBaseTick > nuClockTick)
		{
			return -1;
		}

		if(muBaseTick == 0)
			muBaseTick = nuClockTick;
		if(muSceneBase == 0)
			muSceneBase = nuClockTick;

		if(npAllElapsed != NULL) *npAllElapsed = (FLOAT)(nuClockTick - muBaseTick);

		lfTick = (FLOAT)(nuClockTick - muSceneBase);

		if(npSceneElapsed != NULL) *npSceneElapsed = lfTick;

		if(lfTick >= moScenes[miCurrentScene]->GetDuration())
		{
			lfTick = moScenes[miCurrentScene]->GetDuration();
		}

		//if(miCurrentScene == 1)
		//	_asm int 3;
		if(npValue != NULL) *npValue = moScenes[miCurrentScene]->GetValue(lfTick);

		muFrameCount++;
		if(lfTick < moScenes[miCurrentScene]->GetDuration())
			return miCurrentScene;

		if(miCurrentScene+1 < moScenes.Size())
		{			
			muSceneBase = nuClockTick;
			return miCurrentScene++;	// return the value before increment
		}
		else
		{
			mbEnd = true;
			return miCurrentScene+1;	// return the count of all scenes and don't increase the current scene indicator
		}
	}

private:
	ULONG muBaseTick;
	ULONG muSceneBase;
	ULONG muFrameCount;
	bool mbEnd;
	int miCurrentScene;
	cmmVector<Motion<FLOAT>*> moScenes;

};