function onStartup()

	spoofPlayers()
	Dungeons.onServerStart()
	summonLordVankyner()	
	
	local sendPlayerToTemple = getGlobalStorageValue(gid.SEND_PLAYERS_TO_TEMPLE)
	
	if(sendPlayerToTemple == 1) then
		db.executeQuery("UPDATE `players` SET `posx` = '0', `posy` = '0', `posz` = '0';")
		setGlobalStorageValue(gid.SEND_PLAYERS_TO_TEMPLE, 0)
		print("Sending players to temple.")
	end	
	
	return true
end