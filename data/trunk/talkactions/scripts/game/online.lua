local config = {
	showGamemasters = getBooleanFromString(getConfigValue('displayGamemastersWithOnlineCommand'))
}

UPDATE_LIST_INTERVAL = 1000
onlineList = {}

function updateOnlineList()

	onlineList = {}

	local result = db.getResult("SELECT `name`, `level` FROM `players` WHERE `online` = '1' ORDER BY `name`");
	
	if(result:getID() ~= -1) then
		repeat
			local pname, plevel = result:getDataString("name"), result:getDataInt("level")
			local player = {name = pname, level = plevel}
			table.insert(onlineList, player)
		until not(result:next())
	end
end

function onSay(cid, words, param, channel)
	
	local lastWhoIsOnline = getGlobalStorageValue(gid.LAST_WHO_IS_ONLINE)
	
	if(lastWhoIsOnline == STORAGE_NULL or (lastWhoIsOnline + UPDATE_LIST_INTERVAL) <= os.time() or onlineList == nil) then
		updateOnlineList()
	end

	doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, (#onlineList) .. " player" .. (#onlineList > 1 and "s" or "") .. " online:")
	
	local str, i, pos = "", 1, 1;
	
	for _, info in ipairs(onlineList) do
	
		local pid = getCreatureByName(info.name)
		
		local canAdd = true
		
		if(creature ~= nil) then
			if((config.showGamemasters or getPlayerCustomFlagValue(cid, PLAYERCUSTOMFLAG_GAMEMASTERPRIVILEGES) or not getPlayerCustomFlagValue(pid, PLAYERCUSTOMFLAG_GAMEMASTERPRIVILEGES)) and (not isPlayerGhost(pid) or getPlayerGhostAccess(cid) >= getPlayerGhostAccess(pid))) then
				canAdd = false
			end
		end
	
		if(canAdd) then
			if(pos < 20) then
				str = str .. info.name .. " [" .. info.level .. "]"
				pos = pos + 1
			else
				doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, str)
				
				str = info.name .. " [" .. info.level .. "]"
				pos = 1
			end		
			
			if(i == #onlineList) then
				str = str .. "."
			else
				str = str .. ", "
			end				
		end		
		
		i = i + 1
	end

	if(pos > 1) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, str)
	end
	
	return true
end
