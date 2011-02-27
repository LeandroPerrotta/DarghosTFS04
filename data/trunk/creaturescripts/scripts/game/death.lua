function onDeath(cid, corpse, deathList)
	if isPlayer(cid) == TRUE then
		--Fun��es que ser�o chamadas quando um jogador morrer...
		
		Dungeons.onPlayerDeath(cid)
		setPlayerStorageValue(cid, sid.GIVE_ITEMS_AFTER_DEATH, 1)
	end
	
	return true
end 