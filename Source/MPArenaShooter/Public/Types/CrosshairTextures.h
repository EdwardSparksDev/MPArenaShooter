#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "CrosshairTextures.generated.h"


USTRUCT(BlueprintType)
struct FCrosshairTextures
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D *CrosshairCenter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D *CrosshairTop = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D *CrosshairBottom = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D *CrosshairRight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D *CrosshairLeft = nullptr;
};


FORCEINLINE bool operator==(const FCrosshairTextures &A, const FCrosshairTextures &B)
{
	return A.CrosshairCenter == B.CrosshairCenter &&
		A.CrosshairTop == B.CrosshairTop &&
		A.CrosshairBottom == B.CrosshairBottom &&
		A.CrosshairLeft == B.CrosshairLeft &&
		A.CrosshairRight == B.CrosshairRight;
}


FORCEINLINE bool operator!=(const FCrosshairTextures &A, const FCrosshairTextures &B)
{
	return !(A == B);
}