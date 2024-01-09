// Copyright Capoom Inc. All Rights Reserved.

using System.IO;
using System;
using UnrealBuildTool;

public class OPUS : ModuleRules
{
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/")); }
    }
	/*
    private string SevenZipPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "bit7z")); }
    }
	private string SevenZipIncludePath
	{
        get { return Path.GetFullPath(Path.Combine(SevenZipPath, "include/bit7z")); }
    }
    private string SevenZipLibraryPath
    {
        get { return Path.GetFullPath(Path.Combine(SevenZipPath, "lib/x64/Release/bit7z.lib")); }
    }
    private string SevenZipDLLPath
    {
        get { return Path.GetFullPath(Path.Combine(SevenZipPath, "dll/7za.dll")); }
    }
	*/
    public OPUS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				Path.Combine(ModuleDirectory, "Public"),

            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
                Path.Combine(ModuleDirectory, "Private"),
				//SevenZipIncludePath

            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"HTTP",
				"Json"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"HTTP"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		/*
        PublicAdditionalLibraries.Add(SevenZipLibraryPath);
        PublicDelayLoadDLLs.Add("7za.dll");
        RuntimeDependencies.Add(SevenZipDLLPath);
		*/
    }
}
