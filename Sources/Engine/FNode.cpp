/*
	Copyright (C) 2012
	By Zhao Shuai tirax.cn@gmail.com 2012.7.18
*/

#include "stdafx.h"
#include "FNode.h"

namespace tix
{
	FNode::FNode(E_NODE_TYPE type, FNode* parent)
		: NodeType(type)
		, Parent(NULL)
		, NodeFlag(ENF_VISIBLE | ENF_DIRTY_POS)
		, RelativeRotate(0.f, 0.f, 0.f, 1.f)
		, RelativeScale(1.f, 1.f, 1.f)
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

	void FNode::RegisterElement()
	{
        if (!IsVisible())
        {
            return;
        }
		UpdateAbsoluteTransformation();

		VecRenderElements::const_iterator it = Children.begin();
		for ( ; it != Children.end() ; ++ it)
		{
			(*it)->RegisterElement();
		}

		NodeFlag &= ~ENF_ABSOLUTETRANSFORMATION_UPDATED;
	}

	void FNode::SetPosition(const vector3df& pos)
	{
		RelativePosition	= pos;
		NodeFlag		|= ENF_DIRTY_POS;
	}

	void FNode::SetScale(const vector3df& scale)
	{
		RelativeScale		= scale;
		NodeFlag		|= ENF_DIRTY_SCALE;
	}

	void FNode::SetRotate(const quaternion& rotate)
	{
		RelativeRotate	= rotate;
		NodeFlag		|= ENF_DIRTY_ROT;
	}

	const matrix4& FNode::GetAbsoluteTransformation() const
	{
		return AbsoluteTransformation;
	}

	const matrix4& FNode::GetRelativeTransformation()
	{
		if ((NodeFlag & (ENF_DIRTY_TRANSFORM)) != 0)
		{
			if (NodeFlag & (ENF_DIRTY_SCALE | ENF_DIRTY_ROT))
			{
				RelativeRotate.getMatrix(RelativeTransformation);
				if (RelativeScale != vector3df(1.f, 1.f, 1.f))
				{
					RelativeTransformation.postScale(RelativeScale);
				}
				RelativeTransformation.setTranslation(RelativePosition);
			}
			else
			{
				RelativeTransformation.setTranslation(RelativePosition);
			}
			NodeFlag &= ~ENF_DIRTY_TRANSFORM;
		}
		return RelativeTransformation;
	}

	void FNode::UpdateAllTransformation()
	{
		UpdateAbsoluteTransformation();

		VecRenderElements::const_iterator it = Children.begin();
		for ( ; it != Children.end() ; ++ it)
		{
			(*it)->UpdateAllTransformation();
		}

		NodeFlag &= ~ENF_ABSOLUTETRANSFORMATION_UPDATED;
	}

	void FNode::UpdateAbsoluteTransformation()
	{
		if (Parent && 
		   ((Parent->NodeFlag & ENF_ABSOLUTETRANSFORMATION_UPDATED) || 
		    (NodeFlag & ENF_DIRTY_TRANSFORM)))
		{
			Parent->GetAbsoluteTransformation().mult34(GetRelativeTransformation(), AbsoluteTransformation);
			NodeFlag |= ENF_ABSOLUTETRANSFORMATION_UPDATED;
		}
		else
		{
			if (NodeFlag & ENF_DIRTY_TRANSFORM)
			{
				AbsoluteTransformation = GetRelativeTransformation();
				NodeFlag |= ENF_ABSOLUTETRANSFORMATION_UPDATED;
			}
		}
	}
}
