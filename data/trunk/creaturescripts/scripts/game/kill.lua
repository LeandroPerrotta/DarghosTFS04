function onKill(cid, target, damage, flags)
		
	local cName = getCreatureName(cid)
	
	if(cName == "Ghazran") then
	
		onGhazranDie(corpse)
		
	elseif(cName == "Lord Vankyner") then
	
		onLordVankynerDie()
	end
	
	return TRUE

end