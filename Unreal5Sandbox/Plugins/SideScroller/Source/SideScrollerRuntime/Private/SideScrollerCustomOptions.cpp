#include "SideScrollerCustomOptions.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    constexpr const TCHAR* ObjectKey = TEXT("SCObjects");

    bool IsValidCategory(int32 Value)
    {
        switch (Value)
        {
        case static_cast<int32>(ESideScrollerCategory::Undefined):
        case static_cast<int32>(ESideScrollerCategory::PL):
        case static_cast<int32>(ESideScrollerCategory::EM):
        case static_cast<int32>(ESideScrollerCategory::GMK): return true;
        default: return false;
        }
    }

    bool ParseInt32(const FString& Text, int32& OutValue)
    {
        if (Text.IsEmpty())
        {
            return false;
        }
        
        const int32 FirstDigit = Text[0] == TEXT('-') ? 1 : 0;
        if (FirstDigit == Text.Len()) return false;
        for (int32 Index = FirstDigit; Index < Text.Len(); ++Index)
        {
            if (Text[Index] < TEXT('0') || Text[Index] > TEXT('9')) return false;
        }
        int64 Value = 0;
        if (!LexTryParseString(Value, *Text) || Value < MIN_int32 || Value > MAX_int32) return false;
        OutValue = static_cast<int32>(Value);
        return true;
    }
}

bool SideScrollerCustomOptions::BuildCustomOptions(
    const TArray<FSideScrollerObjectInfo>& Objects, FString& OutOptions, FString& OutError)
{
    OutOptions.Reset();
    OutError.Reset();
    TArray<FString> Entries;
    Entries.Reserve(Objects.Num());
    for (int32 Index = 0; Index < Objects.Num(); ++Index)
    {
        const FSideScrollerObjectInfo& Info = Objects[Index];
        const int32 Category = static_cast<int32>(Info.Category);
        if (!IsValidCategory(Category))
        {
            OutError = FString::Printf(TEXT("ObjectInfos[%d] has an unknown Category: %d"), Index, Category);
            return false;
        }
        Entries.Add(FString::Printf(TEXT("%d:%d"), Category, Info.Id));
    }
    OutOptions = FString(ObjectKey) + TEXT("=") + FString::Join(Entries, TEXT(","));
    return true;
}

SideScrollerCustomOptions::EParseResult SideScrollerCustomOptions::ParseCustomOptions(
    const FString& Options, TArray<FSideScrollerObjectInfo>& OutObjects, FString& OutError)
{
    OutObjects.Reset();
    OutError.Reset();
    if (!UGameplayStatics::HasOption(Options, ObjectKey)) return EParseResult::NotProvided;

    const FString Payload = UGameplayStatics::ParseOption(Options, ObjectKey);
    if (Payload.IsEmpty()) return EParseResult::Success;

    TArray<FString> Entries;
    Payload.ParseIntoArray(Entries, TEXT(","), false);
    TArray<FSideScrollerObjectInfo> ParsedObjects;
    ParsedObjects.Reserve(Entries.Num());
    for (int32 Index = 0; Index < Entries.Num(); ++Index)
    {
        FString CategoryText;
        FString IdText;
        int32 Category = 0;
        FSideScrollerObjectInfo Info;
        if (!Entries[Index].Split(TEXT(":"), &CategoryText, &IdText) ||
            !ParseInt32(CategoryText, Category) || !IsValidCategory(Category) || !ParseInt32(IdText, Info.Id))
        {
            OutError = FString::Printf(TEXT("Invalid SCObjects entry [%d]: '%s' (expected CategoryInt:IdInt)."), Index, *Entries[Index]);
            return EParseResult::Invalid;
        }
        Info.Category = static_cast<ESideScrollerCategory>(Category);
        ParsedObjects.Add(Info);
    }
    OutObjects = MoveTemp(ParsedObjects);
    return EParseResult::Success;
}