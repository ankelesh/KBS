#include "Presentation/KBSPresentationLog.h"

DEFINE_LOG_CATEGORY(LogKBSPresentation)

TAutoConsoleVariable<int32> CVarDumpSequence(
	TEXT("kbs.Presentation.DumpSequence"),
	0,
	TEXT("Non-zero: log each assembled presentation action plus montage resolution failures."),
	ECVF_Default);
