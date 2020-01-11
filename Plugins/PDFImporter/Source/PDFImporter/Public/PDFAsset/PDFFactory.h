#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PDFFactory.generated.h"

UCLASS()
class PDFIMPORTER_API UPDFFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

public:
	// UFactory interface
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual UClass* ResolveSupportedClass() override;
	virtual UObject* FactoryCreateFile(
		UClass* InClass,
		UObject* InParent,
		FName InName,
		EObjectFlags Flags,
		const FString& Filename,
		const TCHAR* Parms,
		FFeedbackContext* Warn,
		bool& bOutOperationCanceled
	)override;
	// End of UFactory interface
};
