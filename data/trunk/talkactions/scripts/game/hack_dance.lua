function onSay(cid, words, param)	
	
	local hackstate = doPlayerGetAfkState(cid)
	
	if(not(hackstate)) then
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