// Some copyright should be here...

using UnrealBuildTool;
using System.IO;

public class PDFImporter : ModuleRules
{
	public PDFImporter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "SlateCore",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "ImageWrapper",
				// ... add private dependencies that you statically link with here ...	
			}
			);

        string Platform = string.Empty;
        string LibName = string.Empty;
        string DllName = string.Empty;
        if (Target.Platform == UnrealTargetPlatform.Win32)
        {
            Platform = "x86";
            LibName = "Ghostscript32.lib";
            DllName = "gsdll32.dll";
        }
        else
        {
            Platform = "x64";
            LibName = "Ghostscript64.lib";
            DllName = "gsdll64.dll";
        }
        string LibrariesPath = Path.Combine(ModuleDirectory, "Libraries", Platform);
        string BinariesPath = Path.Combine(ModuleDirectory, "..", "..", "Binaries", Target.Platform.ToString());

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Includes"));
        PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, LibName));

        RuntimeDependencies.Add(Path.Combine(LibrariesPath, DllName));
        CopyFile(Path.Combine(LibrariesPath, DllName), Path.Combine(BinariesPath, DllName));
	}

    private void CopyFile(string Source, string Destination)
    {
        System.Console.WriteLine("Copying {0} to {1}", Source, Destination);

        if(File.Exists(Destination))
        {
            File.SetAttributes(Destination, File.GetAttributes(Destination) & ~FileAttributes.ReadOnly);
        }

        try
        {
            File.Copy(Source, Destination, true);
        }
        catch(System.Exception e)
        {
            System.Console.WriteLine("Failed to copy file : {0}", e.Message);
        }
    }
}
