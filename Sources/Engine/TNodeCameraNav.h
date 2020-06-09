/*
TiX Engine v2.0 Copyright (C) 2018~2021
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TNodeCameraNav : public TNodeCamera, public TEventHandler
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(CameraNav);
	public:
		virtual ~TNodeCameraNav();

		virtual void Tick(float Dt);
		virtual bool OnEvent(const TEvent& e);
		
		void SetDollySpeed(float s)
		{
			DollySpeed	= s;
		}
		void SetModeSpeed(float s)
		{
			MoveSpeed	= s;
		}

	protected:
		void UpdateCameraAction();

	protected:
		enum E_CAM_ACTION
		{
			ECA_NONE,
			ECA_DOLLY,
			ECA_ROLL,
			ECA_MOVE,
		};

		E_CAM_ACTION Action;
		vector3df OldTarget;
		vector3df OldPosition;

		float RotateSpeed;

		float MoveSpeed;
		float DollySpeed;

		vector2di MouseStartPoint;
		vector2di MouseCurrentPoint;

		int32 HoldKey;
	};

} // end namespace ti

