function onKill(cid, target, damage, flags)
		
	local cName = getCreatureName(target)
	
	if(cName == "Ghazran") then
	
		onGhazranDie(corpse)
		
	elseif(cName == "Lord Vankyner") then
	
		onLordVankynerDie()
	end
	
	return TRUE

end