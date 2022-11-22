#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "DisplayLine.generated.h"

USTRUCT(BlueprintType)
struct FDisplayLine : public FTableRowBase
{
    GENERATED_BODY();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Display Line")
    FText Text;
};