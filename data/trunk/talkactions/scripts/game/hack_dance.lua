function onSay(cid, words, param)	
	
	local hackstate = doPlayerGetAfkState(cid)
	
	if(not(hackstate)) then
	
		local onIsland = (getPlayerStorageValue(cid, sid.IS_ON_TRAINING_ISLAND) == 1) and true or false
		
		if(getPlayerTown(cid) ~= towns.ISLAND_OF_PEACE and not onIsland) then
			doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "To use this command you must be within the training island.")
			return true
		end
	
		setPlayerAntiIdle(cid, ANTI_IDLE_INTERVAL)
		doPlayerSetAfkState(cid)
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Your anti-idle is set to enabled.")
	else
		setPlayerAntiIdle(cid, ANTI_IDLE_NONE)
		doPlayerRemoveAfkState(cid)
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Your anti-idle is set do disabled.")
	end
	
	return true
end