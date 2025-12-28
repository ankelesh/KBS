#include "Modules/ModuleManager.h"

class FKBSTestsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FKBSTestsModule, KBSTests)