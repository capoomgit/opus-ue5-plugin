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
	
    private string LibZipPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "libzipp/Debug")); }
    }
	private string LibZipIncludePath
	{
        get { return Path.GetFullPath(Path.Combine(LibZipPath, "include")); }
    }
    private string LibZipLibraryPath
    {
        get { return Path.GetFullPath(Path.Combine(LibZipPath, "lib/libzippp.lib")); }
    }
    private string LibZipDLLPath
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "dll/libzippp.dll")); }
    }

	private string MinizPath
	{
		get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "miniz-3.0.2")); }
	}

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
				//LibZipIncludePath
				//MinizPath

            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"FileUtilities",
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
		
        //PublicAdditionalLibraries.Add(CapoomUnzipDLL);
        //PublicDelayLoadDLLs.Add("unzip_library.dll");
        //RuntimeDependencies.Add(CapoomUnzipDLL);
		
    }
}
