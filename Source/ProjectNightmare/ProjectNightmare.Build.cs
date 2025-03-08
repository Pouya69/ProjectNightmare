// Fill out your copyright notice in the Description page of Project Settings.

using System;
using System.IO;
using UnrealBuildTool;

public class ProjectNightmare : ModuleRules
{
	public ProjectNightmare(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "LevelSequence", "MovieScene", "MovieSceneTracks", "UMG", "GameplayTasks", "NavigationSystem", "Niagara" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });


        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "..\\icp-client-cpp\\lib-agent-c\\inc"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "..\\icp-client-cpp\\lib-agent-cpp\\inc"));
        PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "..\\icp-client-cpp\\ic-agent-wrapper\\target\\release", "ic_agent_wrapper.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "..\\icp-client-cpp\\build\\Release", "agent_c.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "..\\icp-client-cpp\\build\\Release", "agent_cpp.lib"));

        PublicAdditionalLibraries.AddRange(
            new string[] {
                // "Path/To/ICP-Client/Lib/agent_cpp.lib",
                "kernel32.lib",
                "userenv.lib",
                "bcrypt.lib",
                "ntdll.lib"
            });
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
