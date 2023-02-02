#include "StringMatchUtils.h"

#define LOCTEXT_NAMESPACE "StringMatchUtils"

namespace StringMatchUtils
{
	void CalculateNext(const FString& Pattern, TArray<int>& OutNext)
	{
		int q, k;
		int m = Pattern.Len();
		OutNext[0] = 0;

		for (q = 1, k = 0; q < m; q ++)
		{
			while (k > 0 && Pattern[q] != Pattern[k])
			{
				k = OutNext[k - 1];
			}

			if (Pattern[q] == Pattern[k])
			{
				k ++;
			}
			OutNext[q] = k;
		}
	}

	bool MatchByKMP(const FString& Target, const FString& Pattern)
	{
		if (Pattern.IsEmpty())
		{
			return false;
		}

		int n, m;
		int i, q;
		n = Target.Len();
		m = Pattern.Len();

		TArray<int> Next;
		Next.AddUninitialized(m);
		CalculateNext(Pattern, Next);

		for (i = 0, q = 0; i < n; i ++)
		{
			while (q > 0 && Pattern[q] != Target[i])
			{
				q = Next[q - 1];
			}

			if (Pattern[q] == Target[i])
			{
				q ++;
			}

			if (q == m)
			{
				return true;
			}
		}
		return false;
	}

	void MatchStrings(const TArray<FString>& Targets, const FString& Pattern, TArray<int>& OutMatchResult)
	{
		if (Pattern.IsEmpty())
		{
			return;
		}

		for (int Index = 0; Index < Targets.Num(); Index ++)
		{
			if (MatchByKMP(Targets[Index], Pattern))
			{
				OutMatchResult.Add(Index);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
