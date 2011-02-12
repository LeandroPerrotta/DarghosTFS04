function onCombat(cid, target)

	if(isPlayer(cid) == TRUE and isMonster(target) and (getCreatureName(target) == "Marksman Target" or getCreatureName(target) == "Hitdoll")) then
		startShieldTrain(cid, target)
	end

	--checks attacker
	if(server_distro == DISTROS_OPENTIBIA) then
		local player_attacker = nil
		if(isPlayer(cid) == TRUE) then
			player_attacker = cid
		elseif(isPlayer(getCreatureMaster(cid)) == TRUE) then
			player_attacker = getCreatureMaster(cid)
		end
	
		--checks target
		local player_target = nil
		if(isPlayer(target) == TRUE) then
			player_target = target
		elseif(isPlayer(getCreatureMaster(target)) == TRUE) then
			player_target = getCreatureMaster(target)
		end
	
		if(player_attacker ~= nil and player_target ~= nil) then	
			if(getPlayerTown(player_attacker) == towns.ISLAND_OF_PEACE or
			   getPlayerTown(player_target) == towns.ISLAND_OF_PEACE) then
				return false
			end
		end
	end

	return true
end