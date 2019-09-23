/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct FEnvironmentInfo
	{
		vector3df MainLightDir;
		SColorf MainLightColor;
		float MainLightIntensity;
	};

	class TNodeEnvironment : public TNode
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(Environment);
	public:
		enum E_ENV_FLAG
		{
			ENVF_MAIN_LIGHT_DIRTY = 1 << 0,
		};
		virtual ~TNodeEnvironment();

		virtual void UpdateAllTransformation() override;

		TI_API void SetMainLightDirection(const vector3df& InDir);
		TI_API void SetMainLightColor(const SColorf& InColor);
		TI_API void SetMainLightIntensity(float InIntensity);

		const vector3df& GetMainLightDirection() const
		{
			return MainLightDirection;
		}
		const SColorf& GetMainLightColor() const
		{
			return MainLightColor;
		}
		float GetMainLightIntensity() const
		{
			return MainLightIntensity;
		}
	protected:

	protected:
		uint32 EnvFlags;
		vector3df MainLightDirection;
		SColorf MainLightColor;
		float MainLightIntensity;
	};

} // end namespace tix

