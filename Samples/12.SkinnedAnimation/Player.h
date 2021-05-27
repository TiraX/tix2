/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class Player 
{
public:
	enum
	{
		ActionMoveForward,
		ActionMoveLeft,
		ActionMoveRight,
		ActionMoveBack
	};

	Player();
	virtual ~Player();

	void DoAction(int32 Action);
	void LoadResources();
protected:


protected:
	TNodeSkeletalMesh* SkeletalMesh;
};