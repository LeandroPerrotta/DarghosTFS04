function onSay(cid, words, param, channel)
	
	local storage = getPlayerStorageValue(cid, sid.BANNED_IN_HELP)
	
	if(storage == STORAGE_NULL or storage == 0) then
		return false
	end
	
	if(storage >= os.time()) then
		addEvent(doPlayerSendChannelMessage, 150, cid, "", "Você foi proibido de enviar mensagens no help channel até " .. os.date("%d/%m/%Y - %X", storage) .. " devido uma conduta inaceitavel recente.", TALKTYPE_CHANNEL_W, CHANNEL_HELP)
		return true
	end

	return false
end