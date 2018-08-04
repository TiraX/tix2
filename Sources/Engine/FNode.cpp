/*
	Copyright (C) 2012
	By Zhao Shuai tirax.cn@gmail.com 2012.7.18
*/

#include "stdafx.h"
#include "FNode.h"

namespace tix
{
	FNode::FNode(E_NODE_TYPE type, FNode* parent)
		: Parent(nullptr)
		, Type(type)
		, NodeFlag(ENF_VISIBLE | ENF_DIRTY_POS)
	{
		if (parent)
			parent->AddChild(this);
	}

	FNode::~FNode()
	{
		Remove();
		RemoveAndDeleteAll();
	}

	void FNode::AddChild(FNode* child)
	{
		if (child && (child != this))
		{
			child->Remove(); // remove from old parent
			Children.push_back(child);
			child->Parent = this;
		}
	}

	void FNode::Remove()
	{
		if (Parent)
			Parent->RemoveChild(this);
	}

	bool FNode::RemoveChild(FNode* child)
	{
		VecRenderElements::iterator it = std::find(Children.begin(), Children.end(), child);
		if (it != Children.end())
		{
			(*it)->Parent = 0;
			Children.erase(it);
			return true;
		}
		return false;
	}

	void FNode::RemoveAndDeleteAll()
	{
		VecRenderElements::iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			(*it)->Parent = 0;
			ti_delete (*it);
		}
		Children.clear();
	}

	const matrix4& FNode::GetAbsoluteTransformation() const
	{
		return AbsoluteTransformation;
	}
}
