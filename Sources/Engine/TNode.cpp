/*
	Copyright (C) 2012
	By Zhao Shuai tirax.cn@gmail.com 2012.7.18
*/

#include "stdafx.h"
#include "TNode.h"

namespace tix
{
	TNode::TNode(E_NODE_TYPE type, TNode* parent)
		: NodeType(type)
		, Parent(NULL)
		, NodeFlag(ENF_VISIBLE | ENF_DIRTY_POS)
		, RelativeRotate(0.f, 0.f, 0.f, 1.f)
		, RelativeScale(1.f, 1.f, 1.f)
	{
		if (parent)
			parent->AddChild(this);
	}

	TNode::~TNode()
	{
		Remove();
		RemoveAndDeleteAll();
	}

	void TNode::AddChild(TNode* child)
	{
		if (child && (child != this))
		{
			child->Remove(); // remove from old parent
			Children.push_back(child);
			child->Parent = this;
		}
	}

	void TNode::Remove()
	{
		if (Parent)
			Parent->RemoveChild(this);
	}

	bool TNode::RemoveChild(TNode* child)
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

	void TNode::RemoveAndDeleteAll()
	{
		VecRenderElements::iterator it = Children.begin();
		for (; it != Children.end(); ++it)
		{
			(*it)->Parent = 0;
			ti_delete (*it);
		}
		Children.clear();
	}

	void TNode::SetPosition(const vector3df& pos)
	{
		RelativePosition	= pos;
		NodeFlag		|= ENF_DIRTY_POS;
	}

	void TNode::SetScale(const vector3df& scale)
	{
		RelativeScale		= scale;
		NodeFlag		|= ENF_DIRTY_SCALE;
	}

	void TNode::SetRotate(const quaternion& rotate)
	{
		RelativeRotate	= rotate;
		NodeFlag		|= ENF_DIRTY_ROT;
	}
	
	TNode* TNode::GetNodeById(const TString& uid)
	{
		if (NodeId == uid)
			return this;

		VecRenderElements::const_iterator it = Children.begin();
		for ( ; it != Children.end() ; ++ it)
		{
			TNode* node = (*it)->GetNodeById(uid);
			if (node)
				return node;
		}

		return NULL;
	}

	void TNode::GetNodesByType(E_NODE_TYPE type, TVector<TNode*>& nodes)
	{
		if ( NodeType == type)
		{
			nodes.push_back(this);
		}
		VecRenderElements::const_iterator it = Children.begin();
		for ( ; it != Children.end() ; ++ it)
		{
			(*it)->GetNodesByType(type, nodes);
		}
	}

	void TNode::Update(float dt)
	{
		// update children
		VecRenderElements::const_iterator it = Children.begin();
		for ( ; it != Children.end() ; ++ it)
		{
			(*it)->Update(dt);
		}
	}

	TNode* TNode::IsIntersectWithRay(const line3df& ray, aabbox3df& outBBox, vector3df& outIntersection)
	{
		// test children
		VecRenderElements::const_iterator it = Children.begin();
		for ( ; it != Children.end() ; ++ it)
		{
			TNode* node	= (*it)->IsIntersectWithRay(ray, outBBox, outIntersection);
			if (node)
			{
				return node;
			}
		}

		return NULL;
	}

	TNode* TNode::IsIntersectWithPoint(const vector3df& p, aabbox3df& outBBox, vector3df& outIntersection)
	{
		// test children
		VecRenderElements::const_iterator it = Children.begin();
		for ( ; it != Children.end() ; ++ it)
		{
			TNode* node	= (*it)->IsIntersectWithPoint(p, outBBox, outIntersection);
			if (node)
			{
				return node;
			}
		}

		return NULL;
	}

	TNode* TNode::GetParent(E_NODE_TYPE type)
	{
		TNode* parent = this;
		while (parent = parent->GetParent())
		{
			if (parent->GetType() == type)
			{
				return parent;
			}
		}
		return NULL;
	}
}
