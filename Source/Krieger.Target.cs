// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class KriegerTarget : TargetRules
{
	public KriegerTarget(TargetInfo Target)
	{
		Type = TargetType.Game;
	}

	//
	// TargetRules interface.
	//

	public override void SetupBinaries(
		TargetInfo Target,
		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
		ref List<string> OutExtraModuleNames
		)
	{
		OutExtraModuleNames.AddRange( new string[] { "Krieger" } );
	}
}
