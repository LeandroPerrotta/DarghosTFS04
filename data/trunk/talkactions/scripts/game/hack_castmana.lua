function onSay(cid, words, param)	
	
	local hackstate = getPlayerStorageValue(cid, sid.HACKS_CASTMANA)
	
	if(hackstate == STORAGE_NULL) then
		setPlayerStorageValue(cid, sid.HACKS_CASTMANA, 1)
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "When your mana over 75% of its ful manal will be spent 50%. Mana cast is set to enabled.")
	else
		setPlayerStorageValue(cid, sid.HACKS_CASTMANA, -1)
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Your mana cast is set do disabled.")
	end
	
	return true
end