function onLogout(cid, forceLogout)
	
	local player_dungeonStatus = getPlayerStorageValue(cid, sid.DUNGEON_STATUS)
	
	if(player_dungeonStatus == dungeonStatus.IN_DUNGEON) then
		doPlayerSendTextMessage(cid, MESSAGE_INFO_DESCR, "N�o pode sair no meio de uma Dungeon!")
		return FALSE
	end	
	
	return TRUE
end 