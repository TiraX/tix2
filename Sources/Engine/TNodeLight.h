/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_LIGHT_FLAG
	{
		ELF_LIGHT_POS_DIRTY = 1 << 0,

		ELF_DISPLAYABLE_DIRTY = (ELF_LIGHT_POS_DIRTY ),
	};

	class TNodeLight : public TNode
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(Light);
	public:
		virtual ~TNodeLight();

		// light do not need scale and rotate,
		// make these 2 functions empty
		virtual void SetScale(const vector3df& scale);
		virtual void SetRotate(const quaternion& rotate);

		void SetLightFlag(uint32 flag, bool enable)
		{
			if (enable)
				LightFlag |= flag;
			else
				LightFlag &= ~flag;
		}
		uint32 GetLightFlag()
		{
			return	LightFlag;
		}
		void SetIntensity(float i)
		{
			Intensity = i;
		}
		float GetIntensity()
		{
			return	Intensity;
		}
		void SetDiffuse(const SColor& c)
		{
			Diffuse = c;
		}
		const SColor& GetDiffuse()
		{
			return Diffuse;
		}
		const aabbox3df& GetAffectBox()
		{
			return	AffectBox;
		}
	protected:
		virtual void UpdateAbsoluteTransformation();

	protected:
		float Intensity;
		SColor Diffuse;
		uint32 LightFlag;

		aabbox3df AffectBox;
	};
} // end namespace tix

