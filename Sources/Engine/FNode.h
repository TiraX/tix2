/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TScene;

	class FNode 
	{
	public:
		FNode(E_NODE_TYPE type, FNode* parent);
		virtual ~FNode();

		virtual void AddChild(FNode* child);		// add child at the end of children
		virtual void Remove();

		virtual const matrix4& GetAbsoluteTransformation() const;
		
		FNode* GetParent()
		{
			return Parent;
		}

		uint32 GetFlag() const
		{
			return NodeFlag;
		}

		uint32	GetChildrenCount()
		{
			return (uint32)Children.size();
		}

		virtual vector3df GetAbsolutePosition()
		{
			return AbsoluteTransformation.getTranslation();
		}

		inline bool IsVisible()
		{
			return (NodeFlag & ENF_VISIBLE) != 0;
		}
		inline void SetVisible(bool visible)
		{
			if (visible)
				NodeFlag |= ENF_VISIBLE;
			else
				NodeFlag &= ~ENF_VISIBLE;
		}

		virtual void RemoveAndDeleteAll();

	protected:
		virtual bool RemoveChild(FNode* child);

	protected:

		FNode*				Parent;
		typedef TVector<FNode*>	VecRenderElements;
		VecRenderElements	Children;

		uint32				NodeFlag;

		// Node in render thread only used for render thread frustum cull, only need absolute transform
		// Absolute transformation is passed from game thread
		matrix4				AbsoluteTransformation;
	};

} // end namespace tix

