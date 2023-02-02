#pragma once

#include "Containers/UnrealString.h"

namespace StringMatchUtils
{
	void MatchStrings(const TArray<FString>& Targets, const FString& Pattern, TArray<int>& OutMatchResult);
}
