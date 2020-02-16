// Some copyright should be here...

using UnrealBuildTool;

public class PDFImporterEd : ModuleRules
{
	public PDFImporterEd(ReadOnlyTargetRules Target) : base(Target)
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
                "PDFImporter",
                "UnrealEd",
                "AssetTools",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "PDFViewer",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Projects",
                "PropertyEditor",
                "EditorStyle",
				// ... add private dependencies that you statically link with here ...	
			}
			);
	}
}
