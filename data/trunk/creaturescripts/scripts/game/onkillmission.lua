function onKill(cid, target, damage, flags)
	
	-- Bonartes Mission's	
	local _demonMission = getPlayerStorageValue(cid, QUESTLOG.MISSION_BONARTES.KILL_DEMONS)
	local _heroMission = getPlayerStorageValue(cid, QUESTLOG.MISSION_BONARTES.KILL_HEROS)
	local _behemothMission = getPlayerStorageValue(cid, QUESTLOG.MISSION_BONARTES.KILL_BEHEMOTHS)	
	
	local _targetName = getCreatureName(target)
	
	if(_targetName == "hero" and _heroMission == 2) then
		local kills = getPlayerStorageValue(cid, sid.BONARTES_HERO_KILLS)
		
		if(kills == -1) then
			kills = 0
		end
		
		kills = kills + 1
		
		setPlayerStorageValue(cid, sid.BONARTES_HERO_KILLS, kills)
		
		if(kills == KILL_MISSIONS.BONARTES_HERO) then
			doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Voc� j� derrotou heros sulficientes, v� falar com o Bonartes e o diga que terminou a miss�o.")
		else
			doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Voc� derrotou " .. kills .. " heroes.")
		end
	elseif(_targetName == "behemoth" and _behemothMission == 1) then
		local kills = getPlayerStorageValue(cid, sid.BONARTES_BEHEMOTH_KILLS)
		
		if(kills == -1) then
			kills = 0
		end
		
		kills = kills + 1
		
		setPlayerStorageValue(cid, sid.BONARTES_BEHEMOTH_KILLS, kills)	
		
		if(kills == KILL_MISSIONS.BONARTES_BEHEMOTH) then
			doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Voc� j� derrotou behemoths sulficientes, v� falar com o Bonartes e o diga que terminou a miss�o.")
		else
			doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Voc� derrotou " .. kills .. " behemoths.")
		end
	elseif(_targetName == "demon" and _demonMission == 1) then
		local kills = getPlayerStorageValue(cid, sid.BONARTES_DEMON_KILLS)
		
		if(kills == -1) then
			kills = 0
		end
		
		kills = kills + 1
		
		setPlayerStorageValue(cid, sid.BONARTES_DEMON_KILLS, kills)		
		
		if(kills == KILL_MISSIONS.BONARTES_DEMON) then
			doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Voc� j� derrotou demons sulficientes, v� falar com o Bonartes e o diga que terminou a miss�o.")
		else
			doPlayerSendTextMessage(cid, MESSAGE_EVENT_ADVANCE, "Voc� derrotou " .. kills .. " demons.")
		end		
	end		
	
	return TRUE

end