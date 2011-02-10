function onSay(cid, words, param)
	local playerPos = getPlayerPosition(cid)

	if(param == "") then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "You need to type the parameter.")
		doSendMagicEffect(playerPos, CONST_ME_POFF)
		return true
	else
		param = string.explode(param, ";")
	end

	local player = nil

	local action = "add"
	local duration = 24
	
	if(# param == 1) then
		player = getCreatureByName(param[1])
	elseif(# param == 2) then
		player = getCreatureByName(param[1])
		action = string.lower(param[2])
	elseif(# param == 3) then
		player = getCreatureByName(param[1])
		action = string.lower(param[2])	
		duration = tonumber(param[3])
	end
	
	if(player == 0) then
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Player not found.")
		doSendMagicEffect(playerPos, CONST_ME_POFF)		
		return true
	end
	
	if(action == "add") then
		setPlayerStorageValue(player, sid.BANNED_IN_HELP, os.time() + (60 * 60 * duration))
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "The player " .. getCreatureName(player) .. " has been banned from help channel for " .. duration .. " hours successfuly.")
	elseif(action == "remove") then
		setPlayerStorageValue(player, sid.BANNED_IN_HELP, -1)
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "The player " .. getCreatureName(player) .. " has been permited to send messages in help channel")
	else
		doPlayerSendTextMessage(cid, MESSAGE_STATUS_CONSOLE_BLUE, "Unknown action: " .. action)
		doSendMagicEffect(playerPos, CONST_ME_POFF)	
	end

	return true
end
