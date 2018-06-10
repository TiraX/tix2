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

		virtual void RegisterElement();

		virtual void SetPosition(const vector3df& pos);
		virtual void SetScale(const vector3df& scale);
		virtual void SetRotate(const quaternion& rotate);

		virtual void UpdateAbsoluteTransformation();
		virtual void UpdateAllTransformation();
		virtual const matrix4& GetAbsoluteTransformation() const;
		virtual const matrix4& GetRelativeTransformation();
		
		FNode* GetParent()
		{
			return Parent;
		}

		E_NODE_TYPE GetType() const
		{
			return NodeType;
		}

		uint32 GetFlag() const
		{
			return NodeFlag;
		}

		uint32	GetChildrenCount()
		{
			return (uint32)Children.size();
		}

		virtual const vector3df& GetRelativePosition() const
		{
			return RelativePosition;
		}

		virtual const quaternion& GetRelativeRotation() const
		{
			return RelativeRotate;
		}

		virtual const vector3df& GetRelativeScale() const
		{
			return RelativeScale;
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
		virtual void NotifyNode(const char* message, int param) {};

	protected:
		virtual bool RemoveChild(FNode* child);

	protected:
		union {
			E_NODE_TYPE		NodeType;
			char			NodeTypeName[4];
		};

		FNode*				Parent;
		typedef TVector<FNode*>	VecRenderElements;
		VecRenderElements	Children;

		uint32				NodeFlag;

		vector3df			RelativePosition;
		quaternion			RelativeRotate;
		vector3df			RelativeScale;

		matrix4				AbsoluteTransformation;
		matrix4				RelativeTransformation;
	};

} // end namespace tix

