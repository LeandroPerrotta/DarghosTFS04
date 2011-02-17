function onSay(cid, words, param)	
	
	local hackstate = getPlayerStorageValue(cid, sid.HACKS_DANCE_EVENT)
	
	if(hackstate == STORAGE_NULL) then
		setPlayerAntiIdle(cid, ANTI_IDLE_INTERVAL)
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Your anti-idle is set to enabled.")
	else
		setPlayerAntiIdle(cid, ANTI_IDLE_NONE)
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Your anti-idle is set do disabled.")
	end
	
	return true
end