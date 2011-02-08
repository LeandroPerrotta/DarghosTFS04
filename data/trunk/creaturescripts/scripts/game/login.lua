function onLogin(cid)

	--print("Custom login done!")

	--Register the kill/die event
	registerCreatureEvent(cid, "CustomPlayerDeath")
	registerCreatureEvent(cid, "CustomStages")
	registerCreatureEvent(cid, "Inquisition")
	registerCreatureEvent(cid, "CustomPlayerAttack")
	
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
	
	setPlayerStorageValue(cid, sid.TRAINING_SHIELD, 0)
	setPlayerStorageValue(cid, sid.TELEPORT_RUNE_STATE, -1)
	
	return TRUE
end
