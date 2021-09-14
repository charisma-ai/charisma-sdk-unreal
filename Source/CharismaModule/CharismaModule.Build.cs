using System.IO;
using UnrealBuildTool;

public class CharismaModule : ModuleRules
{
	public CharismaModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ...
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				// ...				
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"CoreUObject",
				"Engine",
				"InputCore",
				"HTTP",
				"Json", 
				"JsonUtilities", 
				"ColyseusClient",
			}
		);

		PublicDefinitions.Add("WITH_OGGVORBIS");

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}
