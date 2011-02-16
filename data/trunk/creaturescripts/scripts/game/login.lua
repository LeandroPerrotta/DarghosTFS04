function onLogin(cid)

	--print("Custom login done!")

	--Register the kill/die event
	registerCreatureEvent(cid, "CustomPlayerDeath")
	registerCreatureEvent(cid, "CustomStages")
	registerCreatureEvent(cid, "Inquisition")
	registerCreatureEvent(cid, "CustomPlayerTarget")
	
	--if(tasks.hasStartedTask(cid)) then
		registerCreatureEvent(cid, "CustomTasks")
	--end
	
	playerRecord()
	runPremiumSystem(cid)
	setRateStage(cid, getPlayerLevel(cid))
	setLoginSkillsRateStage(cid)
	checkItemShop(cid)
	OnKillCreatureMission(cid)
	Dungeons.onLogin(cid)
	--defineFirstItems(cid)
	
	-- island of peace non pvp for TFS via onLogin
	if(darghos_distro == DISTROS_TFS) then
		if(getPlayerTown(cid) == towns.ISLAND_OF_PEACE and getPlayerGroupId(cid) < GROUP_PLAYER_TUTOR) then
			doPlayerSetGroupId(cid, GROUP_PLAYER_NON_PVP)
		end
	end
	
	setPlayerStorageValue(cid, sid.TRAINING_SHIELD, 0)
	setPlayerStorageValue(cid, sid.TELEPORT_RUNE_STATE, -1)
	
	return TRUE
end