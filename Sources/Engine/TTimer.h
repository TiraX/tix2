/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TTimer
	{
	public:
		TI_API static const long long	GetCurrentTimeMillis();
		TI_API static const int			GetCurrentTimeSeconds();
		TI_API static const int			GetCurrentTimeSeconds(int& h, int& m, int& s);
		TI_API static const int			GetCurrentDate();
		TI_API static const int			GetCurrentDate(int& y, int& m, int& d);
		TI_API static const void		GetYMDFromDate(int date, int& y, int& m, int& d);
		TI_API static void				GetCurrentDateAndSeconds(int& d, int& s);
		TI_API static void				GetCurrentWeekDay(int& d);

		TI_API static bool				IsLeapYear(int year);
		// m from 1 - 12; d from 1 - 31
		TI_API static int				DayInYear(int y, int m, int d);
		TI_API static int				DayBetweenDates(int y1, int m1, int d1, int y2, int m2, int d2);
	};

	//////////////////////////////////////////////////////////////////////////

	class TI_API TTimeRecorder
	{
	public:
		TTimeRecorder();
		TTimeRecorder(const TString& InName);
		~TTimeRecorder();

		void LogTimeUsed();
	private:
		void Start();
		void End();

	private:
		TString Name;
		uint64 StartTime;
		uint64 EndTime;
	};

#ifdef TIX_SHIPPING
#	define TIMER_RECORDER(name)
#else
#	define TIMER_RECORDER(name) TTimeRecorder Rec(name)
#endif // TIX_SHIPPING

}