function onTarget(cid, target)

	if(isPlayer(cid) == TRUE and isMonster(target) and (getCreatureName(target) == "Marksman Target" or getCreatureName(target) == "Hitdoll")) then
		startShieldTrain(cid, target)
	end

	return true
end