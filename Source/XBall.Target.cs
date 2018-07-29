// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class XBallTarget : TargetRules
{
	public XBallTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
        bForceEnableExceptions = true;
        ExtraModuleNames.AddRange( new string[] { "XBall" } );
	}
}
