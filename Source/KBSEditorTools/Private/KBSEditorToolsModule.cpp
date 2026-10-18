#include "KBSEditorToolsModule.h"

DEFINE_LOG_CATEGORY(LogKBSEditorTools);

IMPLEMENT_MODULE(FKBSEditorToolsModule, KBSEditorTools)

void FKBSEditorToolsModule::StartupModule()
{
	UE_LOG(LogKBSEditorTools, Log, TEXT("KBSEditorTools loaded"));
}

void FKBSEditorToolsModule::ShutdownModule()
{
}
