function onSay(cid, words, param)	
	
	local hackstate = getPlayerStorageValue(cid, sid.HACKS_LIGHT)
	
	if(hackstate == LIGHT_NONE) then
		setPlayerLight(cid, LIGHT_FULL)
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Your full light is set to enabled.")
	else
		setPlayerLight(cid, LIGHT_NONE)
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Your full light is set do disabled.")
	end
	
	return true
end