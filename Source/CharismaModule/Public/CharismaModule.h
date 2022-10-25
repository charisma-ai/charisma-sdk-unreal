// Copyright 2022 Charisma Entertainment Ltd

#pragma once

#include "Modules/ModuleInterface.h"

class FCharismaModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
};
