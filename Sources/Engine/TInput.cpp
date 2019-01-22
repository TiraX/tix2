/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "TInput.h"

#define _EVENT_LOG(...)	//_LOG

namespace tix
{
	static const int s_txkey_map[EKET_COUNT][2] = 
	{
		{-1 , -1},					//EKET_UNKNOWN
		{KEY_UP, KEY_KEY_W},		//EKET_UP,
		{KEY_DOWN, KEY_KEY_S},		//EKET_DOWN,
		{KEY_LEFT, KEY_KEY_A},		//EKET_LEFT,
		{KEY_RIGHT, KEY_KEY_D},		//EKET_RIGHT,

		{KEY_KEY_Z, KEY_KEY_J},		//EKET_A,
		{KEY_KEY_X, KEY_KEY_K},		//EKET_B,
		{KEY_KEY_C, KEY_KEY_L},		//EKET_C,
		{KEY_KEY_V, KEY_SEMICOLON},	//KET_D,

		{KEY_KEY_1, -1},			//EKET_L1,
		{KEY_KEY_2, -1},			//EKET_L2,
		{KEY_KEY_9, -1},			//EKET_R1,
		{KEY_KEY_0, -1},			//EKET_R2,

		{KEY_ESCAPE, -1},			//EKET_ESCAPE,
		{KEY_SPACE, KEY_RETURN},	//EKET_OK,
		{KEY_F1, KEY_F2},			//EKET_SELECT,

		{KEY_LCONTROL, KEY_RCONTROL},//EKET_FUNC1,
		{KEY_LSHIFT, KEY_RSHIFT},	//EKET_FUNC2,

		{KEY_DELETE, KEY_BACK},		//EKET_DELETE,
		{KEY_KEY_F, -1},	//EKET_FOCUS,
	};

	inline int	GetEventDistance(const _DeviceEvent& e1, const _DeviceEvent& e2)
	{
		return (e1.pos - e2.pos).getLengthSQ();
	}

	E_KEYEVENT_TYPE GetKeycode(int vk_code)
	{
		for (int i = 0 ; i < EKET_COUNT ; ++ i)
		{
			if (s_txkey_map[i][0] == vk_code || s_txkey_map[i][1] == vk_code)
				return (E_KEYEVENT_TYPE)i;
		}
		return EKET_UNKNOWN;
	}

	TEventHandler::TEventHandler()
		: Input(NULL)
	{
		// register to Input
		TInput * input	= TEngine::Get()->GetDevice()->GetInput();
		input->RegisterHandler(this);
	}

	TEventHandler::~TEventHandler()
	{
		if (Input)
		{
			Input->UnRegister(this);
		}
	}

	void TEventHandler::OnRegisterToInput(TInput * input)
	{
		Input	= input;
	}

	//////////////////////////////////////////////////////////////////////////

    const float _InputCurve::heavy_force_value  = 2.2f;
	void _InputCurve::Reset()
	{
		Type			= EET_INVALID;
		Start.type		= EET_INVALID;
		End.type		= EET_INVALID;
		Current.type	= EET_INVALID;
        MaxForce        = 0.f;
	}
    
    void _InputCurve::SetMaxForce(float f)
    {
        if (f > MaxForce)
        {
            MaxForce    = f;
        }
    }

	E_EVENT_TYPE _InputCurve::Check()
	{
		if (Start.type != EET_INVALID && End.type != EET_INVALID)
		{
			int dis_sq		= (End.pos - Start.pos).getLengthSQ();
			int time_dis	= (int)(End.time_stamp - Start.time_stamp);
			if (time_dis < 500)
			{
				if (dis_sq < 24 * 24)
				{
                    if (MaxForce >= heavy_force_value)
                    {
                        _EVENT_LOG("  HEAVY CLICK.\n");
                        Type	= EET_HEAVY_CLICK;
                    }
                    else
                    {
                        _EVENT_LOG("  CLICK.\n");
                        Type	= EET_LEFT_CLICK;
                    }
					return Type;
				}
				else if (dis_sq > 100 * 100)
				{
					_EVENT_LOG("  SWIPE.\n");
					Type	= EET_LEFT_SWIPE;
					return Type;
				}
			}
		}
		else if (Start.type != EET_INVALID && Current.type != EET_INVALID)
		{
			long long curr_time;
			curr_time		= TTimer::GetCurrentTimeMillis();
			int time_dis	= (int)(curr_time - Start.time_stamp);
			if (time_dis > 500)
			{
				_EVENT_LOG("  HOLD AND DRAG. [%d, %d]\n", Current.pos.X, Current.pos.Y);
				Type		= EET_LEFT_HOLDDRAG;
				return Type;
			}
		}

		return EET_INVALID;
	}

	//////////////////////////////////////////////////////////////////////////
	TInput::TInput()
		: InputFlag(0)
        , InputCount(0)
		, PosRead(0)
		, CurvePos(0)
		, LastClickTime(0)
		, EnableImmediate(true)
	{
	}

	TInput::~TInput()
	{}

	void TInput::RegisterHandler(TEventHandler * handler)
	{
		TI_ASSERT(handler);

		if (handler)
		{
			EventHandlers.push_back(handler);
			handler->OnRegisterToInput(this);
		}
	}

	void TInput::UnRegister(TEventHandler * handler)
	{
		VecEventHandlers::iterator it	= std::find(EventHandlers.begin(), EventHandlers.end(), handler);
		if (it != EventHandlers.end())
		{
			EventHandlers.erase(it);
		}
	}
    
    void TInput::IncreaseInputCount(int c)
    {
        InputCount  += c;
    }
    
    void TInput::DecreaseInputCount(int c)
    {
		InputCount  -= c;
        TI_ASSERT(InputCount >= 0);
    }

	void TInput::EnableImmediateSend(bool enable)
	{
		EnableImmediate = enable;
	}

	void TInput::PutEvent(E_EVENT_TYPE type, int touch_id, long long time_stamp, float force, unsigned int param, int posX, int posY)
	{
		_DeviceEvent de;
		de.type			= type;
		de._id			= touch_id;
		de.time_stamp	= time_stamp;
		de.param		= param;
        de.force        = force;
		de.pos.X		= posX;
		de.pos.Y		= posY;
        
        if (InputCount >= 1)
            de.flag     |= DE_IS_MULTI_INPUT;

		AddEventToQuene(de);
	}

	void TInput::UpdateEvents(float dt)
	{
		SendImmediate();
		AnalysisCurve();
	}

	void TInput::SendEvent(TEvent& e)
	{
		//_LOG("send event %d - [%d, %d].\n", e.type, e.posX0, e.posY0);
		for (unsigned int i = 0 ; i < EventHandlers.size() ; ++ i)
		{
			TEventHandler* h	= EventHandlers[i];
			if ( !h->OnEvent(e) )
			{
				e.SetFlag(EVTF_HANDLED, true);
				// stop event pass to next
				break;
			}
		}
	}

	void TInput::AddEventToQuene(const _DeviceEvent& e)
	{
		SetFlag(IPTF_SEQUENCE_DIRTY, true);
		if (EnableImmediate)
		{
			ImmediateEvents[PosRead] = e;
			// need mutex lock
			PosRead = (PosRead + 1) % k_max_immediate_queue;
			Cursor = e.pos;
		}

		if (e.type == EET_LEFT_DOWN)
		{
            // if there is multi input, only receive the last one
            // clear the last input
            if (InputCount >= 1)
            {
                GetCurrentCurve().Reset();
            }
            
			_InputCurve& curve			= GetNextCurve();
			curve.Reset();
			curve.Start					= e;
			curve.Current				= e;
            curve.SetMaxForce(e.force);
		}
		else if (e.type == EET_LEFT_UP)
		{
            GetCurrentCurve().End		= e;
            GetCurrentCurve().SetMaxForce(e.force);
		}
		else if (e.type == EET_MOVE)
		{
            GetCurrentCurve().Current	= e;
            GetCurrentCurve().SetMaxForce(e.force);
		}
//#ifdef TI_PLATFORM_WIN32
		else if (e.type == EET_ZOOMIN ||
				 e.type == EET_ZOOMOUT)
		{
			_InputCurve& curve = GetNextCurve();
			curve.Reset();
			curve.Start = e;
			curve.Current = e;
			curve.End = e;
			curve.SetMaxForce(e.force);
		}
//#endif // TI_PLATFORM_WIN32

	}

	void TInput::SendImmediate()
	{
		if ((InputFlag & IPTF_SEQUENCE_DIRTY) == 0)
			return;

		if (EnableImmediate)
		{
			for (int i = 0; i < k_max_immediate_queue; ++i)
			{
				_DeviceEvent& de = ImmediateEvents[i];

				SendImmediate(de);
			}
		}
		else
		{
			for (int i = 0; i < k_max_curve_queue; ++i)
			{
				_InputCurve& curve = CurveQueue[i];
				_DeviceEvent& de0 = curve.Start;
				_DeviceEvent& de1 = curve.Current;
				_DeviceEvent& de2 = curve.End;

				// send original events
				if (de0.type == EET_ZOOMIN ||
					de0.type == EET_ZOOMOUT)
				{
					SendImmediate(de0);
				}
				else
				{
					SendImmediate(de0);
					SendImmediate(de1);
					SendImmediate(de2);
				}
			}
		}
		SetFlag(IPTF_SEQUENCE_DIRTY, false);
	}

	void TInput::SendImmediate(_DeviceEvent& de)
	{
		TEvent event_to_send;
		if ((de.flag & DE_ALREAD_SENT) == 0 && de.type != EET_INVALID)
		{
			event_to_send		= de;
			if ((de.flag & DE_IS_MULTI_INPUT) != 0)
				event_to_send.SetFlag(EVTF_SECOND, true);
			SendEvent(event_to_send);
			if (event_to_send.HasFlag(EVTF_HANDLED))
				de.flag			|= DE_HANDLED;
			de.flag				|= DE_ALREAD_SENT;
		}
	}

	void TInput::AnalysisCurve()
	{
		TEvent event_to_send;

		int read_pos		= CurvePos;
		for (int i = 0 ; i < k_max_curve_queue ; ++ i)
		{
			_InputCurve& curve		= CurveQueue[read_pos];
			read_pos				= (read_pos + 1) % k_max_curve_queue;

			if (curve.Type == EET_INVALID)
			{
				if (curve.Check() != EET_INVALID)
				{
					if ((curve.Start.flag & DE_HANDLED) == 0 &&
						(curve.End.flag & DE_HANDLED) == 0)
					{
						TEvent e;
						e.type				= curve.Type;
						e.posX0				= curve.End.pos.X;
						e.posY0				= curve.End.pos.Y;
						e.posX1				= curve.Start.pos.X;
						e.posY1				= curve.Start.pos.Y;
						if ((curve.Start.flag & DE_IS_MULTI_INPUT) != 0)
							e.SetFlag(EVTF_SECOND, true);
						SendEvent(e);

						// check for double click
						if (e.type == EET_LEFT_CLICK)
						{
							long long curr_time = TTimer::GetCurrentTimeMillis();
							long long dt = curr_time - LastClickTime;
							if (dt < 500)	// in half second
							{
								e.type	= EET_DOUBLE_CLICK;
								SendEvent(e);
							}

							LastClickTime = curr_time;
						}
					}
				}
			}
			else if (curve.Type == EET_LEFT_HOLDDRAG)
			{
				if (curve.Check() == EET_LEFT_HOLDDRAG)
				{
					TEvent e;
					e.type				= curve.Type;
					e.posX0				= curve.Current.pos.X;
					e.posY0				= curve.Current.pos.Y;
					e.posX1				= curve.Start.pos.X;
					e.posY1				= curve.Start.pos.Y;
                    if ((curve.Start.flag & DE_IS_MULTI_INPUT) != 0)
                        e.SetFlag(EVTF_SECOND, true);
					SendEvent(e);
				}
			}
		}
	}

	_InputCurve& TInput::GetNextCurve()
	{
		CurvePos		= (CurvePos + 1) % k_max_curve_queue;
		return			 CurveQueue[CurvePos];
	}

	_InputCurve& TInput::GetCurrentCurve()
	{
		return			 CurveQueue[CurvePos];
	}
}
