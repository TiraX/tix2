/*
TiX Engine v2.0 Copyright (C) 2018
By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeCameraNav.h"

namespace tix
{
	TNodeCameraNav::TNodeCameraNav(TNode* Parent)
		: TNodeCamera(Parent)
		, Action(ECA_NONE)
		, RotateSpeed(1.0f)
		, MoveSpeed(0.1f)
		, DollySpeed(0.01f)
		, HoldKey(EET_INVALID)
	{
	}

	TNodeCameraNav::~TNodeCameraNav()
	{
	}

	void TNodeCameraNav::Tick(float Dt)
	{
		UpdateCameraAction();
		TNode::Tick(Dt);
	}

//#define USE_MIDDLE

#ifdef USE_MIDDLE
#define ROTATE_MOUSE_BTN_DOWN	EET_MIDDLE_DOWN
#define ROTATE_MOUSE_BTN_UP		EET_MIDDLE_UP
#else
#define ROTATE_MOUSE_BTN_DOWN	EET_LEFT_DOWN
#define ROTATE_MOUSE_BTN_UP		EET_LEFT_UP
#endif

	bool TNodeCameraNav::OnEvent(const TEvent& e)
	{
		if (TEngine::Get()->GetScene()->GetActiveCamera() != this)
		{
			return true;
		}
#ifdef TI_PLATFORM_WIN32
		bool altDown	= ((::GetKeyState(VK_MENU) & (1<<(sizeof(SHORT)*8-1))) != 0);
#else
		bool altDown	= true;
#endif
		if (!altDown)
		{
			return true;
		}
		if (e.type	== ROTATE_MOUSE_BTN_DOWN ||
			e.type	== EET_RIGHT_DOWN)
		{
			MouseStartPoint.X = e.posX0;
			MouseStartPoint.Y = e.posY0;
			OldTarget = Target;
			OldPosition = RelativePosition;

			HoldKey = e.type;
			return false;
		}
		else if (e.type	== ROTATE_MOUSE_BTN_UP ||
				 e.type == EET_RIGHT_UP)
		{
			if (Action == ECA_ROLL ||
				Action == ECA_MOVE)
			{
				OldTarget = Target;
				OldPosition = RelativePosition;
			}
			Action = ECA_NONE;
			HoldKey = EET_INVALID;
			return false;
		}
		else if (e.type == EET_MOVE)
		{
			if (HoldKey == ROTATE_MOUSE_BTN_DOWN)
			{
				Action = ECA_ROLL;
				MouseCurrentPoint.X = e.posX0;
				MouseCurrentPoint.Y = e.posY0;
				return false;
			}
			else if (HoldKey == EET_RIGHT_DOWN)
			{
				Action = ECA_MOVE;
				MouseCurrentPoint.X = e.posX0;
				MouseCurrentPoint.Y = e.posY0;
				return false;
			}
		}
		else if (e.type == EET_ZOOMIN)
		{
			Action = ECA_DOLLY;
			MouseCurrentPoint.X = e.posX0;
			return false;
		}
		else if (e.type == EET_ZOOMOUT)
		{
			Action = ECA_DOLLY;
			MouseCurrentPoint.X = -e.posX0;
			return false;
		}
		else if (e.type == EET_WHEEL)
		{
			Action = ECA_DOLLY;
			MouseCurrentPoint.X	= e.posX0;
			return false;
		}
        else if (e.type == EET_ZOOMIN)
        {
            Action = ECA_DOLLY;
            MouseCurrentPoint.X	= e.posX0;
            return false;
        }
        else if (e.type == EET_ZOOMOUT)
        {
            Action = ECA_DOLLY;
            MouseCurrentPoint.X	= -e.posX0;
            return false;
        }

		return true;
	}

	void TNodeCameraNav::UpdateCameraAction()
	{
		const vector2di& mouseStart = MouseStartPoint;
		const vector2di& mouseCurrent = MouseCurrentPoint;

		if (Action == ECA_ROLL)
		{
#ifdef TI_PLATFORM_IOS
			const float rot_speed = 0.2f;
#else
			const float rot_speed = 1.f;
#endif
			matrix4 mat;
			quaternion rotX, rotY, rot;
			vector3df tar_offset = OldPosition - OldTarget;
			vector3df axis = UpVector.crossProduct(tar_offset);
			axis.normalize();
			rotX.fromAngleAxis(-DEG_TO_RAD(mouseCurrent.X - mouseStart.X) * rot_speed, UpVector);
			rotY.fromAngleAxis(DEG_TO_RAD(mouseCurrent.Y - mouseStart.Y) * rot_speed, axis);
			rot = rotX * rotY;
			rot.getMatrix(mat);

			mat.transformVect(tar_offset);
			SetPosition(OldTarget + tar_offset);

			CameraFlags |= ECAMF_MAT_VIEW_DIRTY;
		}
		else if (Action == ECA_DOLLY)
		{
			vector3df p2t = Target - RelativePosition;
			float dis = p2t.getLength();
			float speed_adjust = 1.f;
			if (dis < 2.f)
				speed_adjust = 0.2f;
			p2t.normalize();
			vector3df speed = p2t * float(MouseCurrentPoint.X) * DollySpeed * speed_adjust;
			RelativePosition += speed;

			vector3df p2t1 = Target - RelativePosition;
			if (p2t1.dotProduct(p2t) < 0.f)
			{
				Target += speed;
			}

			NodeFlag |= ENF_DIRTY_POS;
			CameraFlags |= ECAMF_MAT_VIEW_DIRTY;

			Action = ECA_NONE;
			OldTarget = Target;
			OldPosition = RelativePosition;
		}
		
		if (Action == ECA_MOVE)
		{
			vector3df p2t = Target - RelativePosition;
			vector3df dirx = p2t.crossProduct(UpVector);
			dirx.normalize();
			vector3df diry = dirx.crossProduct(p2t);
			diry.normalize();

			vector3df move_offset = dirx * (float)(mouseCurrent.X - mouseStart.X) + diry * (float)(mouseCurrent.Y - mouseStart.Y);
			move_offset *= MoveSpeed;
			Target = OldTarget + move_offset;
			RelativePosition = OldPosition + move_offset;

			NodeFlag |= ENF_DIRTY_POS;
			CameraFlags |= ECAMF_MAT_VIEW_DIRTY;
		}
	}
}
