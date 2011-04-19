function onThink(cid, interval)
	if(not isCreature(cid)) then
		return
	end

	local onIsland = (getPlayerStorageValue(cid, sid.IS_ON_TRAINING_ISLAND) == 1) and true or false
	
	if(not onIsland) then
		return
	end
	
	local tile = getTileInfo(getCreaturePosition(cid))
	
	if(not tile.optional) then
		setPlayerStorageValue(cid, sid.IS_ON_TRAINING_ISLAND, STORAGE_NULL)
		setPlayerStorageValue(cid, sid.NEXT_STAMINA_UPDATE, STORAGE_NULL)		
		
		return
	end
	
	
	
	local nextStaminaUpdate = getPlayerStorageValue(cid, sid.NEXT_STAMINA_UPDATE)
	
	if(nextStaminaUpdate ~= -1 and os.time() < nextStaminaUpdate) then
		return
	end
	
	local bonusStamina = 40 * 60
	local maxStamina = 42 * 60
	
	local staminaMinutes = getPlayerStamina(cid)
	local newStamina = staminaMinutes + 1
	
	local highStaminaInterval = (60 * 10)
	local lowStaminaInterval = (60 * 3)
	
	if(isPremium(cid)) then
		highStaminaInterval = (60 * 7)
		lowStaminaInterval = (60 * 2)	
	end
	
	if(staminaMinutes >= maxStamina) then
		return
	end
	
	if(newStamina >= bonusStamina) then		
		setPlayerStorageValue(cid, sid.NEXT_STAMINA_UPDATE, os.time() + highStaminaInterval)
	else	
		setPlayerStorageValue(cid, sid.NEXT_STAMINA_UPDATE, os.time() + lowStaminaInterval)
	end
	
	if(nextStaminaUpdate ~= -1) then
		doPlayerSetStamina(cid, newStamina)	
	end
end
